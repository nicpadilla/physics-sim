#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/mac_grid.hpp>

#include <vector>

namespace physics_sim
{
struct WaterState
{
    MacGrid2D grid;
    std::vector<FluidParticle> particles;

    void reset_fluid() noexcept
    {
        grid.clear_fields();
        particles.clear();
    }

    void reset_scene() noexcept
    {
        grid.clear_fields();
        grid.clear_solids();
        particles.clear();
    }
};
} // namespace physics_sim

