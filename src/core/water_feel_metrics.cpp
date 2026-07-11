#include <physics_sim/water_feel_metrics.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace physics_sim
{
namespace
{
[[nodiscard]] Vec2 cell_center_velocity(const MacGrid2D& grid, std::size_t x, std::size_t y)
{
    return {
        (grid.velocity_u(x, y) + grid.velocity_u(x + 1, y)) * 0.5f,
        (grid.velocity_v(x, y) + grid.velocity_v(x, y + 1)) * 0.5f,
    };
}
} // namespace

WaterFeelMetrics measure_water_feel(std::span<const FluidParticle> particles, const MacGrid2D& grid)
{
    WaterFeelMetrics result;
    if (particles.empty() || grid.width() == 0 || grid.height() == 0 || grid.cell_size() <= 0.0f)
    {
        return result;
    }

    const double cell_size = static_cast<double>(grid.cell_size());
    const double domain_width = static_cast<double>(grid.width()) * cell_size;
    const double domain_height = static_cast<double>(grid.height()) * cell_size;
    std::vector<std::size_t> cell_particle_counts(grid.cell_count(), 0);
    std::vector<double> column_surface(grid.width(), std::numeric_limits<double>::infinity());
    double min_x = std::numeric_limits<double>::infinity();
    double max_x = -std::numeric_limits<double>::infinity();
    std::size_t valid_particles = 0;

    for (const FluidParticle& particle : particles)
    {
        const double x = static_cast<double>(particle.position.x);
        const double y = static_cast<double>(particle.position.y);
        if (!std::isfinite(x) || !std::isfinite(y) || x < 0.0 || y < 0.0 || x >= domain_width || y >= domain_height)
        {
            continue;
        }

        const std::size_t cell_x = std::min(static_cast<std::size_t>(x / cell_size), grid.width() - 1);
        const std::size_t cell_y = std::min(static_cast<std::size_t>(y / cell_size), grid.height() - 1);
        if (grid.solid(cell_x, cell_y))
        {
            continue;
        }

        ++cell_particle_counts[grid.cell_index(cell_x, cell_y)];
        column_surface[cell_x] = std::min(column_surface[cell_x], y / cell_size);
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        ++valid_particles;
    }

    if (valid_particles == 0)
    {
        return result;
    }

    result.horizontal_footprint_cells = std::max(0.0, (max_x - min_x) / cell_size);
    double slope_sum_squared = 0.0;
    std::size_t slope_samples = 0;
    for (std::size_t x = 0; x < column_surface.size(); ++x)
    {
        if (std::isfinite(column_surface[x]))
        {
            ++result.occupied_columns;
        }
        if (x == 0 || !std::isfinite(column_surface[x - 1]) || !std::isfinite(column_surface[x]))
        {
            continue;
        }

        const double slope = std::abs(column_surface[x] - column_surface[x - 1]);
        slope_sum_squared += slope * slope;
        result.surface_max_slope = std::max(result.surface_max_slope, slope);
        ++slope_samples;
    }
    if (slope_samples > 0)
    {
        result.surface_rms_slope = std::sqrt(slope_sum_squared / static_cast<double>(slope_samples));
    }

    double mean_cell_count = 0.0;
    std::size_t occupied_cell_count = 0;
    for (const std::size_t count : cell_particle_counts)
    {
        if (count > 0)
        {
            mean_cell_count += static_cast<double>(count);
            ++occupied_cell_count;
        }
    }
    if (occupied_cell_count > 0)
    {
        mean_cell_count /= static_cast<double>(occupied_cell_count);
        double variance = 0.0;
        for (const std::size_t count : cell_particle_counts)
        {
            if (count > 0)
            {
                const double delta = static_cast<double>(count) - mean_cell_count;
                variance += delta * delta;
            }
        }
        variance /= static_cast<double>(occupied_cell_count);
        result.particle_count_coefficient_of_variation =
            mean_cell_count > 0.0 ? std::sqrt(variance) / mean_cell_count : 0.0;
    }

    std::vector<unsigned char> visited(grid.cell_count(), 0);
    std::vector<std::size_t> pending;
    std::size_t largest_component_particles = 0;
    for (std::size_t start = 0; start < cell_particle_counts.size(); ++start)
    {
        if (cell_particle_counts[start] == 0 || visited[start] != 0)
        {
            continue;
        }

        ++result.particle_components;
        pending.clear();
        pending.push_back(start);
        visited[start] = 1;
        std::size_t component_particles = 0;
        while (!pending.empty())
        {
            const std::size_t current = pending.back();
            pending.pop_back();
            component_particles += cell_particle_counts[current];
            const std::size_t x = current % grid.width();
            const std::size_t y = current / grid.width();

            const auto visit = [&](std::size_t neighbor)
            {
                if (cell_particle_counts[neighbor] > 0 && visited[neighbor] == 0)
                {
                    visited[neighbor] = 1;
                    pending.push_back(neighbor);
                }
            };
            for (int offset_y = -1; offset_y <= 1; ++offset_y)
            {
                for (int offset_x = -1; offset_x <= 1; ++offset_x)
                {
                    if (offset_x == 0 && offset_y == 0)
                    {
                        continue;
                    }
                    const int neighbor_x = static_cast<int>(x) + offset_x;
                    const int neighbor_y = static_cast<int>(y) + offset_y;
                    if (neighbor_x >= 0 && neighbor_y >= 0
                        && neighbor_x < static_cast<int>(grid.width())
                        && neighbor_y < static_cast<int>(grid.height()))
                    {
                        visit(grid.cell_index(static_cast<std::size_t>(neighbor_x), static_cast<std::size_t>(neighbor_y)));
                    }
                }
            }
        }
        largest_component_particles = std::max(largest_component_particles, component_particles);
    }
    result.largest_component_particle_fraction =
        static_cast<double>(largest_component_particles) / static_cast<double>(valid_particles);

    double vorticity_sum_squared = 0.0;
    std::size_t vorticity_samples = 0;
    for (std::size_t y = 1; y + 1 < grid.height(); ++y)
    {
        for (std::size_t x = 1; x + 1 < grid.width(); ++x)
        {
            if (cell_particle_counts[grid.cell_index(x, y)] == 0)
            {
                continue;
            }
            const Vec2 left = cell_center_velocity(grid, x - 1, y);
            const Vec2 right = cell_center_velocity(grid, x + 1, y);
            const Vec2 top = cell_center_velocity(grid, x, y - 1);
            const Vec2 bottom = cell_center_velocity(grid, x, y + 1);
            const double dv_dx = static_cast<double>(right.y - left.y) / (2.0 * cell_size);
            const double du_dy = static_cast<double>(bottom.x - top.x) / (2.0 * cell_size);
            const double vorticity = dv_dx - du_dy;
            vorticity_sum_squared += vorticity * vorticity;
            ++vorticity_samples;
        }
    }
    if (vorticity_samples > 0)
    {
        result.vorticity_rms = std::sqrt(vorticity_sum_squared / static_cast<double>(vorticity_samples));
    }

    return result;
}
} // namespace physics_sim
