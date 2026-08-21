#pragma once

#include <physics_sim/state_digest.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace physics_sim
{
// Canonical runtime identity for the supported deterministic contract. Solver
// scratch buffers that are fully overwritten before they can affect the next
// step are deliberately excluded. Every exposed value that can change future
// physics, device behavior, challenge accounting, or stepping is included.
[[nodiscard]] inline std::string versioned_water_state_digest(const WaterSimulation2D &simulation, double fixed_timestep, bool paused = false,
                                                              bool single_step_pending = false)
{
    StateDigestBuilder digest{StateDigestDomain::PhysicsRuntime};
    digest.mix_string("WaterSimulation2D");
    const auto mix_size = [&digest](std::size_t value) { digest.mix_u64(static_cast<std::uint64_t>(value)); };

    const auto &grid = simulation.grid();
    digest.mix_u64(simulation.simulation_tick());
    digest.mix_double(fixed_timestep);
    digest.mix_bool(paused);
    digest.mix_bool(single_step_pending);
    mix_size(grid.width());
    mix_size(grid.height());
    digest.mix_float(grid.cell_size());

    const FluidSolverSettings &settings = simulation.solver_settings();
    digest.mix_enum(settings.profile);
    digest.mix_enum(settings.tier);
    digest.mix_float(settings.gravity_acceleration);
    digest.mix_i64(settings.pressure_max_iterations);
    digest.mix_float(settings.pressure_relative_residual_target);
    digest.mix_float(settings.rest_density);
    mix_size(settings.particles_per_full_cell);
    digest.mix_float(settings.density_kernel_radius_cells);
    digest.mix_i64(settings.density_correction_iterations);
    digest.mix_float(settings.max_density_correction_fraction);
    digest.mix_float(settings.density_correction_velocity_ratio);
    digest.mix_float(settings.flip_blend);
    digest.mix_float(settings.velocity_retention);
    digest.mix_float(settings.apic_affine_ratio);
    digest.mix_enum(settings.wall_material);
    digest.mix_float(settings.wall_tangential_velocity_retention);
    digest.mix_float(settings.viscosity_coefficient);
    digest.mix_float(settings.surface_tension_coefficient);
    digest.mix_float(settings.max_surface_velocity_delta_fraction);
    digest.mix_bool(settings.resampling.enabled);
    mix_size(settings.resampling.min_particles_per_fluid_cell);
    mix_size(settings.resampling.target_particles_per_fluid_cell);
    mix_size(settings.resampling.max_particles_per_fluid_cell);
    mix_size(settings.resampling.max_resampling_operations_per_step);
    digest.mix_float(settings.resampling.split_offset_fraction);
    digest.mix_float(settings.resampling.min_split_particle_mass);
    digest.mix_bool(settings.regularization.enabled);
    mix_size(settings.regularization.minimum_particle_count);
    digest.mix_u64(settings.regularization.interval_ticks);
    digest.mix_i64(settings.regularization.iterations);
    digest.mix_float(settings.regularization.support_radius_cells);
    digest.mix_float(settings.regularization.strength);
    digest.mix_float(settings.regularization.max_displacement_fraction);
    digest.mix_u64(settings.density_metrics_interval_ticks);

    const auto &particles = simulation.particles();
    mix_size(particles.size());
    for (const FluidParticle &particle : particles)
    {
        digest.mix_float(particle.position.x);
        digest.mix_float(particle.position.y);
        digest.mix_float(particle.velocity.x);
        digest.mix_float(particle.velocity.y);
        digest.mix_float(particle.mass);
        digest.mix_float(particle.volume);
        digest.mix_float(particle.density);
        digest.mix_float(particle.affine_velocity.m00);
        digest.mix_float(particle.affine_velocity.m01);
        digest.mix_float(particle.affine_velocity.m10);
        digest.mix_float(particle.affine_velocity.m11);
        mix_size(particle.neighbor_count);
    }

    mix_size(grid.pressure_values().size());
    for (const float value : grid.pressure_values())
        digest.mix_float(value);
    mix_size(grid.divergence_values().size());
    for (const float value : grid.divergence_values())
        digest.mix_float(value);
    mix_size(grid.u_values().size());
    for (const float value : grid.u_values())
        digest.mix_float(value);
    mix_size(grid.v_values().size());
    for (const float value : grid.v_values())
        digest.mix_float(value);
    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            digest.mix_bool(grid.solid(x, y));
            digest.mix_float(simulation.cell_volume_fraction(x, y));
            digest.mix_float(simulation.cell_density(x, y));
        }
    }

    const auto &emitters = simulation.emitters();
    mix_size(emitters.size());
    for (const WaterEmitter &emitter : emitters)
    {
        digest.mix_enum(emitter.kind);
        digest.mix_float(emitter.position.x);
        digest.mix_float(emitter.position.y);
        digest.mix_float(emitter.direction.x);
        digest.mix_float(emitter.direction.y);
        digest.mix_float(emitter.speed);
        digest.mix_float(emitter.emission_rate);
        digest.mix_bool(emitter.enabled);
        digest.mix_double(emitter.emission_accumulator);
        digest.mix_u64(emitter.emitted_particles);
    }

    const auto &gates = simulation.gates();
    mix_size(gates.size());
    for (const WaterGate &gate : gates)
    {
        mix_size(gate.x);
        mix_size(gate.y);
        digest.mix_bool(gate.open);
    }

    const auto &sensors = simulation.sensors();
    mix_size(sensors.size());
    for (const WaterSensor &sensor : sensors)
    {
        mix_size(sensor.x);
        mix_size(sensor.y);
        mix_size(sensor.width);
        mix_size(sensor.height);
        digest.mix_bool(sensor.enabled);
        digest.mix_bool(sensor.active);
        digest.mix_bool(sensor.objective);
        digest.mix_string(sensor.label);
    }

    const auto &drains = simulation.drains();
    mix_size(drains.size());
    for (const WaterDrain &drain : drains)
    {
        mix_size(drain.x);
        mix_size(drain.y);
        mix_size(drain.width);
        mix_size(drain.height);
        digest.mix_bool(drain.enabled);
    }

    const auto &pumps = simulation.pumps();
    mix_size(pumps.size());
    for (const WaterPump &pump : pumps)
    {
        mix_size(pump.x);
        mix_size(pump.y);
        mix_size(pump.width);
        mix_size(pump.height);
        digest.mix_bool(pump.enabled);
        digest.mix_float(pump.direction.x);
        digest.mix_float(pump.direction.y);
        digest.mix_float(pump.strength);
    }

    const auto &valves = simulation.valves();
    mix_size(valves.size());
    for (const WaterValve &valve : valves)
    {
        mix_size(valve.x);
        mix_size(valve.y);
        digest.mix_bool(valve.open);
    }

    const WaterSimulationMetrics &metrics = simulation.metrics();
    digest.mix_u64(metrics.total_emitted);
    digest.mix_double(metrics.total_emitted_mass);
    digest.mix_u64(metrics.total_removed);
    digest.mix_double(metrics.total_removed_mass);
    digest.mix_u64(metrics.total_outflow);
    digest.mix_double(metrics.total_outflow_mass);
    mix_size(metrics.active_particles);
    mix_size(metrics.active_cells);
    mix_size(metrics.visible_fluid_cells);
    mix_size(metrics.pressure_active_cells);
    digest.mix_double(metrics.active_cell_overreach_ratio);
    mix_size(metrics.active_sensors);
    mix_size(metrics.objective_sensors);
    digest.mix_bool(metrics.objective_completed);
    digest.mix_double(metrics.average_divergence_after_projection);
    digest.mix_double(metrics.max_divergence_after_projection);
    digest.mix_double(metrics.min_density);
    digest.mix_double(metrics.max_density);
    digest.mix_double(metrics.average_density);
    digest.mix_double(metrics.average_density_error);
    digest.mix_double(metrics.max_density_error);
    digest.mix_double(metrics.average_neighbor_count);
    mix_size(metrics.max_neighbor_count);
    digest.mix_double(metrics.kinetic_energy);

    const PressureSolveResult &pressure = metrics.pressure_solve;
    digest.mix_i64(pressure.iterations);
    digest.mix_i64(pressure.max_iterations);
    digest.mix_float(pressure.target_relative_residual);
    digest.mix_float(pressure.target_absolute_residual);
    digest.mix_float(pressure.initial_residual);
    digest.mix_float(pressure.final_residual);
    digest.mix_float(pressure.absolute_residual);
    digest.mix_float(pressure.relative_residual);
    digest.mix_float(pressure.rhs_l2);
    digest.mix_float(pressure.solution_l2);
    digest.mix_float(pressure.pressure_dt);
    digest.mix_float(pressure.rest_density);
    digest.mix_bool(pressure.converged);
    mix_size(pressure.visible_cells);
    mix_size(pressure.active_cells);
    mix_size(pressure.pressure_active_cells);
    digest.mix_double(pressure.active_cell_overreach_ratio);

    return digest.finish();
}
} // namespace physics_sim
