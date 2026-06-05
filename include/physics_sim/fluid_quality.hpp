#pragma once

#include <physics_sim/math.hpp>
#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace physics_sim
{
struct FluidQualitySnapshot
{
    std::string scenario{};
    std::uint64_t tick = 0;
    WaterSimulationMetrics metrics{};
    std::size_t particle_count = 0;
    double average_speed = 0.0;
    double max_speed = 0.0;
    double kinetic_energy = 0.0;
    Vec2 center_of_mass{};
    Vec2 min_position{};
    Vec2 max_position{};
    std::size_t particles_in_solids = 0;
    std::size_t particles_out_of_domain = 0;
    double pooled_height = 0.0;
    double min_density = 0.0;
    double max_density = 0.0;
    double average_density = 0.0;
    double average_density_error = 0.0;
    double max_density_error = 0.0;
    double average_neighbor_count = 0.0;
    std::size_t max_neighbor_count = 0;
    double total_particle_mass = 0.0;
    double total_particle_volume = 0.0;
    double mass_error = 0.0;
    double momentum_x = 0.0;
    double momentum_y = 0.0;
    double divergence_l2 = 0.0;
    double pressure_l2 = 0.0;
    double hydrostatic_pressure_shape_error = 0.0;
    double surface_height = 0.0;
    double surface_height_jitter = 0.0;
    double max_volume_fraction = 0.0;
    double average_volume_fraction = 0.0;
};

struct FluidQualityScenarioResult
{
    std::vector<FluidQualitySnapshot> snapshots{};
    WaterSimulation2D final_state{};
};

using FluidQualitySetupCallback = std::function<void(WaterSimulation2D&)>;
using FluidQualityTickCallback = std::function<void(WaterSimulation2D&, std::uint64_t)>;

[[nodiscard]] inline double sum_particle_mass(const std::vector<FluidParticle>& particles) noexcept
{
    double total = 0.0;
    for (const auto& particle : particles)
    {
        total += static_cast<double>(particle.mass > 0.0f ? particle.mass : 0.0f);
    }
    return total;
}

[[nodiscard]] inline double sum_particle_volume(const std::vector<FluidParticle>& particles) noexcept
{
    double total = 0.0;
    for (const auto& particle : particles)
    {
        total += static_cast<double>(particle.volume > 0.0f ? particle.volume : 0.0f);
    }
    return total;
}

[[nodiscard]] inline Vec2 sum_particle_momentum(const std::vector<FluidParticle>& particles) noexcept
{
    Vec2 momentum{};
    for (const auto& particle : particles)
    {
        momentum.x += particle.velocity.x * particle.mass;
        momentum.y += particle.velocity.y * particle.mass;
    }
    return momentum;
}

[[nodiscard]] inline FluidQualitySnapshot capture_fluid_quality_snapshot(
    WaterSimulation2D& simulation,
    std::string scenario_name,
    std::uint64_t tick,
    std::optional<float> basin_floor_y = std::nullopt,
    std::optional<std::size_t> pool_width_cells = std::nullopt,
    double initial_particle_mass = 0.0)
{
    simulation.refresh_density_metrics();
    simulation.refresh_cell_classification();

    FluidQualitySnapshot snapshot;
    snapshot.scenario = std::move(scenario_name);
    snapshot.tick = tick;
    snapshot.metrics = simulation.metrics();
    snapshot.particle_count = simulation.particles().size();
    snapshot.min_density = snapshot.metrics.min_density;
    snapshot.max_density = snapshot.metrics.max_density;
    snapshot.average_density = snapshot.metrics.average_density;
    snapshot.average_density_error = snapshot.metrics.average_density_error;
    snapshot.max_density_error = snapshot.metrics.max_density_error;
    snapshot.average_neighbor_count = snapshot.metrics.average_neighbor_count;
    snapshot.max_neighbor_count = snapshot.metrics.max_neighbor_count;
    snapshot.total_particle_mass = sum_particle_mass(simulation.particles());
    snapshot.total_particle_volume = sum_particle_volume(simulation.particles());
    const Vec2 total_momentum = sum_particle_momentum(simulation.particles());
    snapshot.momentum_x = total_momentum.x;
    snapshot.momentum_y = total_momentum.y;

    if (!simulation.particles().empty())
    {
        snapshot.min_position = simulation.particles().front().position;
        snapshot.max_position = simulation.particles().front().position;

        double speed_sum = 0.0;
        double speed_max = 0.0;
        double kinetic_energy_sum = 0.0;
        std::size_t solids = 0;
        std::size_t out_of_domain = 0;

        const auto& grid = simulation.grid();
        const float cell_size = grid.cell_size();
        const float domain_width = static_cast<float>(grid.width()) * cell_size;
        const float domain_height = static_cast<float>(grid.height()) * cell_size;

        for (const auto& particle : simulation.particles())
        {
            const double speed = std::sqrt(static_cast<double>(particle.velocity.x) * static_cast<double>(particle.velocity.x)
                + static_cast<double>(particle.velocity.y) * static_cast<double>(particle.velocity.y));
            speed_sum += speed;
            speed_max = std::max(speed_max, speed);
            kinetic_energy_sum += 0.5 * speed * speed;

            snapshot.min_position.x = std::min(snapshot.min_position.x, particle.position.x);
            snapshot.min_position.y = std::min(snapshot.min_position.y, particle.position.y);
            snapshot.max_position.x = std::max(snapshot.max_position.x, particle.position.x);
            snapshot.max_position.y = std::max(snapshot.max_position.y, particle.position.y);

            if (particle.position.x < 0.0f
                || particle.position.y < 0.0f
                || particle.position.x >= domain_width
                || particle.position.y >= domain_height)
            {
                ++out_of_domain;
                continue;
            }

            const std::size_t cell_x = static_cast<std::size_t>(std::floor(particle.position.x / cell_size));
            const std::size_t cell_y = static_cast<std::size_t>(std::floor(particle.position.y / cell_size));
            if (grid.contains(cell_x, cell_y) && grid.solid(cell_x, cell_y))
            {
                ++solids;
            }
        }

        const double particle_count = static_cast<double>(simulation.particles().size());
        snapshot.center_of_mass = particle_center_of_mass(simulation.particles());
        snapshot.average_speed = speed_sum / particle_count;
        snapshot.max_speed = speed_max;
        snapshot.kinetic_energy = kinetic_energy_sum;
        snapshot.particles_in_solids = solids;
        snapshot.particles_out_of_domain = out_of_domain;
        snapshot.surface_height = 0.0;

        if (basin_floor_y.has_value())
        {
            const double span_height = (static_cast<double>(snapshot.max_position.y) - static_cast<double>(snapshot.min_position.y)) / static_cast<double>(cell_size);
            const double floor_depth = std::max(0.0, (static_cast<double>(*basin_floor_y) - static_cast<double>(snapshot.center_of_mass.y)) / static_cast<double>(cell_size));
            snapshot.pooled_height = std::max(span_height, floor_depth);
        }

        if (pool_width_cells.has_value() && *pool_width_cells > 0)
        {
            // Occupied MAC-grid cells usually overcount the visible pool height because the solver keeps a
            // conservative raster of fluid support cells. Normalize by two so this proxy stays in height units.
            const double occupancy_depth = static_cast<double>(snapshot.metrics.active_cells)
                / (static_cast<double>(*pool_width_cells) * 2.0);
            snapshot.pooled_height = std::max(snapshot.pooled_height, occupancy_depth);
        }

        const double span_height = (static_cast<double>(snapshot.max_position.y) - static_cast<double>(snapshot.min_position.y)) / static_cast<double>(cell_size);
        snapshot.surface_height = snapshot.pooled_height > 0.0 ? snapshot.pooled_height : std::max(0.0, span_height);
    }

    const auto& grid = simulation.grid();
    const auto& pressure_values = grid.pressure_values();
    const auto& divergence_values = grid.divergence_values();
    double pressure_sum_sq = 0.0;
    double divergence_sum_sq = 0.0;
    double max_volume_fraction = 0.0;
    double average_volume_fraction = 0.0;
    std::size_t fluid_cell_count = 0;
    std::vector<std::pair<double, double>> pressure_depth_samples;
    pressure_depth_samples.reserve(grid.cell_count());

    double top_fluid_center_y = std::numeric_limits<double>::max();
    double bottom_fluid_center_y = std::numeric_limits<double>::lowest();
    double min_pressure = std::numeric_limits<double>::max();
    double max_pressure = std::numeric_limits<double>::lowest();

    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            if (simulation.cell_state(x, y) != FluidCellState::Fluid)
            {
                continue;
            }

            const std::size_t idx = grid.cell_index(x, y);
            const double pressure = static_cast<double>(pressure_values[idx]);
            const double divergence = static_cast<double>(divergence_values[idx]);
            const double volume_fraction = static_cast<double>(simulation.cell_volume_fraction(x, y));
            const double cell_center_y = (static_cast<double>(y) + 0.5) * static_cast<double>(grid.cell_size());

            pressure_sum_sq += pressure * pressure;
            divergence_sum_sq += divergence * divergence;
            average_volume_fraction += volume_fraction;
            max_volume_fraction = std::max(max_volume_fraction, volume_fraction);
            min_pressure = std::min(min_pressure, pressure);
            max_pressure = std::max(max_pressure, pressure);
            top_fluid_center_y = std::min(top_fluid_center_y, cell_center_y);
            bottom_fluid_center_y = std::max(bottom_fluid_center_y, cell_center_y);
            pressure_depth_samples.emplace_back(cell_center_y, pressure);
            ++fluid_cell_count;
        }
    }

    if (fluid_cell_count > 0)
    {
        snapshot.pressure_l2 = std::sqrt(pressure_sum_sq / static_cast<double>(fluid_cell_count));
        snapshot.divergence_l2 = std::sqrt(divergence_sum_sq / static_cast<double>(fluid_cell_count));
        snapshot.max_volume_fraction = max_volume_fraction;
        snapshot.average_volume_fraction = average_volume_fraction / static_cast<double>(fluid_cell_count);

        if (bottom_fluid_center_y > top_fluid_center_y && max_pressure > min_pressure)
        {
            const double depth_span = bottom_fluid_center_y - top_fluid_center_y;
            const double pressure_span = max_pressure - min_pressure;
            double shape_error_sum = 0.0;

            for (const auto& sample : pressure_depth_samples)
            {
                const double actual_depth_fraction = std::clamp((sample.first - top_fluid_center_y) / depth_span, 0.0, 1.0);
                const double actual_pressure_fraction = std::clamp((sample.second - min_pressure) / pressure_span, 0.0, 1.0);
                const double delta = actual_pressure_fraction - actual_depth_fraction;
                shape_error_sum += delta * delta;
            }

            snapshot.hydrostatic_pressure_shape_error = std::sqrt(shape_error_sum / static_cast<double>(fluid_cell_count));
        }
    }

    const double denominator = std::max(initial_particle_mass + snapshot.metrics.total_emitted_mass, 1.0e-9);
    snapshot.mass_error = std::abs(
        snapshot.total_particle_mass
        - initial_particle_mass
        - snapshot.metrics.total_emitted_mass
        + snapshot.metrics.total_removed_mass
        + snapshot.metrics.total_outflow_mass) / denominator;

    return snapshot;
}

[[nodiscard]] inline FluidQualityScenarioResult run_fluid_quality_scenario(
    std::string scenario_name,
    WaterSimulation2D simulation,
    std::vector<std::uint64_t> sample_ticks,
    const FluidQualitySetupCallback& setup,
    const FluidQualityTickCallback& tick_callback = {},
    std::optional<float> basin_floor_y = std::nullopt,
    std::optional<std::size_t> pool_width_cells = std::nullopt)
{
    std::sort(sample_ticks.begin(), sample_ticks.end());
    sample_ticks.erase(std::unique(sample_ticks.begin(), sample_ticks.end()), sample_ticks.end());

    if (setup)
    {
        setup(simulation);
    }

    if (tick_callback)
    {
        tick_callback(simulation, 0);
    }

    simulation.ensure_particle_transfer_properties();
    const double initial_particle_mass = sum_particle_mass(simulation.particles());

    FluidQualityScenarioResult result;
    result.snapshots.reserve(sample_ticks.size());

    std::size_t next_sample_index = 0;
    if (!sample_ticks.empty() && sample_ticks.front() == 0)
    {
        result.snapshots.push_back(capture_fluid_quality_snapshot(simulation, scenario_name, 0, basin_floor_y, pool_width_cells, initial_particle_mass));
        ++next_sample_index;
    }

    const std::uint64_t max_tick = sample_ticks.empty() ? 0 : sample_ticks.back();
    for (std::uint64_t tick = 1; tick <= max_tick; ++tick)
    {
        if (tick_callback)
        {
            tick_callback(simulation, tick);
        }

        simulation.step(1.0 / 120.0);

        while (next_sample_index < sample_ticks.size() && sample_ticks[next_sample_index] == tick)
        {
            result.snapshots.push_back(capture_fluid_quality_snapshot(simulation, scenario_name, tick, basin_floor_y, pool_width_cells, initial_particle_mass));
            ++next_sample_index;
        }
    }

    for (std::size_t index = 0; index < result.snapshots.size(); ++index)
    {
        if (index == 0)
        {
            result.snapshots[index].surface_height_jitter = 0.0;
            continue;
        }

        result.snapshots[index].surface_height_jitter = std::abs(result.snapshots[index].surface_height - result.snapshots[index - 1].surface_height);
    }

    result.final_state = std::move(simulation);
    return result;
}
} // namespace physics_sim
