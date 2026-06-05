#include <physics_sim/math.hpp>
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

enum class BenchmarkTierFilter
{
    Live,
    Offline,
    All,
};

[[nodiscard]] const char* tier_name(physics_sim::FluidSolverQualityTier tier) noexcept
{
    return tier == physics_sim::FluidSolverQualityTier::Offline ? "offline" : "live";
}

[[nodiscard]] bool tier_matches(BenchmarkTierFilter filter, physics_sim::FluidSolverQualityTier tier) noexcept
{
    switch (filter)
    {
    case BenchmarkTierFilter::Live:
        return tier == physics_sim::FluidSolverQualityTier::Live;
    case BenchmarkTierFilter::Offline:
        return tier == physics_sim::FluidSolverQualityTier::Offline;
    case BenchmarkTierFilter::All:
        return true;
    }

    return false;
}

[[nodiscard]] BenchmarkTierFilter parse_tier_filter(int argc, char* argv[])
{
    BenchmarkTierFilter filter = BenchmarkTierFilter::Live;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if (arg == "--tier" && i + 1 < argc)
        {
            const std::string_view value = argv[++i];
            if (value == "live")
            {
                filter = BenchmarkTierFilter::Live;
            }
            else if (value == "offline")
            {
                filter = BenchmarkTierFilter::Offline;
            }
            else if (value == "all")
            {
                filter = BenchmarkTierFilter::All;
            }
            else
            {
                std::fprintf(stderr, "Unknown benchmark tier: %.*s\n", static_cast<int>(value.size()), value.data());
                std::exit(1);
            }
            continue;
        }

        std::fprintf(stderr, "Unknown command-line argument: %.*s\n", static_cast<int>(arg.size()), arg.data());
        std::exit(1);
    }

    return filter;
}

bool run_benchmark(
    const char* name,
    physics_sim::FluidSolverQualityTier tier,
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

    const double average_step_ms = (elapsed_seconds * 1000.0) / static_cast<double>(steps);

    std::printf(
        "scene=%s tier=%s grid=%zux%zu cell=%.1f steps=%d end_particles=%zu active_cells=%zu pressure_active_cells=%zu pressure_iterations=%d pressure_relative_residual=%.6f pressure_target_relative_residual=%.6f pressure_converged=%s average_density_error=%.6f max_density_error=%.6f mass_error=%.6f elapsed_seconds=%.2f average_step_ms=%.4f budget_seconds=%.2f\n",
        name,
        tier_name(tier),
        sim.grid().width(),
        sim.grid().height(),
        sim.grid().cell_size(),
        steps,
        sim.particles().size(),
        sim.metrics().active_cells,
        pressure.active_cells,
        pressure.iterations,
        pressure.relative_residual,
        pressure.target_relative_residual,
        pressure.converged ? "true" : "false",
        sim.metrics().average_density_error,
        sim.metrics().max_density_error,
        mass_error,
        elapsed_seconds,
        average_step_ms,
        budget_seconds);

    if (elapsed_seconds > budget_seconds)
    {
        std::fprintf(stderr, "Benchmark %s (%s) exceeded budget: %.2f > %.2f seconds\n", name, tier_name(tier), elapsed_seconds, budget_seconds);
        return false;
    }

    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    const BenchmarkTierFilter filter = parse_tier_filter(argc, argv);

    const auto run_if_selected = [&](physics_sim::FluidSolverQualityTier tier,
                                     const char* name,
                                     int width,
                                     int height,
                                     float cell_size,
                                     int steps,
                                     double budget_seconds,
                                     const auto& setup) -> bool
    {
        if (!tier_matches(filter, tier))
        {
            return true;
        }

        physics_sim::WaterSimulation2D sim{static_cast<std::size_t>(width), static_cast<std::size_t>(height), cell_size};
        physics_sim::FluidSolverSettings settings = tier == physics_sim::FluidSolverQualityTier::Offline
            ? physics_sim::WaterSimulation2D::offline_solver_settings()
            : physics_sim::WaterSimulation2D::live_solver_settings();

        setup(sim, settings);
        sim.set_solver_settings(settings);

        return run_benchmark(name, tier, sim, steps, budget_seconds);
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

    if (!run_if_selected(physics_sim::FluidSolverQualityTier::Live, "small-container-stress", 12, 12, 1.0f, 6000, 20.0, configure_small_container))
    {
        return 1;
    }
    if (!run_if_selected(physics_sim::FluidSolverQualityTier::Offline, "small-container-stress", 12, 12, 1.0f, 6000, 30.0, configure_small_container))
    {
        return 1;
    }

    if (!run_if_selected(physics_sim::FluidSolverQualityTier::Live, "demo-grid-flow", 80, 45, 16.0f, 240, 3.0, configure_demo_grid))
    {
        return 1;
    }
    if (!run_if_selected(physics_sim::FluidSolverQualityTier::Offline, "demo-grid-flow", 80, 45, 16.0f, 240, 6.0, configure_demo_grid))
    {
        return 1;
    }

    return 0;
}
