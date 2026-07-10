#include <physics_sim/simulation.hpp>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}
}

int main()
{
    physics_sim::SimulationConfig config;
    config.grid_width = 8;
    config.grid_height = 6;
    config.cell_size = 2.0f;
    config.fixed_timestep = 1.0 / 120.0;
    config.solver_profile = physics_sim::FluidSolverProfile::Balanced;

    physics_sim::Simulation simulation{config};
    physics_sim::Simulation repeated{config};
    simulation.apply(physics_sim::SetSolidCellCommand{2, 3, true});
    repeated.apply(physics_sim::SetSolidCellCommand{2, 3, true});
    simulation.apply(physics_sim::AddEmitterCommand{
        physics_sim::SimulationEmitterKind::Directional,
        {6.0f, 2.0f},
        {0.0f, 1.0f},
        4.0f,
        120.0f,
        true});
    repeated.apply(physics_sim::AddEmitterCommand{
        physics_sim::SimulationEmitterKind::Directional,
        {6.0f, 2.0f},
        {0.0f, 1.0f},
        4.0f,
        120.0f,
        true});

    require(simulation.step(), "running simulation did not advance");
    require(repeated.step(), "repeated simulation did not advance");
    require(simulation.state_digest() == repeated.state_digest(), "identical simulations produced different state digests");
    auto snapshot = simulation.snapshot();
    require(snapshot.tick == 1, "snapshot tick did not advance");
    require(snapshot.grid_width == 8 && snapshot.grid_height == 6, "snapshot grid differs from config");
    require(snapshot.solid_cells.at(3 * 8 + 2) == 1, "solid command was not reflected in snapshot");
    require(!snapshot.particles.empty(), "emitter command produced no particles");
    require(simulation.metrics().tick == snapshot.tick, "metrics and snapshot ticks differ");

    simulation.apply(physics_sim::SetPausedCommand{true});
    require(!simulation.step(), "paused simulation advanced without a single-step command");
    simulation.apply(physics_sim::SingleStepCommand{});
    require(simulation.step(), "single-step command did not advance paused simulation");
    require(simulation.paused(), "single step unexpectedly resumed simulation");

    simulation.apply(physics_sim::ClearFluidCommand{});
    require(simulation.snapshot().particles.empty(), "clear-fluid command retained particles");
    require(simulation.snapshot().solid_cells.at(3 * 8 + 2) == 1, "clear-fluid command removed scene solids");

    bool rejected = false;
    try
    {
        simulation.apply(physics_sim::SetSolidCellCommand{99, 99, true});
    }
    catch (const std::out_of_range&)
    {
        rejected = true;
    }
    require(rejected, "out-of-grid command was not rejected");

    std::puts("simulation API tests passed");
    return 0;
}
