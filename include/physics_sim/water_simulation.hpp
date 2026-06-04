#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/mac_grid.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace physics_sim
{
enum class WaterEmitterKind
{
    Directional,
    Omni,
};

struct WaterEmitter
{
    WaterEmitterKind kind = WaterEmitterKind::Directional;
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float speed = 6.0f;
    float emission_rate = 0.0f;
    bool enabled = true;
    double emission_accumulator = 0.0;
    std::uint64_t emitted_particles = 0;
};

struct WaterSimulationMetrics
{
    std::uint64_t total_emitted = 0;
    std::size_t active_cells = 0;
    double average_divergence_after_projection = 0.0;
    double max_divergence_after_projection = 0.0;
};

class WaterSimulation2D
{
public:
    using size_type = std::size_t;

    WaterSimulation2D() = default;

    WaterSimulation2D(size_type width, size_type height, float cell_size = 1.0f)
    {
        resize(width, height, cell_size);
    }

    void resize(size_type width, size_type height, float cell_size = 1.0f)
    {
        grid_.resize(width, height, cell_size);
        particles_.clear();
        emitters_.clear();
        fluid_cells_.assign(grid_.cell_count(), 0U);
        u_previous_.assign(grid_.u_count(), 0.0f);
        v_previous_.assign(grid_.v_count(), 0.0f);
        pressure_next_.assign(grid_.cell_count(), 0.0f);
        metrics_ = {};
    }

    [[nodiscard]] MacGrid2D& grid() noexcept
    {
        return grid_;
    }

    [[nodiscard]] const MacGrid2D& grid() const noexcept
    {
        return grid_;
    }

    [[nodiscard]] std::vector<FluidParticle>& particles() noexcept
    {
        return particles_;
    }

    [[nodiscard]] const std::vector<FluidParticle>& particles() const noexcept
    {
        return particles_;
    }

    [[nodiscard]] std::vector<WaterEmitter>& emitters() noexcept
    {
        return emitters_;
    }

    [[nodiscard]] const std::vector<WaterEmitter>& emitters() const noexcept
    {
        return emitters_;
    }

    [[nodiscard]] const WaterSimulationMetrics& metrics() const noexcept
    {
        return metrics_;
    }

    void add_emitter(const WaterEmitter& emitter)
    {
        emitters_.push_back(emitter);
    }

    void add_particle(const FluidParticle& particle)
    {
        particles_.push_back(particle);
    }

    void set_solid_cell(size_type x, size_type y, bool value)
    {
        if (grid_.contains(x, y))
        {
            grid_.set_solid(x, y, value);
        }
    }

    void paint_wall_line(Vec2 start, Vec2 end, bool solid)
    {
        if (grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        const auto to_cell_x = [&](float value) -> int
        {
            return static_cast<int>(std::floor(value / grid_.cell_size()));
        };

        const auto to_cell_y = [&](float value) -> int
        {
            return static_cast<int>(std::floor(value / grid_.cell_size()));
        };

        int x0 = to_cell_x(start.x);
        int y0 = to_cell_y(start.y);
        const int x1 = to_cell_x(end.x);
        const int y1 = to_cell_y(end.y);

        const int dx = std::abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (true)
        {
            if (x0 >= 0 && y0 >= 0)
            {
                set_solid_cell(static_cast<size_type>(x0), static_cast<size_type>(y0), solid);
            }

            if (x0 == x1 && y0 == y1)
            {
                break;
            }

            const int twice_err = err * 2;
            if (twice_err >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (twice_err <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void clear_fluid() noexcept
    {
        particles_.clear();
        grid_.clear_fields();
        fluid_cells_.assign(grid_.cell_count(), 0U);
        metrics_ = {};
        for (auto& emitter : emitters_)
        {
            emitter.emission_accumulator = 0.0;
            emitter.emitted_particles = 0;
        }
    }

    void clear_scene() noexcept
    {
        clear_fluid();
        grid_.clear_solids();
        emitters_.clear();
    }

    void step(double dt)
    {
        if (dt <= 0.0 || grid_.width() == 0 || grid_.height() == 0)
        {
            metrics_ = {};
            return;
        }

        const float step_seconds = static_cast<float>(dt);
        const float gravity = 9.8f;
        const float inv_cell_size = 1.0f / grid_.cell_size();

        emit_particles(step_seconds);

        for (auto& particle : particles_)
        {
            particle.velocity.y += gravity * step_seconds;
        }

        fluid_cells_.assign(grid_.cell_count(), 0U);
        grid_.clear_fields();

        scatter_particles_to_grid();
        apply_boundary_conditions();
        mark_fluid_cells();

        u_previous_ = collect_u();
        v_previous_ = collect_v();

        project_pressures();
        apply_pressure_gradient();
        apply_boundary_conditions();

        double divergence_sum = 0.0;
        double divergence_max = 0.0;
        std::size_t divergence_count = 0;
        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = grid_.cell_index(x, y);
                if (!is_active_cell(idx))
                {
                    grid_.divergence(x, y) = 0.0f;
                    continue;
                }

                const float divergence = compute_divergence_for_cell(x, y, inv_cell_size);
                grid_.divergence(x, y) = divergence;
                divergence_sum += std::abs(static_cast<double>(divergence));
                divergence_max = std::max(divergence_max, std::abs(static_cast<double>(divergence)));
                ++divergence_count;
            }
        }

        transfer_grid_to_particles(step_seconds);
        advect_particles(step_seconds);

        metrics_.total_emitted = total_emitted_;
        metrics_.active_cells = divergence_count;
        metrics_.average_divergence_after_projection = divergence_count > 0 ? divergence_sum / static_cast<double>(divergence_count) : 0.0;
        metrics_.max_divergence_after_projection = divergence_max;
    }

private:
    [[nodiscard]] bool is_valid_cell(size_type x, size_type y) const noexcept
    {
        return grid_.contains(x, y);
    }

    [[nodiscard]] size_type safe_cell_index(size_type x, size_type y) const noexcept
    {
        return grid_.cell_index(x, y);
    }

    [[nodiscard]] bool is_active_cell(size_type index) const noexcept
    {
        return index < fluid_cells_.size() && fluid_cells_[index] != 0U;
    }

    [[nodiscard]] float domain_width() const noexcept
    {
        return static_cast<float>(grid_.width()) * grid_.cell_size();
    }

    [[nodiscard]] float domain_height() const noexcept
    {
        return static_cast<float>(grid_.height()) * grid_.cell_size();
    }

    [[nodiscard]] static float clampf(float value, float min_value, float max_value) noexcept
    {
        return value < min_value ? min_value : (value > max_value ? max_value : value);
    }

    [[nodiscard]] static Vec2 normalized_or_default(Vec2 value, Vec2 fallback) noexcept
    {
        const float len = length(value);
        if (len <= 0.0f)
        {
            return fallback;
        }
        return value / len;
    }

    [[nodiscard]] static float lerp(float a, float b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    template <typename IndexFn>
    [[nodiscard]] static float bilinear_sample(
        const std::vector<float>& values,
        size_type width,
        size_type height,
        float x,
        float y,
        IndexFn&& index_fn) noexcept
    {
        if (width == 0 || height == 0 || values.empty())
        {
            return 0.0f;
        }

        if (width == 1 && height == 1)
        {
            return values.front();
        }

        const float x_clamped = width > 1 ? clampf(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
        const float y_clamped = height > 1 ? clampf(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
        const size_type x0 = static_cast<size_type>(std::floor(x_clamped));
        const size_type y0 = static_cast<size_type>(std::floor(y_clamped));
        const size_type x1 = std::min(x0 + 1, width - 1);
        const size_type y1 = std::min(y0 + 1, height - 1);
        const float tx = x_clamped - static_cast<float>(x0);
        const float ty = y_clamped - static_cast<float>(y0);

        const float v00 = values[index_fn(x0, y0)];
        const float v10 = values[index_fn(x1, y0)];
        const float v01 = values[index_fn(x0, y1)];
        const float v11 = values[index_fn(x1, y1)];
        const float a = lerp(v00, v10, tx);
        const float b = lerp(v01, v11, tx);
        return lerp(a, b, ty);
    }

    template <typename IndexFn>
    static void bilinear_scatter(
        std::vector<float>& values,
        std::vector<float>& weights,
        size_type width,
        size_type height,
        float x,
        float y,
        float contribution,
        IndexFn&& index_fn) noexcept
    {
        if (width == 0 || height == 0 || values.empty())
        {
            return;
        }

        const float x_clamped = width > 1 ? clampf(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
        const float y_clamped = height > 1 ? clampf(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
        const size_type x0 = static_cast<size_type>(std::floor(x_clamped));
        const size_type y0 = static_cast<size_type>(std::floor(y_clamped));
        const size_type x1 = std::min(x0 + 1, width - 1);
        const size_type y1 = std::min(y0 + 1, height - 1);
        const float tx = x_clamped - static_cast<float>(x0);
        const float ty = y_clamped - static_cast<float>(y0);

        const float w00 = (1.0f - tx) * (1.0f - ty);
        const float w10 = tx * (1.0f - ty);
        const float w01 = (1.0f - tx) * ty;
        const float w11 = tx * ty;

        const auto accumulate = [&](size_type x_index, size_type y_index, float w) {
            const size_type idx = index_fn(x_index, y_index);
            values[idx] += contribution * w;
            weights[idx] += w;
        };

        accumulate(x0, y0, w00);
        accumulate(x1, y0, w10);
        accumulate(x0, y1, w01);
        accumulate(x1, y1, w11);
    }

    [[nodiscard]] Vec2 sample_velocity(const Vec2& position,
                                       const std::vector<float>& u_values,
                                       const std::vector<float>& v_values) const noexcept
    {
        const float inv = 1.0f / grid_.cell_size();
        const float u = bilinear_sample(
            u_values,
            grid_.width() + 1,
            grid_.height(),
            position.x * inv,
            position.y * inv - 0.5f,
            [&](size_type x, size_type y) { return y * (grid_.width() + 1) + x; });
        const float v = bilinear_sample(
            v_values,
            grid_.width(),
            grid_.height() + 1,
            position.x * inv - 0.5f,
            position.y * inv,
            [&](size_type x, size_type y) { return y * grid_.width() + x; });
        return {u, v};
    }

    void emit_particles(float dt)
    {
        for (auto& emitter : emitters_)
        {
            if (!emitter.enabled || emitter.emission_rate <= 0.0f)
            {
                continue;
            }

            emitter.emission_accumulator += static_cast<double>(emitter.emission_rate) * static_cast<double>(dt);
            const auto direction = normalized_or_default(emitter.direction, Vec2{0.0f, 1.0f});

            while (emitter.emission_accumulator >= 1.0)
            {
                emitter.emission_accumulator -= 1.0;

                Vec2 spawn_position = emitter.position;
                Vec2 spawn_velocity = direction * emitter.speed;

                if (emitter.kind == WaterEmitterKind::Omni)
                {
                    const double seed = static_cast<double>(emitter.emitted_particles + 1);
                    const double angle = std::fmod(seed * 2.39996322972865332, 6.2831853071795864769);
                    const float c = static_cast<float>(std::cos(angle));
                    const float s = static_cast<float>(std::sin(angle));
                    Vec2 radial{c, s};
                    spawn_velocity = radial * emitter.speed;
                }

                particles_.push_back({spawn_position, spawn_velocity});
                ++emitter.emitted_particles;
                ++total_emitted_;
            }
        }
    }

    void scatter_particles_to_grid()
    {
        u_weights_.assign(grid_.u_count(), 0.0f);
        v_weights_.assign(grid_.v_count(), 0.0f);
        if (u_previous_.size() != grid_.u_count())
        {
            u_previous_.assign(grid_.u_count(), 0.0f);
        }
        if (v_previous_.size() != grid_.v_count())
        {
            v_previous_.assign(grid_.v_count(), 0.0f);
        }

        for (const auto& particle : particles_)
        {
            const float inv = 1.0f / grid_.cell_size();
            bilinear_scatter(
                grid_.u_values(),
                u_weights_,
                grid_.width() + 1,
                grid_.height(),
                particle.position.x * inv,
                particle.position.y * inv - 0.5f,
                particle.velocity.x,
                [&](size_type x, size_type y) { return y * (grid_.width() + 1) + x; });

            bilinear_scatter(
                grid_.v_values(),
                v_weights_,
                grid_.width(),
                grid_.height() + 1,
                particle.position.x * inv - 0.5f,
                particle.position.y * inv,
                particle.velocity.y,
                [&](size_type x, size_type y) { return y * grid_.width() + x; });
        }

        normalize_faces();
    }

    void normalize_faces()
    {
        for (size_type i = 0; i < grid_.u_count(); ++i)
        {
            const float weight = u_weights_[i];
            grid_.u_raw()[i] = weight > 0.0f ? grid_.u_raw()[i] / weight : 0.0f;
        }
        for (size_type i = 0; i < grid_.v_count(); ++i)
        {
            const float weight = v_weights_[i];
            grid_.v_raw()[i] = weight > 0.0f ? grid_.v_raw()[i] / weight : 0.0f;
        }
    }

    void apply_boundary_conditions()
    {
        const auto width = grid_.width();
        const auto height = grid_.height();

        for (size_type y = 0; y < height; ++y)
        {
            grid_.u(0, y) = 0.0f;
            grid_.u(width, y) = 0.0f;
            for (size_type x = 0; x < width; ++x)
            {
                if (grid_.solid(x, y))
                {
                    grid_.u(x, y) = 0.0f;
                    grid_.u(x + 1, y) = 0.0f;
                    grid_.v(x, y) = 0.0f;
                    grid_.v(x, y + 1) = 0.0f;
                }
                if (x + 1 < width && grid_.solid(x, y))
                {
                    grid_.u(x + 1, y) = 0.0f;
                }
            }
        }

        for (size_type x = 0; x < width; ++x)
        {
            grid_.v(x, 0) = 0.0f;
            grid_.v(x, height) = 0.0f;
        }

        for (size_type y = 0; y < height; ++y)
        {
            for (size_type x = 0; x < width; ++x)
            {
                if (grid_.solid(x, y))
                {
                    if (x > 0)
                    {
                        grid_.u(x, y) = 0.0f;
                    }
                    if (x + 1 <= width)
                    {
                        grid_.u(x + 1, y) = 0.0f;
                    }
                    if (y > 0)
                    {
                        grid_.v(x, y) = 0.0f;
                    }
                    if (y + 1 <= height)
                    {
                        grid_.v(x, y + 1) = 0.0f;
                    }
                }
            }
        }
    }

    void mark_fluid_cells()
    {
        const float inv = 1.0f / grid_.cell_size();
        for (const auto& particle : particles_)
        {
            const int cell_x = static_cast<int>(std::floor(particle.position.x * inv));
            const int cell_y = static_cast<int>(std::floor(particle.position.y * inv));
            for (int offset_y = -2; offset_y <= 2; ++offset_y)
            {
                for (int offset_x = -2; offset_x <= 2; ++offset_x)
                {
                    const int x = cell_x + offset_x;
                    const int y = cell_y + offset_y;
                    if (x >= 0 && y >= 0 && static_cast<size_type>(x) < grid_.width() && static_cast<size_type>(y) < grid_.height())
                    {
                        const size_type idx = safe_cell_index(static_cast<size_type>(x), static_cast<size_type>(y));
                        if (!grid_.solid(static_cast<size_type>(x), static_cast<size_type>(y)))
                        {
                            fluid_cells_[idx] = 1U;
                        }
                    }
                }
            }
        }

        if (!particles_.empty() || !emitters_.empty())
        {
            for (size_type y = 0; y < grid_.height(); ++y)
            {
                for (size_type x = 0; x < grid_.width(); ++x)
                {
                    if (!grid_.solid(x, y))
                    {
                        fluid_cells_[safe_cell_index(x, y)] = 1U;
                    }
                }
            }
        }
    }

    void project_pressures()
    {
        const auto cell_count = grid_.cell_count();
        if (pressure_next_.size() != cell_count)
        {
            pressure_next_.assign(cell_count, 0.0f);
        }

        std::vector<float> pressure_current(cell_count, 0.0f);
        const float cell_size = grid_.cell_size();

        for (int iteration = 0; iteration < 80; ++iteration)
        {
            for (size_type y = 0; y < grid_.height(); ++y)
            {
                for (size_type x = 0; x < grid_.width(); ++x)
                {
                    const size_type idx = safe_cell_index(x, y);
                    if (!is_active_cell(idx) || grid_.solid(x, y))
                    {
                        pressure_next_[idx] = 0.0f;
                        continue;
                    }

                    float neighbor_sum = 0.0f;
                    int neighbor_count = 0;

                    auto consider_neighbor = [&](int nx, int ny)
                    {
                        if (nx < 0 || ny < 0)
                        {
                            return;
                        }
                        const size_type sx = static_cast<size_type>(nx);
                        const size_type sy = static_cast<size_type>(ny);
                        if (!grid_.contains(sx, sy) || grid_.solid(sx, sy))
                        {
                            return;
                        }

                        const size_type nidx = safe_cell_index(sx, sy);
                        if (!is_active_cell(nidx))
                        {
                            return;
                        }

                        neighbor_sum += pressure_current[nidx];
                        ++neighbor_count;
                    };

                    consider_neighbor(static_cast<int>(x) - 1, static_cast<int>(y));
                    consider_neighbor(static_cast<int>(x) + 1, static_cast<int>(y));
                    consider_neighbor(static_cast<int>(x), static_cast<int>(y) - 1);
                    consider_neighbor(static_cast<int>(x), static_cast<int>(y) + 1);

                    const float divergence = compute_divergence_for_cell(x, y, 1.0f / cell_size);
                    pressure_next_[idx] = neighbor_count > 0
                        ? (neighbor_sum - divergence * cell_size * cell_size) / static_cast<float>(neighbor_count)
                        : 0.0f;
                }
            }

            pressure_current.swap(pressure_next_);
        }

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = safe_cell_index(x, y);
                grid_.pressure(x, y) = is_active_cell(idx) ? pressure_current[idx] : 0.0f;
            }
        }
    }

    [[nodiscard]] float compute_divergence_for_cell(size_type x, size_type y, float inv_cell_size) const noexcept
    {
        const float left = face_velocity_u(static_cast<int>(x), static_cast<int>(y));
        const float right = face_velocity_u(static_cast<int>(x) + 1, static_cast<int>(y));
        const float top = face_velocity_v(static_cast<int>(x), static_cast<int>(y));
        const float bottom = face_velocity_v(static_cast<int>(x), static_cast<int>(y) + 1);
        return (right - left + bottom - top) * inv_cell_size;
    }

    [[nodiscard]] float face_velocity_u(int x, int y) const noexcept
    {
        if (x < 0 || y < 0 || static_cast<size_type>(y) >= grid_.height() || static_cast<size_type>(x) > grid_.width())
        {
            return 0.0f;
        }

        return grid_.u(static_cast<size_type>(x), static_cast<size_type>(y));
    }

    [[nodiscard]] float face_velocity_v(int x, int y) const noexcept
    {
        if (x < 0 || y < 0 || static_cast<size_type>(x) >= grid_.width() || static_cast<size_type>(y) > grid_.height())
        {
            return 0.0f;
        }

        return grid_.v(static_cast<size_type>(x), static_cast<size_type>(y));
    }

    void apply_pressure_gradient()
    {
        const float inv = 1.0f / grid_.cell_size();

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x <= grid_.width(); ++x)
            {
                const float left_pressure = (x > 0 && !grid_.solid(x - 1, y)) ? grid_.pressure(x - 1, y) : 0.0f;
                const float right_pressure = (x < grid_.width() && !grid_.solid(std::min(x, grid_.width() - 1), y)) ? grid_.pressure(std::min(x, grid_.width() - 1), y) : 0.0f;

                if (x == 0 || x == grid_.width())
                {
                    grid_.u(x, y) = 0.0f;
                    continue;
                }

                const bool left_active = is_active_cell(safe_cell_index(x - 1, y));
                const bool right_active = is_active_cell(safe_cell_index(x, y));
                const float p_left = left_active && !grid_.solid(x - 1, y) ? left_pressure : 0.0f;
                const float p_right = right_active && !grid_.solid(x, y) ? right_pressure : 0.0f;
                grid_.u(x, y) -= (p_right - p_left) * inv;
            }
        }

        for (size_type y = 0; y <= grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                if (y == 0 || y == grid_.height())
                {
                    grid_.v(x, y) = 0.0f;
                    continue;
                }

                const bool top_active = is_active_cell(safe_cell_index(x, y - 1));
                const bool bottom_active = is_active_cell(safe_cell_index(x, y));
                const float p_top = top_active && !grid_.solid(x, y - 1) ? grid_.pressure(x, y - 1) : 0.0f;
                const float p_bottom = bottom_active && !grid_.solid(x, y) ? grid_.pressure(x, y) : 0.0f;
                grid_.v(x, y) -= (p_bottom - p_top) * inv;
            }
        }
    }

    void transfer_grid_to_particles(float /*dt*/)
    {
        const float flip_blend = 0.95f;

        for (auto& particle : particles_)
        {
            const Vec2 pic_velocity = sample_velocity(particle.position, grid_.u_values(), grid_.v_values());
            const Vec2 previous_velocity = sample_velocity(particle.position, u_previous_, v_previous_);
            const Vec2 flip_velocity{
                particle.velocity.x + (pic_velocity.x - previous_velocity.x),
                particle.velocity.y + (pic_velocity.y - previous_velocity.y),
            };
            particle.velocity.x = lerp(pic_velocity.x, flip_velocity.x, flip_blend);
            particle.velocity.y = lerp(pic_velocity.y, flip_velocity.y, flip_blend);
        }
    }

    void advect_particles(float dt)
    {
        const float width = domain_width();
        const float height = domain_height();
        const float epsilon = grid_.cell_size() * 0.001f;

        for (auto& particle : particles_)
        {
            const Vec2 previous_position = particle.position;
            particle.position.x += particle.velocity.x * dt;
            particle.position.y += particle.velocity.y * dt;

            particle.position.x = clampf(particle.position.x, epsilon, std::max(epsilon, width - epsilon));
            particle.position.y = clampf(particle.position.y, epsilon, std::max(epsilon, height - epsilon));

            const size_type cell_x = static_cast<size_type>(std::floor(particle.position.x / grid_.cell_size()));
            const size_type cell_y = static_cast<size_type>(std::floor(particle.position.y / grid_.cell_size()));

            if (grid_.contains(cell_x, cell_y) && grid_.solid(cell_x, cell_y))
            {
                particle.position = previous_position;
                particle.position.x = clampf(particle.position.x, epsilon, std::max(epsilon, width - epsilon));
                particle.position.y = clampf(particle.position.y, epsilon, std::max(epsilon, height - epsilon));
                particle.velocity = {};
            }
        }
    }

    [[nodiscard]] std::vector<float> collect_u() const
    {
        return grid_.u_values();
    }

    [[nodiscard]] std::vector<float> collect_v() const
    {
        return grid_.v_values();
    }

    MacGrid2D grid_{};
    std::vector<FluidParticle> particles_{};
    std::vector<WaterEmitter> emitters_{};
    std::vector<std::uint8_t> fluid_cells_{};
    std::vector<float> u_weights_{};
    std::vector<float> v_weights_{};
    std::vector<float> u_previous_{};
    std::vector<float> v_previous_{};
    std::vector<float> pressure_next_{};
    WaterSimulationMetrics metrics_{};
    std::uint64_t total_emitted_ = 0;
};
} // namespace physics_sim
