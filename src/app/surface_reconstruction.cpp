#include "physics_sim/surface_reconstruction.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>

namespace physics_sim
{
namespace
{

struct Sample
{
    SurfacePoint point{};
    float value = 0.0f;
};

[[nodiscard]] Sample intersection(const Sample& inside, const Sample& outside, float threshold)
{
    const float denominator = outside.value - inside.value;
    const float t = denominator == 0.0f ? 0.5f : std::clamp((threshold - inside.value) / denominator, 0.0f, 1.0f);
    return {
        {inside.point.x + (outside.point.x - inside.point.x) * t,
         inside.point.y + (outside.point.y - inside.point.y) * t},
        threshold};
}

void append_clipped_triangle(
    std::vector<SurfaceTriangle>& output,
    const std::array<Sample, 3>& input,
    float threshold)
{
    std::array<Sample, 6> polygon{};
    std::size_t count = 0;
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        const Sample& current = input[index];
        const Sample& next = input[(index + 1) % input.size()];
        const bool current_inside = current.value > threshold;
        const bool next_inside = next.value > threshold;
        if (current_inside)
        {
            polygon[count++] = current;
        }
        if (current_inside != next_inside)
        {
            polygon[count++] = current_inside
                ? intersection(current, next, threshold)
                : intersection(next, current, threshold);
        }
    }

    for (std::size_t index = 1; index + 1 < count; ++index)
    {
        const float coverage = std::clamp(
            (polygon[0].value + polygon[index].value + polygon[index + 1].value) / 3.0f,
            threshold,
            std::max(threshold, 1.0f));
        output.push_back({polygon[0].point, polygon[index].point, polygon[index + 1].point, coverage});
    }
}

} // namespace

ParticleSurfaceField build_particle_surface_field(
    std::span<const FluidParticle> particles,
    std::span<const std::uint8_t> coarse_solid_cells,
    std::size_t coarse_width,
    std::size_t coarse_height,
    float cell_size,
    std::size_t subdivisions)
{
    ParticleSurfaceField field;
    if (coarse_width == 0 || coarse_height == 0 || cell_size <= 0.0f
        || (!coarse_solid_cells.empty() && coarse_solid_cells.size() != coarse_width * coarse_height))
    {
        return field;
    }

    subdivisions = std::clamp<std::size_t>(subdivisions, 1, 8);
    field.width = coarse_width * subdivisions;
    field.height = coarse_height * subdivisions;
    field.sample_spacing = cell_size / static_cast<float>(subdivisions);
    field.vertex_values.assign((field.width + 1) * (field.height + 1), 0.0f);
    field.solid_cells.assign(field.width * field.height, 0);

    if (!coarse_solid_cells.empty())
    {
        for (std::size_t y = 0; y < field.height; ++y)
        {
            for (std::size_t x = 0; x < field.width; ++x)
            {
                const std::size_t coarse_x = std::min(x / subdivisions, coarse_width - 1);
                const std::size_t coarse_y = std::min(y / subdivisions, coarse_height - 1);
                field.solid_cells[y * field.width + x] = coarse_solid_cells[coarse_y * coarse_width + coarse_x] != 0 ? 1 : 0;
            }
        }
    }

    constexpr double pi = 3.14159265358979323846;
    constexpr float support_scale = 3.0f;
    constexpr float contour_threshold = 0.5f;
    constexpr float kernel_peak = contour_threshold / (1.0f - 1.0f / (2.0f * support_scale * support_scale));
    const float domain_width = static_cast<float>(coarse_width) * cell_size;
    const float domain_height = static_cast<float>(coarse_height) * cell_size;
    const auto segment_crosses_solid = [&](Vec2 start, Vec2 end)
    {
        if (coarse_solid_cells.empty())
        {
            return false;
        }
        const float distance = length(end - start);
        const int steps = std::max(1, static_cast<int>(std::ceil(distance / std::max(cell_size * 0.5f, 0.001f))));
        for (int step = 1; step <= steps; ++step)
        {
            const float ratio = static_cast<float>(step) / static_cast<float>(steps);
            const Vec2 sample = start + (end - start) * ratio;
            if (sample.x < 0.0f || sample.y < 0.0f || sample.x >= domain_width || sample.y >= domain_height)
            {
                continue;
            }
            const std::size_t solid_x = std::min(static_cast<std::size_t>(sample.x / cell_size), coarse_width - 1);
            const std::size_t solid_y = std::min(static_cast<std::size_t>(sample.y / cell_size), coarse_height - 1);
            if (coarse_solid_cells[solid_y * coarse_width + solid_x] != 0)
            {
                return true;
            }
        }
        return false;
    };
    for (const FluidParticle& particle : particles)
    {
        if (particle.volume <= 0.0f || !std::isfinite(particle.volume)
            || !std::isfinite(particle.position.x) || !std::isfinite(particle.position.y))
        {
            continue;
        }
        field.particle_area += static_cast<double>(particle.volume);
        const float support_radius = support_scale * std::sqrt(2.0f * particle.volume / static_cast<float>(pi));
        if (support_radius <= 0.0f)
        {
            continue;
        }

        const float min_world_x = std::max(0.0f, particle.position.x - support_radius);
        const float max_world_x = std::min(domain_width, particle.position.x + support_radius);
        const float min_world_y = std::max(0.0f, particle.position.y - support_radius);
        const float max_world_y = std::min(domain_height, particle.position.y + support_radius);
        const std::size_t min_x = std::min(static_cast<std::size_t>(std::floor(min_world_x / field.sample_spacing)), field.width);
        const std::size_t max_x = std::min(static_cast<std::size_t>(std::ceil(max_world_x / field.sample_spacing)), field.width);
        const std::size_t min_y = std::min(static_cast<std::size_t>(std::floor(min_world_y / field.sample_spacing)), field.height);
        const std::size_t max_y = std::min(static_cast<std::size_t>(std::ceil(max_world_y / field.sample_spacing)), field.height);
        const float inverse_radius_squared = 1.0f / (support_radius * support_radius);
        for (std::size_t y = min_y; y <= max_y; ++y)
        {
            const float world_y = static_cast<float>(y) * field.sample_spacing;
            for (std::size_t x = min_x; x <= max_x; ++x)
            {
                const float world_x = static_cast<float>(x) * field.sample_spacing;
                const float offset_x = world_x - particle.position.x;
                const float offset_y = world_y - particle.position.y;
                const float normalized_radius_squared = (offset_x * offset_x + offset_y * offset_y) * inverse_radius_squared;
                if (normalized_radius_squared < 1.0f
                    && !segment_crosses_solid(particle.position, Vec2{world_x, world_y}))
                {
                    field.vertex_values[y * (field.width + 1) + x] += kernel_peak * (1.0f - normalized_radius_squared);
                }
            }
        }
    }
    return field;
}

namespace
{
[[nodiscard]] std::vector<SurfaceTriangle> reconstruct_particle_surface_at_threshold(
    const ParticleSurfaceField& field,
    float threshold)
{
    if (field.width == 0 || field.height == 0 || field.sample_spacing <= 0.0f
        || field.vertex_values.size() != (field.width + 1) * (field.height + 1)
        || (!field.solid_cells.empty() && field.solid_cells.size() != field.width * field.height))
    {
        return {};
    }

    std::vector<SurfaceTriangle> triangles;
    triangles.reserve(field.width * field.height);
    for (std::size_t y = 0; y < field.height; ++y)
    {
        for (std::size_t x = 0; x < field.width; ++x)
        {
            const std::size_t cell_index = y * field.width + x;
            if (!field.solid_cells.empty() && field.solid_cells[cell_index] != 0)
            {
                continue;
            }
            const std::size_t top = y * (field.width + 1) + x;
            const std::size_t bottom = (y + 1) * (field.width + 1) + x;
            const Sample top_left{{x * field.sample_spacing, y * field.sample_spacing}, field.vertex_values[top]};
            const Sample top_right{{(x + 1) * field.sample_spacing, y * field.sample_spacing}, field.vertex_values[top + 1]};
            const Sample bottom_right{{(x + 1) * field.sample_spacing, (y + 1) * field.sample_spacing}, field.vertex_values[bottom + 1]};
            const Sample bottom_left{{x * field.sample_spacing, (y + 1) * field.sample_spacing}, field.vertex_values[bottom]};
            append_clipped_triangle(triangles, {top_left, top_right, bottom_right}, threshold);
            append_clipped_triangle(triangles, {top_left, bottom_right, bottom_left}, threshold);
        }
    }
    return triangles;
}
} // namespace

std::vector<SurfaceTriangle> reconstruct_particle_surface(const ParticleSurfaceField& field, float threshold)
{
    if (threshold >= 0.0f || field.particle_area <= 0.0)
    {
        return reconstruct_particle_surface_at_threshold(field, std::max(0.0f, threshold));
    }

    return reconstruct_particle_surface_at_threshold(field, particle_surface_threshold(field));
}

float particle_surface_threshold(const ParticleSurfaceField& field)
{
    const float maximum_value = field.vertex_values.empty()
        ? 0.0f
        : *std::max_element(field.vertex_values.begin(), field.vertex_values.end());
    if (maximum_value <= 0.0f)
    {
        return 0.0f;
    }

    float lower = 0.0f;
    float upper = maximum_value;
    for (int iteration = 0; iteration < 10; ++iteration)
    {
        const float candidate = (lower + upper) * 0.5f;
        const double candidate_area = surface_area(reconstruct_particle_surface_at_threshold(field, candidate));
        if (candidate_area > field.particle_area)
        {
            lower = candidate;
        }
        else
        {
            upper = candidate;
        }
    }
    return (lower + upper) * 0.5f;
}

double surface_area(std::span<const SurfaceTriangle> triangles) noexcept
{
    double area = 0.0;
    for (const SurfaceTriangle& triangle : triangles)
    {
        const double ab_x = static_cast<double>(triangle.b.x - triangle.a.x);
        const double ab_y = static_cast<double>(triangle.b.y - triangle.a.y);
        const double ac_x = static_cast<double>(triangle.c.x - triangle.a.x);
        const double ac_y = static_cast<double>(triangle.c.y - triangle.a.y);
        area += std::abs(ab_x * ac_y - ab_y * ac_x) * 0.5;
    }
    return area;
}

std::vector<SurfaceTriangle> reconstruct_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float cell_size,
    float threshold)
{
    if (width == 0 || height == 0 || cell_volume_fractions.size() != width * height ||
        (!solid_cells.empty() && solid_cells.size() != width * height))
    {
        return {};
    }

    std::vector<float> vertices((width + 1) * (height + 1), 0.0f);
    for (std::size_t vertex_y = 0; vertex_y <= height; ++vertex_y)
    {
        for (std::size_t vertex_x = 0; vertex_x <= width; ++vertex_x)
        {
            float sum = 0.0f;
            std::size_t count = 0;
            for (int offset_y = -1; offset_y <= 0; ++offset_y)
            {
                for (int offset_x = -1; offset_x <= 0; ++offset_x)
                {
                    const int cell_x = static_cast<int>(vertex_x) + offset_x;
                    const int cell_y = static_cast<int>(vertex_y) + offset_y;
                    if (cell_x < 0 || cell_y < 0 || cell_x >= static_cast<int>(width) || cell_y >= static_cast<int>(height))
                    {
                        continue;
                    }
                    const std::size_t index = static_cast<std::size_t>(cell_y) * width + static_cast<std::size_t>(cell_x);
                    if (!solid_cells.empty() && solid_cells[index])
                    {
                        continue;
                    }
                    sum += std::clamp(cell_volume_fractions[index], 0.0f, 1.0f);
                    ++count;
                }
            }
            vertices[vertex_y * (width + 1) + vertex_x] = count == 0 ? 0.0f : sum / static_cast<float>(count);
        }
    }

    std::vector<SurfaceTriangle> triangles;
    triangles.reserve(width * height * 3);
    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const std::size_t cell_index = y * width + x;
            if (!solid_cells.empty() && solid_cells[cell_index])
            {
                continue;
            }
            const Sample top_left{{x * cell_size, y * cell_size}, vertices[y * (width + 1) + x]};
            const Sample top_right{{(x + 1) * cell_size, y * cell_size}, vertices[y * (width + 1) + x + 1]};
            const Sample bottom_right{{(x + 1) * cell_size, (y + 1) * cell_size}, vertices[(y + 1) * (width + 1) + x + 1]};
            const Sample bottom_left{{x * cell_size, (y + 1) * cell_size}, vertices[(y + 1) * (width + 1) + x]};
            append_clipped_triangle(triangles, {top_left, top_right, bottom_right}, threshold);
            append_clipped_triangle(triangles, {top_left, bottom_right, bottom_left}, threshold);
        }
    }
    return triangles;
}

SurfaceSemantics analyze_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float threshold,
    std::size_t allowed_min_x,
    std::size_t allowed_max_x,
    std::size_t allowed_min_y,
    std::size_t allowed_max_y)
{
    SurfaceSemantics result{};
    result.min_x = width;
    result.min_y = height;
    if (cell_volume_fractions.size() != width * height || (!solid_cells.empty() && solid_cells.size() != width * height))
    {
        return result;
    }
    allowed_max_x = std::min(allowed_max_x, width == 0 ? 0 : width - 1);
    allowed_max_y = std::min(allowed_max_y, height == 0 ? 0 : height - 1);
    std::vector<bool> occupied(width * height, false);
    for (std::size_t y = 0; y < height; ++y)
    {
        for (std::size_t x = 0; x < width; ++x)
        {
            const std::size_t index = y * width + x;
            occupied[index] = (solid_cells.empty() || !solid_cells[index]) && cell_volume_fractions[index] > threshold;
            if (!occupied[index])
            {
                continue;
            }
            ++result.occupied_cells;
            result.min_x = std::min(result.min_x, x);
            result.max_x = std::max(result.max_x, x);
            result.min_y = std::min(result.min_y, y);
            result.max_y = std::max(result.max_y, y);
            if (x < allowed_min_x || x > allowed_max_x || y < allowed_min_y || y > allowed_max_y)
            {
                ++result.outside_allowed_cells;
            }
        }
    }

    std::vector<bool> visited(width * height, false);
    constexpr std::array<std::array<int, 2>, 4> neighbors{{{{-1, 0}}, {{1, 0}}, {{0, -1}}, {{0, 1}}}};
    for (std::size_t start = 0; start < occupied.size(); ++start)
    {
        if (!occupied[start] || visited[start])
        {
            continue;
        }
        ++result.connected_components;
        std::size_t component_size = 0;
        std::queue<std::size_t> pending;
        pending.push(start);
        visited[start] = true;
        while (!pending.empty())
        {
            const std::size_t index = pending.front();
            pending.pop();
            ++component_size;
            const int x = static_cast<int>(index % width);
            const int y = static_cast<int>(index / width);
            for (const auto& offset : neighbors)
            {
                const int nx = x + offset[0];
                const int ny = y + offset[1];
                if (nx < 0 || ny < 0 || nx >= static_cast<int>(width) || ny >= static_cast<int>(height))
                {
                    continue;
                }
                const std::size_t neighbor = static_cast<std::size_t>(ny) * width + static_cast<std::size_t>(nx);
                if (occupied[neighbor] && !visited[neighbor])
                {
                    visited[neighbor] = true;
                    pending.push(neighbor);
                }
            }
        }
        if (component_size == 1)
        {
            ++result.isolated_cells;
        }
    }
    if (result.occupied_cells == 0)
    {
        result.min_x = result.min_y = result.max_x = result.max_y = 0;
    }
    return result;
}

} // namespace physics_sim
