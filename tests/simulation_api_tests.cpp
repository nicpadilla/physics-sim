#include <physics_sim/simulation.hpp>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace
{
void require(bool condition, const char *message)
{
    if (!condition)
    {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}
} // namespace

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
    simulation.apply(physics_sim::AddEmitterCommand{physics_sim::SimulationEmitterKind::Directional, {6.0f, 2.0f}, {0.0f, 1.0f}, 4.0f, 120.0f, true});
    simulation.apply(physics_sim::SeedParticleCommand{{3.5f, 3.5f}, {0.0f, 0.0f}});
    repeated.apply(physics_sim::AddEmitterCommand{physics_sim::SimulationEmitterKind::Directional, {6.0f, 2.0f}, {0.0f, 1.0f}, 4.0f, 120.0f, true});
    repeated.apply(physics_sim::SeedParticleCommand{{3.5f, 3.5f}, {0.0f, 0.0f}});

    require(physics_sim::Simulation::versioned_state_digest_version() == physics_sim::StateDigestFormatVersion,
            "facade digest version does not match the shared format");
    require(simulation.versioned_state_digest() == repeated.versioned_state_digest(), "identical initial simulations produced different versioned digests");

    require(simulation.step(), "running simulation did not advance");
    require(repeated.step(), "repeated simulation did not advance");
    require(simulation.state_digest() == repeated.state_digest(), "identical simulations produced different legacy state digests");
    require(simulation.versioned_state_digest() == repeated.versioned_state_digest(), "identical simulations produced different versioned state digests");
    auto snapshot = simulation.snapshot();
    require(snapshot.tick == 1, "snapshot tick did not advance");
    require(snapshot.grid_width == 8 && snapshot.grid_height == 6, "snapshot grid differs from config");
    require(snapshot.solid_cells.at(3 * 8 + 2) == 1, "solid command was not reflected in snapshot");
    require(snapshot.densities.size() == 48 && snapshot.pressures.size() == 48 && snapshot.divergences.size() == 48 && snapshot.velocities.size() == 48,
            "diagnostic fields were not exposed through the snapshot");
    require(!snapshot.particles.empty(), "emitter command produced no particles");
    require(simulation.metrics().tick == snapshot.tick, "metrics and snapshot ticks differ");

    const std::string running_digest = simulation.versioned_state_digest();
    simulation.apply(physics_sim::SetPausedCommand{true});
    require(simulation.paused(), "pause command did not update the facade state");
    require(simulation.versioned_state_digest() != running_digest, "pause state did not affect versioned runtime identity");
    const std::string paused_digest = simulation.versioned_state_digest();
    require(!simulation.step(), "paused simulation advanced without a single-step command");
    simulation.apply(physics_sim::SingleStepCommand{});
    require(simulation.single_step_pending(), "single-step command did not become pending");
    require(simulation.versioned_state_digest() != paused_digest, "pending single step did not affect versioned runtime identity");
    require(simulation.step(), "single-step command did not advance paused simulation");
    require(simulation.paused(), "single step unexpectedly resumed simulation");
    require(!simulation.single_step_pending(), "single step remained pending after execution");

    physics_sim::SimulationConfig different_gravity = config;
    different_gravity.gravity_acceleration += 1.0f;
    physics_sim::Simulation gravity_variant{different_gravity};
    physics_sim::Simulation baseline_variant{config};
    require(gravity_variant.versioned_state_digest() != baseline_variant.versioned_state_digest(),
            "gravity configuration did not affect versioned runtime identity");

    physics_sim::SimulationConfig different_timestep = config;
    different_timestep.fixed_timestep = 1.0 / 60.0;
    physics_sim::Simulation timestep_variant{different_timestep};
    require(timestep_variant.versioned_state_digest() != baseline_variant.versioned_state_digest(), "fixed timestep did not affect versioned runtime identity");

    simulation.apply(physics_sim::ClearFluidCommand{});
    require(simulation.snapshot().particles.empty(), "clear-fluid command retained particles");
    require(simulation.snapshot().solid_cells.at(3 * 8 + 2) == 1, "clear-fluid command removed scene solids");

    bool rejected = false;
    try
    {
        simulation.apply(physics_sim::SetSolidCellCommand{99, 99, true});
    }
    catch (const std::out_of_range &)
    {
        rejected = true;
    }
    require(rejected, "out-of-grid command was not rejected");

    std::puts("simulation API tests passed");
    return 0;
}
