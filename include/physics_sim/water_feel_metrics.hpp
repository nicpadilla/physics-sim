#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/mac_grid.hpp>

#include <cstddef>
#include <span>

namespace physics_sim
{
struct WaterFeelMetrics
{
    double horizontal_footprint_cells = 0.0;
    std::size_t occupied_columns = 0;
    double surface_rms_slope = 0.0;
    double surface_max_slope = 0.0;
    double particle_count_coefficient_of_variation = 0.0;
    std::size_t particle_components = 0;
    double largest_component_particle_fraction = 0.0;
    double vorticity_rms = 0.0;
};

[[nodiscard]] WaterFeelMetrics measure_water_feel(
    std::span<const FluidParticle> particles,
    const MacGrid2D& grid);
} // namespace physics_sim
