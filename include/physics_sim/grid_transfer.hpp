#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/mac_grid.hpp>
#include <physics_sim/math.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace physics_sim
{
struct ParticleGridTransferSummary
{
    double total_particle_mass = 0.0;
    Vec2 particle_momentum{};
};

template <typename IndexFn>
[[nodiscard]] inline float bilinear_sample(
    const std::vector<float>& values,
    std::size_t width,
    std::size_t height,
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

    const float x_clamped = width > 1 ? clamp(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
    const float y_clamped = height > 1 ? clamp(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
    const std::size_t x0 = static_cast<std::size_t>(std::floor(x_clamped));
    const std::size_t y0 = static_cast<std::size_t>(std::floor(y_clamped));
    const std::size_t x1 = std::min(x0 + 1, width - 1);
    const std::size_t y1 = std::min(y0 + 1, height - 1);
    const float tx = x_clamped - static_cast<float>(x0);
    const float ty = y_clamped - static_cast<float>(y0);

    const float v00 = values[index_fn(x0, y0)];
    const float v10 = values[index_fn(x1, y0)];
    const float v01 = values[index_fn(x0, y1)];
    const float v11 = values[index_fn(x1, y1)];
    const float a = v00 + (v10 - v00) * tx;
    const float b = v01 + (v11 - v01) * tx;
    return a + (b - a) * ty;
}

template <typename IndexFn>
inline void bilinear_scatter(
    std::vector<float>& values,
    std::vector<float>& weights,
    std::size_t width,
    std::size_t height,
    float x,
    float y,
    float contribution,
    IndexFn&& index_fn) noexcept
{
    if (width == 0 || height == 0 || values.empty())
    {
        return;
    }

    const float x_clamped = width > 1 ? clamp(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
    const float y_clamped = height > 1 ? clamp(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
    const std::size_t x0 = static_cast<std::size_t>(std::floor(x_clamped));
    const std::size_t y0 = static_cast<std::size_t>(std::floor(y_clamped));
    const std::size_t x1 = std::min(x0 + 1, width - 1);
    const std::size_t y1 = std::min(y0 + 1, height - 1);
    const float tx = x_clamped - static_cast<float>(x0);
    const float ty = y_clamped - static_cast<float>(y0);

    const float w00 = (1.0f - tx) * (1.0f - ty);
    const float w10 = tx * (1.0f - ty);
    const float w01 = (1.0f - tx) * ty;
    const float w11 = tx * ty;

    const auto accumulate = [&](std::size_t x_index, std::size_t y_index, float w)
    {
        const std::size_t idx = index_fn(x_index, y_index);
        values[idx] += contribution * w;
        weights[idx] += w;
    };

    accumulate(x0, y0, w00);
    accumulate(x1, y0, w10);
    accumulate(x0, y1, w01);
    accumulate(x1, y1, w11);
}

[[nodiscard]] inline Vec2 sample_mac_grid_velocity(const MacGrid2D& grid, Vec2 position) noexcept
{
    const float inv = 1.0f / grid.cell_size();
    const float u = bilinear_sample(
        grid.u_values(),
        grid.width() + 1,
        grid.height(),
        position.x * inv,
        position.y * inv - 0.5f,
        [&](std::size_t x, std::size_t y) { return y * (grid.width() + 1) + x; });
    const float v = bilinear_sample(
        grid.v_values(),
        grid.width(),
        grid.height() + 1,
        position.x * inv - 0.5f,
        position.y * inv,
        [&](std::size_t x, std::size_t y) { return y * grid.width() + x; });
    return {u, v};
}

inline void scatter_particle_velocity(
    MacGrid2D& grid,
    const FluidParticle& particle,
    std::vector<float>& u_weights,
    std::vector<float>& v_weights) noexcept
{
    const float inv = 1.0f / grid.cell_size();
    bilinear_scatter(
        grid.u_values(),
        u_weights,
        grid.width() + 1,
        grid.height(),
        particle.position.x * inv,
        particle.position.y * inv - 0.5f,
        particle.velocity.x,
        [&](std::size_t x, std::size_t y) { return y * (grid.width() + 1) + x; });

    bilinear_scatter(
        grid.v_values(),
        v_weights,
        grid.width(),
        grid.height() + 1,
        particle.position.x * inv - 0.5f,
        particle.position.y * inv,
        particle.velocity.y,
        [&](std::size_t x, std::size_t y) { return y * grid.width() + x; });
}

inline void normalize_mac_grid_faces(MacGrid2D& grid, const std::vector<float>& u_weights, const std::vector<float>& v_weights) noexcept
{
    for (std::size_t i = 0; i < grid.u_count(); ++i)
    {
        const float weight = u_weights[i];
        grid.u_raw()[i] = weight > 0.0f ? grid.u_raw()[i] / weight : 0.0f;
    }

    for (std::size_t i = 0; i < grid.v_count(); ++i)
    {
        const float weight = v_weights[i];
        grid.v_raw()[i] = weight > 0.0f ? grid.v_raw()[i] / weight : 0.0f;
    }
}

[[nodiscard]] inline Vec2 apic_velocity_at(const FluidParticle& particle, Vec2 sample_position) noexcept
{
    return particle.velocity + particle.affine_velocity * (sample_position - particle.position);
}

[[nodiscard]] inline Vec2 pic_flip_blend(
    Vec2 previous_particle_velocity,
    Vec2 pic_velocity,
    Vec2 previous_grid_velocity,
    Vec2 current_grid_velocity,
    float flip_blend) noexcept
{
    const float beta = clamp(flip_blend, 0.0f, 1.0f);
    const Vec2 flip_velocity = previous_particle_velocity + (current_grid_velocity - previous_grid_velocity);
    return {
        pic_velocity.x * (1.0f - beta) + flip_velocity.x * beta,
        pic_velocity.y * (1.0f - beta) + flip_velocity.y * beta,
    };
}

inline void normalize_mac_grid_faces_by_mass(MacGrid2D& grid, const std::vector<float>& u_masses, const std::vector<float>& v_masses) noexcept
{
    normalize_mac_grid_faces(grid, u_masses, v_masses);
}

[[nodiscard]] inline Vec2 grid_momentum_from_faces(
    const MacGrid2D& grid,
    const std::vector<float>& u_masses,
    const std::vector<float>& v_masses) noexcept
{
    Vec2 momentum{};
    for (std::size_t i = 0; i < grid.u_count() && i < u_masses.size(); ++i)
    {
        momentum.x += grid.u_values()[i] * u_masses[i];
    }
    for (std::size_t i = 0; i < grid.v_count() && i < v_masses.size(); ++i)
    {
        momentum.y += grid.v_values()[i] * v_masses[i];
    }
    return momentum;
}

template <typename IndexFn, typename FacePositionFn>
inline void bilinear_scatter_apic_component(
    std::vector<float>& values,
    std::vector<float>& masses,
    std::size_t width,
    std::size_t height,
    float x,
    float y,
    const FluidParticle& particle,
    float particle_mass,
    bool scatter_x_component,
    IndexFn&& index_fn,
    FacePositionFn&& face_position_fn) noexcept
{
    if (width == 0 || height == 0 || values.empty())
    {
        return;
    }

    const float x_clamped = width > 1 ? clamp(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
    const float y_clamped = height > 1 ? clamp(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
    const std::size_t x0 = static_cast<std::size_t>(std::floor(x_clamped));
    const std::size_t y0 = static_cast<std::size_t>(std::floor(y_clamped));
    const std::size_t x1 = std::min(x0 + 1, width - 1);
    const std::size_t y1 = std::min(y0 + 1, height - 1);
    const float tx = x_clamped - static_cast<float>(x0);
    const float ty = y_clamped - static_cast<float>(y0);

    const float w00 = (1.0f - tx) * (1.0f - ty);
    const float w10 = tx * (1.0f - ty);
    const float w01 = (1.0f - tx) * ty;
    const float w11 = tx * ty;

    const auto accumulate = [&](std::size_t x_index, std::size_t y_index, float weight)
    {
        const std::size_t idx = index_fn(x_index, y_index);
        const Vec2 face_velocity = apic_velocity_at(particle, face_position_fn(x_index, y_index));
        values[idx] += (scatter_x_component ? face_velocity.x : face_velocity.y) * particle_mass * weight;
        masses[idx] += particle_mass * weight;
    };

    accumulate(x0, y0, w00);
    accumulate(x1, y0, w10);
    accumulate(x0, y1, w01);
    accumulate(x1, y1, w11);
}

[[nodiscard]] inline ParticleGridTransferSummary scatter_particles_apic_to_grid(
    MacGrid2D& grid,
    const std::vector<FluidParticle>& particles,
    std::vector<float>& u_masses,
    std::vector<float>& v_masses) noexcept
{
    ParticleGridTransferSummary summary;
    if (u_masses.size() != grid.u_count())
    {
        u_masses.assign(grid.u_count(), 0.0f);
    }
    if (v_masses.size() != grid.v_count())
    {
        v_masses.assign(grid.v_count(), 0.0f);
    }

    const float inv = 1.0f / grid.cell_size();
    for (const auto& particle : particles)
    {
        const float particle_mass = particle.mass > 0.0f
            ? particle.mass
            : (particle.volume > 0.0f ? particle.volume : 1.0f);

        summary.total_particle_mass += static_cast<double>(particle_mass);
        summary.particle_momentum.x += particle.velocity.x * particle_mass;
        summary.particle_momentum.y += particle.velocity.y * particle_mass;

        bilinear_scatter_apic_component(
            grid.u_values(),
            u_masses,
            grid.width() + 1,
            grid.height(),
            particle.position.x * inv,
            particle.position.y * inv - 0.5f,
            particle,
            particle_mass,
            true,
            [&](std::size_t x, std::size_t y) { return y * (grid.width() + 1) + x; },
            [&](std::size_t x, std::size_t y) {
                return Vec2{static_cast<float>(x) * grid.cell_size(), (static_cast<float>(y) + 0.5f) * grid.cell_size()};
            });

        bilinear_scatter_apic_component(
            grid.v_values(),
            v_masses,
            grid.width(),
            grid.height() + 1,
            particle.position.x * inv - 0.5f,
            particle.position.y * inv,
            particle,
            particle_mass,
            false,
            [&](std::size_t x, std::size_t y) { return y * grid.width() + x; },
            [&](std::size_t x, std::size_t y) {
                return Vec2{(static_cast<float>(x) + 0.5f) * grid.cell_size(), static_cast<float>(y) * grid.cell_size()};
            });
    }

    return summary;
}
} // namespace physics_sim
