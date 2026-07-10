#include <physics_sim/math.hpp>
#include <physics_sim/solver_profile.hpp>
#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)

bool finite_values(const std::vector<float>& values) noexcept
{
    for (float value : values)
    {
        if (!std::isfinite(value))
        {
            return false;
        }
    }

    return true;
}

void require_finite_grid(const physics_sim::WaterSimulation2D& sim)
{
    REQUIRE(finite_values(sim.grid().pressure_values()), "pressure became non-finite during benchmark");
    REQUIRE(finite_values(sim.grid().divergence_values()), "divergence became non-finite during benchmark");
    REQUIRE(finite_values(sim.grid().u_values()), "u velocity became non-finite during benchmark");
    REQUIRE(finite_values(sim.grid().v_values()), "v velocity became non-finite during benchmark");
}

[[nodiscard]] double sum_particle_mass(const std::vector<physics_sim::FluidParticle>& particles) noexcept
{
    double total = 0.0;
    for (const auto& particle : particles)
    {
        total += static_cast<double>(particle.mass > 0.0f ? particle.mass : 0.0f);
    }
    return total;
}

[[nodiscard]] const char* tier_name(physics_sim::FluidSolverQualityTier tier) noexcept
{
    return tier == physics_sim::FluidSolverQualityTier::Offline ? "offline" : "live";
}

[[nodiscard]] std::vector<physics_sim::FluidSolverProfile> all_profiles()
{
    return {
        physics_sim::FluidSolverProfile::Fast,
        physics_sim::FluidSolverProfile::Balanced,
        physics_sim::FluidSolverProfile::Quality,
    };
}

[[nodiscard]] std::vector<physics_sim::FluidSolverProfile> parse_profile_value(std::string_view value)
{
    if (value == "all")
    {
        return all_profiles();
    }

    if (value == "fast" || value == "live")
    {
        return {physics_sim::FluidSolverProfile::Fast};
    }

    if (value == "balanced")
    {
        return {physics_sim::FluidSolverProfile::Balanced};
    }

    if (value == "quality" || value == "offline")
    {
        return {physics_sim::FluidSolverProfile::Quality};
    }

    std::fprintf(stderr, "Unknown benchmark profile: %.*s\n", static_cast<int>(value.size()), value.data());
    std::exit(1);
}

[[nodiscard]] std::vector<physics_sim::FluidSolverProfile> parse_profile_selection(int argc, char* argv[])
{
    std::vector<physics_sim::FluidSolverProfile> selected{physics_sim::FluidSolverProfile::Fast};
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if ((arg == "--profile" || arg == "--tier") && i + 1 < argc)
        {
            selected = parse_profile_value(argv[++i]);
            continue;
        }

        std::fprintf(stderr, "Unknown command-line argument: %.*s\n", static_cast<int>(arg.size()), arg.data());
        std::exit(1);
    }

    return selected;
}

[[nodiscard]] double budget_seconds_for_profile(physics_sim::FluidSolverProfile profile, bool demo_scene) noexcept
{
    switch (profile)
    {
    case physics_sim::FluidSolverProfile::Fast:
        return demo_scene ? 3.0 : 20.0;
    case physics_sim::FluidSolverProfile::Balanced:
        return demo_scene ? 4.0 : 24.0;
    case physics_sim::FluidSolverProfile::Quality:
        return demo_scene ? 6.0 : 30.0;
    }

    return demo_scene ? 4.0 : 24.0;
}

void require_finite_pressure(const physics_sim::PressureSolveResult& pressure)
{
    REQUIRE(pressure.iterations >= 0, "pressure iteration count became negative");
    REQUIRE(std::isfinite(pressure.initial_residual), "pressure initial residual became non-finite");
    REQUIRE(std::isfinite(pressure.final_residual), "pressure final residual became non-finite");
    REQUIRE(std::isfinite(pressure.relative_residual), "pressure relative residual became non-finite");
    REQUIRE(std::isfinite(pressure.absolute_residual), "pressure absolute residual became non-finite");
    REQUIRE(std::isfinite(pressure.target_relative_residual), "pressure target relative residual became non-finite");
    REQUIRE(std::isfinite(pressure.target_absolute_residual), "pressure target absolute residual became non-finite");
    REQUIRE(std::isfinite(pressure.rhs_l2), "pressure RHS norm became non-finite");
    REQUIRE(std::isfinite(pressure.solution_l2), "pressure solution norm became non-finite");
    REQUIRE(std::isfinite(pressure.pressure_dt), "pressure dt became non-finite");
    REQUIRE(std::isfinite(pressure.rest_density), "pressure rest density became non-finite");
    REQUIRE(std::isfinite(pressure.active_cell_overreach_ratio), "pressure overreach became non-finite");
}

void require_profile_acceptance(
    physics_sim::FluidSolverProfile profile,
    const physics_sim::WaterSimulation2D& sim,
    const physics_sim::PressureSolveResult& pressure,
    double mass_error)
{
    REQUIRE(pressure.converged, "pressure solve did not converge during benchmark");
    REQUIRE(std::isfinite(sim.metrics().average_density_error), "average density error became non-finite during benchmark");
    REQUIRE(std::isfinite(sim.metrics().max_density_error), "max density error became non-finite during benchmark");
    REQUIRE(std::isfinite(sim.metrics().kinetic_energy), "kinetic energy became non-finite during benchmark");
    REQUIRE(std::isfinite(mass_error), "mass error became non-finite during benchmark");
    REQUIRE(mass_error <= 1.0e-6, "benchmark mass accounting drifted");

    if (profile == physics_sim::FluidSolverProfile::Fast)
    {
        return;
    }

    // The recovery uses a full four-neighbor pressure halo. A directional halo
    // lowered this ratio but failed the fixed U-container pool-height contract.
    if (pressure.active_cell_overreach_ratio > 3.5)
    {
        char message[256];
        std::snprintf(
            message,
            sizeof(message),
            "pressure active-cell overreach exceeded the benchmark budget: overreach=%.6f",
            pressure.active_cell_overreach_ratio);
        REQUIRE(false, message);
    }

    if (profile == physics_sim::FluidSolverProfile::Balanced)
    {
        if (sim.metrics().average_density_error > 3.0)
        {
            char message[256];
            std::snprintf(
                message,
                sizeof(message),
                "balanced profile average density error exceeded budget: avg=%.6f max=%.6f",
                sim.metrics().average_density_error,
                sim.metrics().max_density_error);
            REQUIRE(false, message);
        }
        REQUIRE(sim.metrics().max_density_error <= 10.0, "balanced profile max density error exceeded budget");
        return;
    }

    REQUIRE(sim.metrics().average_density_error <= 1.0, "quality profile average density error exceeded budget");
    REQUIRE(sim.metrics().max_density_error <= 1.25, "quality profile max density error exceeded budget");
}

bool run_benchmark(
    physics_sim::FluidSolverProfile profile,
    const char* name,
    physics_sim::WaterSimulation2D& sim,
    int steps,
    double budget_seconds)
{
    sim.ensure_particle_transfer_properties();
    const double initial_mass = sum_particle_mass(sim.particles());

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < steps; ++i)
    {
        sim.step(1.0 / 120.0);
        require_finite_grid(sim);
    }
    sim.refresh_density_metrics();
    const double elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    REQUIRE(!sim.particles().empty(), "benchmark lost all particles");
    REQUIRE(sim.metrics().total_emitted > 0, "benchmark never emitted fluid");

    const auto& pressure = sim.metrics().pressure_solve;
    const double current_mass = sum_particle_mass(sim.particles());
    const double denominator = std::max(initial_mass + sim.metrics().total_emitted_mass, 1.0e-9);
    const double mass_error = std::abs(
        current_mass
        - initial_mass
        - sim.metrics().total_emitted_mass
        + sim.metrics().total_removed_mass
        + sim.metrics().total_outflow_mass) / denominator;
    const double kinetic_energy = sim.metrics().kinetic_energy;

    require_finite_pressure(pressure);
    REQUIRE(std::isfinite(kinetic_energy), "benchmark kinetic energy became non-finite");
    require_profile_acceptance(profile, sim, pressure, mass_error);

    const double average_step_ms = (elapsed_seconds * 1000.0) / static_cast<double>(steps);
    const char* profile_name = physics_sim::solver_profile_name(profile);
    const char* tier_label = tier_name(sim.solver_settings().tier);

    std::printf(
        "scene=%s profile=%s tier=%s grid=%zux%zu cell=%.1f steps=%d end_particles=%zu active_cells=%zu visible_cells=%zu pressure_active_cells=%zu active_cell_overreach=%.6f pressure_iterations=%d pressure_relative_residual=%.6f pressure_target_relative_residual=%.6f pressure_absolute_residual=%.6f pressure_target_absolute_residual=%.6f pressure_rhs_l2=%.6f pressure_solution_l2=%.6f pressure_dt=%.6f rest_density=%.6f pressure_converged=%s average_density_error=%.6f max_density_error=%.6f kinetic_energy=%.6f mass_error=%.6f elapsed_seconds=%.2f average_step_ms=%.4f budget_seconds=%.2f\n",
        name,
        profile_name,
        tier_label,
        sim.grid().width(),
        sim.grid().height(),
        sim.grid().cell_size(),
        steps,
        sim.particles().size(),
        sim.metrics().active_cells,
        sim.metrics().visible_fluid_cells,
        pressure.pressure_active_cells,
        pressure.active_cell_overreach_ratio,
        pressure.iterations,
        pressure.relative_residual,
        pressure.target_relative_residual,
        pressure.absolute_residual,
        pressure.target_absolute_residual,
        pressure.rhs_l2,
        pressure.solution_l2,
        pressure.pressure_dt,
        pressure.rest_density,
        pressure.converged ? "true" : "false",
        sim.metrics().average_density_error,
        sim.metrics().max_density_error,
        kinetic_energy,
        mass_error,
        elapsed_seconds,
        average_step_ms,
        budget_seconds);

    if (elapsed_seconds > budget_seconds)
    {
        std::fprintf(stderr, "Benchmark %s (%s/%s) exceeded budget: %.2f > %.2f seconds\n", name, profile_name, tier_label, elapsed_seconds, budget_seconds);
        return false;
    }

    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    const auto selected_profiles = parse_profile_selection(argc, argv);

    const auto run_case = [&](physics_sim::FluidSolverProfile profile,
                                     const char* name,
                                     int width,
                                     int height,
                                     float cell_size,
                                     int steps,
                                     bool demo_scene,
                                     const auto& setup) -> bool
    {
        physics_sim::WaterSimulation2D sim{static_cast<std::size_t>(width), static_cast<std::size_t>(height), cell_size};
        physics_sim::FluidSolverSettings settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(profile);

        setup(sim, settings);
        sim.set_solver_settings(settings);

        return run_benchmark(profile, name, sim, steps, budget_seconds_for_profile(profile, demo_scene));
    };

    const auto configure_small_container = [](physics_sim::WaterSimulation2D& sim, physics_sim::FluidSolverSettings&)
    {
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 3; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = physics_sim::Vec2{6.0f, 2.0f};
        emitter.direction = physics_sim::Vec2{0.0f, 1.0f};
        emitter.speed = 6.0f;
        emitter.emission_rate = 12.0f;
        sim.add_emitter(emitter);
    };

    const auto configure_demo_grid = [](physics_sim::WaterSimulation2D& sim, physics_sim::FluidSolverSettings&)
    {
        for (std::size_t y = 24; y <= 35; ++y)
        {
            sim.set_solid_cell(20, y, true);
            sim.set_solid_cell(60, y, true);
        }
        for (std::size_t x = 20; x <= 60; ++x)
        {
            sim.set_solid_cell(x, 36, true);
        }

        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = physics_sim::Vec2{640.0f, 160.0f};
        emitter.direction = physics_sim::Vec2{0.0f, 1.0f};
        emitter.speed = 8.0f;
        emitter.emission_rate = 120.0f;
        sim.add_emitter(emitter);
    };

    for (const auto profile : selected_profiles)
    {
        if (!run_case(profile, "small-container-stress", 12, 12, 1.0f, 6000, false, configure_small_container))
        {
            return 1;
        }

        if (!run_case(profile, "demo-grid-flow", 80, 45, 16.0f, 240, true, configure_demo_grid))
        {
            return 1;
        }
    }

    return 0;
}
