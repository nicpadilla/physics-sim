#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/fluid_density.hpp>
#include <physics_sim/fluid_cell_classification.hpp>
#include <physics_sim/grid_transfer.hpp>
#include <physics_sim/math.hpp>

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

// This test file intentionally compiles against the desired water-simulation API
// before the implementation exists. The first build should fail here.
#include <physics_sim/water_simulation.hpp>

namespace
{
bool nearly_equal(double lhs, double rhs, double epsilon = 0.0005) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}

bool finite_vec2(const physics_sim::Vec2& value) noexcept
{
    return std::isfinite(value.x) && std::isfinite(value.y);
}

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

physics_sim::Vec2 sum_momentum(const std::vector<physics_sim::FluidParticle>& particles) noexcept
{
    physics_sim::Vec2 momentum{};
    for (const auto& particle : particles)
    {
        momentum.x += particle.velocity.x * particle.mass;
        momentum.y += particle.velocity.y * particle.mass;
    }

    return momentum;
}

[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}
} // namespace

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)

int main()
{
    using physics_sim::Vec2;

    {
        physics_sim::FluidDensitySettings settings;
        settings.rest_density = 1.0f;
        settings.particle_volume = 1.0f;
        settings.kernel_radius = 1.0f;

        std::vector<physics_sim::FluidParticle> particles{
            {Vec2{4.0f, 4.0f}, Vec2{}},
            {Vec2{5.0f, 4.0f}, Vec2{}},
            {Vec2{20.0f, 20.0f}, Vec2{}},
        };

        const auto first_metrics = physics_sim::update_particle_density_metrics(particles, settings);
        const double first_density_a = particles[0].density;
        const double first_density_b = particles[1].density;
        const double first_density_far = particles[2].density;

        REQUIRE(nearly_equal(particles[0].mass, settings.rest_density * settings.particle_volume), "particle mass was not initialized from rest density and volume");
        REQUIRE(nearly_equal(particles[0].volume, settings.particle_volume), "particle volume was not initialized from density settings");
        REQUIRE(particles[0].neighbor_count == 1, "first particle did not count its symmetric neighbor");
        REQUIRE(particles[1].neighbor_count == 1, "second particle did not count its symmetric neighbor");
        REQUIRE(particles[2].neighbor_count == 0, "far particle counted an out-of-kernel neighbor");
        REQUIRE(nearly_equal(first_density_a, first_density_b, 0.000001), "symmetric particles produced different density estimates");
        REQUIRE(first_density_a > first_density_far, "neighbor density did not exceed isolated self density");
        REQUIRE(first_metrics.min_density > 0.0, "density metrics did not record a minimum density");
        REQUIRE(first_metrics.max_density >= first_metrics.min_density, "density metrics min/max are inconsistent");
        REQUIRE(first_metrics.max_density_error >= first_metrics.average_density_error, "density error metrics are inconsistent");

        const auto second_metrics = physics_sim::update_particle_density_metrics(particles, settings);
        REQUIRE(nearly_equal(first_density_a, particles[0].density, 0.000001), "density update was not repeatable for the first particle");
        REQUIRE(nearly_equal(first_density_b, particles[1].density, 0.000001), "density update was not repeatable for the second particle");
        REQUIRE(nearly_equal(first_density_far, particles[2].density, 0.000001), "density update was not repeatable for the far particle");
        REQUIRE(nearly_equal(first_metrics.average_density, second_metrics.average_density, 0.000001), "average density metric changed between repeated updates");
    }

    {
        const auto live_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        const auto offline_settings = physics_sim::WaterSimulation2D::offline_solver_settings();
        const auto fast_settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Fast);
        const auto balanced_settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Balanced);
        const auto quality_settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Quality);

        REQUIRE(live_settings.tier == physics_sim::FluidSolverQualityTier::Live, "live solver settings did not report live tier");
        REQUIRE(offline_settings.tier == physics_sim::FluidSolverQualityTier::Offline, "offline solver settings did not report offline tier");
        REQUIRE(fast_settings.profile == physics_sim::FluidSolverProfile::Fast, "fast solver settings did not report fast profile");
        REQUIRE(balanced_settings.profile == physics_sim::FluidSolverProfile::Balanced, "balanced solver settings did not report balanced profile");
        REQUIRE(quality_settings.profile == physics_sim::FluidSolverProfile::Quality, "quality solver settings did not report quality profile");
        REQUIRE(live_settings.profile == physics_sim::FluidSolverProfile::Fast, "live solver settings did not map to the fast profile");
        REQUIRE(offline_settings.profile == physics_sim::FluidSolverProfile::Quality, "offline solver settings did not map to the quality profile");
        REQUIRE(live_settings.pressure_max_iterations == fast_settings.pressure_max_iterations, "live solver settings did not preserve benchmark compatibility");
        REQUIRE(live_settings.pressure_relative_residual_target > offline_settings.pressure_relative_residual_target, "offline solver settings did not tighten the pressure target");
        REQUIRE(balanced_settings.tier == physics_sim::FluidSolverQualityTier::Live, "balanced solver settings did not report the live tier");
        REQUIRE(balanced_settings.particles_per_full_cell == 4, "balanced solver settings did not preserve the target particle density");
        REQUIRE(balanced_settings.density_correction_iterations == 1, "balanced solver settings did not enable a single density correction pass");
        REQUIRE(nearly_equal(balanced_settings.max_density_correction_fraction, 0.025f, 0.000001), "balanced solver settings did not cap density correction tightly enough");
        REQUIRE(balanced_settings.resampling.enabled, "balanced solver settings did not enable resampling");
        REQUIRE(balanced_settings.resampling.min_particles_per_fluid_cell == 1, "balanced solver settings did not lower the minimum particle count");
        REQUIRE(balanced_settings.resampling.target_particles_per_fluid_cell == 4, "balanced solver settings did not keep the target particle count");
        REQUIRE(balanced_settings.resampling.max_particles_per_fluid_cell == 8, "balanced solver settings did not keep the maximum particle count");
        REQUIRE(balanced_settings.resampling.max_resampling_operations_per_step == 48, "balanced solver settings did not cap resampling operations");
        REQUIRE(nearly_equal(balanced_settings.flip_blend, 0.55f, 0.000001), "balanced solver settings did not tune the FLIP blend");
        REQUIRE(nearly_equal(balanced_settings.velocity_retention, 0.88f, 0.000001), "balanced solver settings did not tune the velocity retention");
        REQUIRE(nearly_equal(balanced_settings.viscosity_coefficient, 0.025f, 0.000001), "balanced solver settings did not enable the target viscosity");
        REQUIRE(nearly_equal(balanced_settings.apic_affine_ratio, 0.0f, 0.000001), "balanced solver settings did not keep APIC disabled");
        REQUIRE(quality_settings.density_correction_iterations > live_settings.density_correction_iterations, "quality solver settings did not enable density correction");
        REQUIRE(quality_settings.resampling.enabled, "quality solver settings did not enable particle resampling");
        REQUIRE(quality_settings.apic_affine_ratio > 0.0f, "quality solver settings did not enable APIC affine transfer");
        REQUIRE(quality_settings.pressure_max_iterations > balanced_settings.pressure_max_iterations, "quality solver settings did not increase pressure iterations");
        REQUIRE(quality_settings.particles_per_full_cell >= balanced_settings.particles_per_full_cell, "quality solver settings did not keep enough particles per cell");

        physics_sim::WaterSimulation2D simulation{8, 8, 1.0f};
        REQUIRE(simulation.solver_settings().profile == physics_sim::FluidSolverProfile::Balanced, "simulation did not start with balanced solver defaults");

        auto custom_settings = live_settings;
        custom_settings.pressure_max_iterations = 1;
        custom_settings.pressure_relative_residual_target = 1.0e-8f;
        custom_settings.particles_per_full_cell = 6;
        custom_settings.apic_affine_ratio = 0.5f;
        simulation.set_solver_settings(custom_settings);

        REQUIRE(simulation.solver_settings().pressure_max_iterations == 1, "simulation did not store custom solver settings");
        REQUIRE(nearly_equal(simulation.solver_settings().pressure_relative_residual_target, 1.0e-8, 1.0e-10), "simulation did not preserve the custom pressure target");
        REQUIRE(simulation.solver_settings().particles_per_full_cell == 6, "simulation did not preserve particles-per-cell calibration");
        REQUIRE(nearly_equal(simulation.solver_settings().apic_affine_ratio, 0.5, 0.000001), "simulation did not preserve APIC blend");
    }

    {
        physics_sim::WaterSimulation2D simulation{18, 18, 1.0f};
        for (std::size_t x = 0; x < simulation.grid().width(); ++x)
        {
            simulation.set_solid_cell(x, 0, true);
            simulation.set_solid_cell(x, simulation.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < simulation.grid().height(); ++y)
        {
            simulation.set_solid_cell(0, y, true);
            simulation.set_solid_cell(simulation.grid().width() - 1, y, true);
        }

        simulation.particles().push_back({Vec2{8.0f, 8.0f}, Vec2{1.0f, 1.0f}});
        simulation.particles().push_back({Vec2{9.0f, 8.0f}, Vec2{-1.0f, 1.0f}});

        auto custom_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        custom_settings.pressure_max_iterations = 1;
        custom_settings.pressure_relative_residual_target = 1.0e-8f;
        simulation.set_solver_settings(custom_settings);

        simulation.step(0.1);

        REQUIRE(simulation.metrics().pressure_solve.max_iterations == 1, "pressure solve did not use the configured iteration cap");
        REQUIRE(nearly_equal(simulation.metrics().pressure_solve.target_relative_residual, 1.0e-8, 1.0e-10), "pressure solve did not report the configured pressure target");
    }

    {
        physics_sim::WaterSimulation2D control{24, 24, 1.0f};
        physics_sim::WaterSimulation2D viscous{24, 24, 1.0f};
        physics_sim::WaterSimulation2D cohesive{24, 24, 1.0f};

        for (auto* simulation : {&control, &viscous, &cohesive})
        {
            for (std::size_t x = 0; x < simulation->grid().width(); ++x)
            {
                simulation->set_solid_cell(x, 0, true);
                simulation->set_solid_cell(x, simulation->grid().height() - 1, true);
            }
            for (std::size_t y = 0; y < simulation->grid().height(); ++y)
            {
                simulation->set_solid_cell(0, y, true);
                simulation->set_solid_cell(simulation->grid().width() - 1, y, true);
            }
        }

        constexpr double pi = 3.14159265358979323846;
        for (std::size_t index = 0; index < 12; ++index)
        {
            const double angle = (2.0 * pi * static_cast<double>(index)) / 12.0;
            const float x = 12.0f + static_cast<float>(std::cos(angle) * 3.0);
            const float y = 12.0f + static_cast<float>(std::sin(angle) * 3.0);
            control.particles().push_back({Vec2{x, y}, Vec2{1.5f, 0.0f}});
            viscous.particles().push_back({Vec2{x, y}, Vec2{1.5f, 0.0f}});
            cohesive.particles().push_back({Vec2{x, y}, Vec2{1.5f, 0.0f}});
        }

        auto viscous_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        viscous_settings.viscosity_coefficient = 20.0f;
        viscous.set_solver_settings(viscous_settings);

        auto cohesive_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        cohesive_settings.surface_tension_coefficient = 20.0f;
        cohesive.set_solver_settings(cohesive_settings);

        for (int i = 0; i < 24; ++i)
        {
            control.step(1.0 / 120.0);
            viscous.step(1.0 / 120.0);
            cohesive.step(1.0 / 120.0);
        }

        REQUIRE(viscous.metrics().kinetic_energy < control.metrics().kinetic_energy, "viscosity did not reduce kinetic energy");

        REQUIRE(control.metrics().kinetic_energy > viscous.metrics().kinetic_energy, "viscosity test sanity check failed");

        physics_sim::WaterSimulation2D surface_control{24, 24, 1.0f};
        physics_sim::WaterSimulation2D surface_cohesive{24, 24, 1.0f};
        for (auto* simulation : {&surface_control, &surface_cohesive})
        {
            for (std::size_t x = 0; x < simulation->grid().width(); ++x)
            {
                simulation->set_solid_cell(x, 0, true);
                simulation->set_solid_cell(x, simulation->grid().height() - 1, true);
            }
            for (std::size_t y = 0; y < simulation->grid().height(); ++y)
            {
                simulation->set_solid_cell(0, y, true);
                simulation->set_solid_cell(simulation->grid().width() - 1, y, true);
            }
        }

        constexpr std::size_t particle_count = 16;
        constexpr float center_x = 12.0f;
        constexpr float center_y = 12.0f;
        constexpr float ring_radius = 3.5f;
        for (std::size_t index = 0; index < particle_count; ++index)
        {
            const double angle = (2.0 * pi * static_cast<double>(index)) / static_cast<double>(particle_count);
            const float x = center_x + static_cast<float>(std::cos(angle) * ring_radius);
            const float y = center_y + static_cast<float>(std::sin(angle) * ring_radius);
            surface_control.particles().push_back({Vec2{x, y}, Vec2{0.0f, 0.0f}});
            surface_cohesive.particles().push_back({Vec2{x, y}, Vec2{0.0f, 0.0f}});
        }

        auto surface_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        surface_settings.surface_tension_coefficient = 200.0f;
        surface_cohesive.set_solver_settings(surface_settings);

        for (int i = 0; i < 120; ++i)
        {
            surface_control.step(1.0 / 120.0);
            surface_cohesive.step(1.0 / 120.0);
        }

        const auto max_radius = [](const physics_sim::WaterSimulation2D& simulation) -> double
        {
            if (simulation.particles().empty())
            {
                return 0.0;
            }

            double max_distance = 0.0;
            Vec2 centroid{};
            for (const auto& particle : simulation.particles())
            {
                centroid = centroid + particle.position;
            }
            centroid = centroid / static_cast<float>(simulation.particles().size());

            for (const auto& particle : simulation.particles())
            {
                const double dx = static_cast<double>(particle.position.x - centroid.x);
                const double dy = static_cast<double>(particle.position.y - centroid.y);
                max_distance = std::max(max_distance, std::sqrt(dx * dx + dy * dy));
            }

            return max_distance;
        };

        REQUIRE(max_radius(surface_cohesive) < max_radius(surface_control), "surface tension did not reduce the droplet radius");
    }

    {
        physics_sim::FluidDensitySettings settings;
        settings.rest_density = 1.0f;
        settings.particle_volume = 1.0f;
        settings.kernel_radius = 1.0f;

        physics_sim::DensityCorrectionSettings correction_settings;
        correction_settings.iterations = 4;
        correction_settings.max_correction = 0.2f;

        std::vector<physics_sim::FluidParticle> crowded_particles{
            {Vec2{4.00f, 4.00f}, Vec2{}},
            {Vec2{4.02f, 4.00f}, Vec2{}},
            {Vec2{3.98f, 4.00f}, Vec2{}},
            {Vec2{4.00f, 4.02f}, Vec2{}},
            {Vec2{4.00f, 3.98f}, Vec2{}},
            {Vec2{4.01f, 4.01f}, Vec2{}},
            {Vec2{3.99f, 4.01f}, Vec2{}},
            {Vec2{4.01f, 3.99f}, Vec2{}},
        };

        const auto correction = physics_sim::apply_density_constraint_correction(crowded_particles, settings, correction_settings);
        REQUIRE(crowded_particles.size() == 8, "density correction changed particle count");
        REQUIRE(nearly_equal(correction.total_mass_before, correction.total_mass_after, 0.000001), "density correction changed total mass");
        REQUIRE(nearly_equal(correction.center_of_mass_before.x, correction.center_of_mass_after.x, 0.0005), "density correction shifted center of mass x");
        REQUIRE(nearly_equal(correction.center_of_mass_before.y, correction.center_of_mass_after.y, 0.0005), "density correction shifted center of mass y");
        REQUIRE(correction.max_density_error_after < correction.max_density_error_before, "density correction did not reduce max density error");
        REQUIRE(correction.max_applied_correction <= correction_settings.max_correction + 0.000001f, "density correction exceeded the configured cap");
    }

    {
        physics_sim::WaterSimulation2D control{48, 48, 1.0f};
        physics_sim::WaterSimulation2D corrected{48, 48, 1.0f};
        auto control_settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Balanced);
        auto corrected_settings = control_settings;
        control_settings.density_correction_iterations = 0;
        corrected_settings.density_correction_iterations = 1;
        control_settings.resampling.enabled = false;
        corrected_settings.resampling.enabled = false;
        control.set_solver_settings(control_settings);
        corrected.set_solver_settings(corrected_settings);

        const auto populate_cluster = [](physics_sim::WaterSimulation2D& simulation)
        {
            for (std::size_t y = 0; y < 33; ++y)
            {
                for (std::size_t x = 0; x < 33; ++x)
                {
                    const float offset_x = (static_cast<float>(x) - 16.0f) * 0.035f;
                    const float offset_y = (static_cast<float>(y) - 16.0f) * 0.035f;
                    simulation.add_particle({Vec2{24.0f + offset_x, 24.0f + offset_y}, Vec2{0.0f, 0.0f}});
                }
            }
        };

        populate_cluster(control);
        populate_cluster(corrected);

        control.step(1.0 / 120.0);
        corrected.step(1.0 / 120.0);
        control.refresh_density_metrics();
        corrected.refresh_density_metrics();

        REQUIRE(corrected.metrics().max_density_error < control.metrics().max_density_error, "solver density correction did not run above the old particle-count cutoff");
        REQUIRE(corrected.metrics().average_density_error <= control.metrics().average_density_error, "solver density correction did not improve average density error above the old particle-count cutoff");
    }

    {
        physics_sim::WaterSimulation2D control{16, 16, 1.0f};
        physics_sim::WaterSimulation2D corrected{16, 16, 1.0f};
        control.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());
        corrected.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());

        for (auto* simulation : {&control, &corrected})
        {
            for (std::size_t x = 0; x < simulation->grid().width(); ++x)
            {
                simulation->set_solid_cell(x, 0, true);
                simulation->set_solid_cell(x, simulation->grid().height() - 1, true);
            }
            for (std::size_t y = 0; y < simulation->grid().height(); ++y)
            {
                simulation->set_solid_cell(0, y, true);
                simulation->set_solid_cell(simulation->grid().width() - 1, y, true);
            }
        }

        for (std::size_t index = 0; index < 16; ++index)
        {
            const float x = 8.0f + static_cast<float>(index % 4) * 0.02f;
            const float y = 8.0f + static_cast<float>(index / 4) * 0.02f;
            control.particles().push_back({Vec2{x, y}, Vec2{0.0f, 0.0f}});
            corrected.particles().push_back({Vec2{x, y}, Vec2{0.0f, 0.0f}});
        }

        auto solver_settings = physics_sim::WaterSimulation2D::live_solver_settings();
        solver_settings.density_correction_iterations = 2;
        solver_settings.max_density_correction_fraction = 0.15f;
        corrected.set_solver_settings(solver_settings);

        for (int i = 0; i < 4; ++i)
        {
            control.step(1.0 / 120.0);
            corrected.step(1.0 / 120.0);
        }

        REQUIRE(corrected.metrics().max_density_error < control.metrics().max_density_error, "solver density correction did not reduce max density error");
        REQUIRE(corrected.metrics().average_density_error <= control.metrics().average_density_error, "solver density correction did not reduce average density error");
        REQUIRE(std::isfinite(corrected.metrics().max_density_error), "solver density correction produced a non-finite density error");
    }

    {
        physics_sim::MacGrid2D grid{4, 4, 1.0f};
        physics_sim::FluidDensitySettings settings;
        settings.rest_density = 2.0f;
        settings.particle_volume = 0.25f;
        settings.kernel_radius = 0.5f;

        std::vector<physics_sim::FluidParticle> particles{
            {Vec2{1.25f, 1.25f}, Vec2{}, 0.0f, 0.25f},
            {Vec2{1.75f, 1.25f}, Vec2{}, 0.0f, 0.25f},
            {Vec2{1.25f, 1.75f}, Vec2{}, 0.0f, 0.25f},
            {Vec2{1.75f, 1.75f}, Vec2{}, 0.0f, 0.25f},
            {Vec2{2.25f, 1.25f}, Vec2{}, 0.0f, 0.25f},
        };
        grid.set_solid(2, 1, true);

        const auto classification = physics_sim::classify_fluid_cells(grid, particles, settings);
        REQUIRE(classification.state(1, 1) == physics_sim::FluidCellState::Fluid, "full particle cell was not classified as fluid");
        REQUIRE(nearly_equal(classification.volume_fraction(1, 1), 1.0, 0.000001), "full particle cell did not clamp to full volume");
        REQUIRE(nearly_equal(classification.density(1, 1), settings.rest_density, 0.000001), "full particle cell density did not match rest density");
        REQUIRE(classification.state(2, 1) == physics_sim::FluidCellState::Solid, "solid cell did not override particle volume classification");
        REQUIRE(classification.state(1, 0) == physics_sim::FluidCellState::Air, "free-surface neighbor was not classified as air");
        REQUIRE(classification.fluid_cell_count == 1, "unexpected fluid-cell count");
        REQUIRE(classification.solid_cell_count == 1, "unexpected solid-cell count");
    }

    {
        physics_sim::WaterSimulation2D sim{4, 4, 1.0f};
        sim.set_solid_cell(2, 1, true);
        sim.add_particle({Vec2{1.5f, 1.5f}, Vec2{}, 0.0f, 0.5f});
        sim.refresh_cell_classification();

        REQUIRE(sim.cell_state(1, 1) == physics_sim::FluidCellState::Fluid, "simulation did not expose a fluid classified cell");
        REQUIRE(sim.cell_state(2, 1) == physics_sim::FluidCellState::Solid, "simulation did not expose a solid classified cell");
        REQUIRE(sim.cell_state(0, 0) == physics_sim::FluidCellState::Air, "simulation did not expose an air classified cell");
        REQUIRE(nearly_equal(sim.cell_volume_fraction(1, 1), 0.5, 0.000001), "simulation cell volume fraction was incorrect");
        REQUIRE(nearly_equal(sim.cell_density(1, 1), 1.0, 0.000001), "simulation cell density was incorrect");
    }

    {
        physics_sim::WaterSimulation2D sim{24, 24, 1.0f};
        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = Vec2{12.0f, 4.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 6.0f;
        emitter.emission_rate = 32.0f;
        sim.add_emitter(emitter);

        sim.step(0.5);

        REQUIRE(!sim.particles().empty(), "emitter did not spawn particles");
        REQUIRE(sim.particles().front().velocity.y > 0.0f, "emitted particles did not move downward");
        REQUIRE(sim.metrics().total_emitted > 0, "emitter count did not increment");
    }

    {
        physics_sim::WaterSimulation2D sim{80, 45, 16.0f};
        sim.add_particle({Vec2{640.0f, 160.0f}, Vec2{0.0f, 0.0f}});

        sim.step(1.0 / 120.0);

        REQUIRE(sim.metrics().active_cells > 0, "local particle did not mark any active cells");
        REQUIRE(sim.metrics().visible_fluid_cells > 0, "local particle did not report visible fluid cells");
        REQUIRE(sim.metrics().pressure_active_cells > 0, "local particle did not report pressure-active cells");
        REQUIRE(sim.metrics().pressure_active_cells <= 16, "local particle activated too much of the demo grid");
        REQUIRE(sim.metrics().active_cell_overreach_ratio <= 16.0, "local particle overreach ratio was not bounded");
    }

    {
        physics_sim::WaterSimulation2D control{8, 8, 1.0f};
        physics_sim::WaterSimulation2D resampled{8, 8, 1.0f};
        control.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());
        resampled.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());

        for (auto* simulation : {&control, &resampled})
        {
            for (std::size_t x = 0; x < simulation->grid().width(); ++x)
            {
                simulation->set_solid_cell(x, 0, true);
                simulation->set_solid_cell(x, simulation->grid().height() - 1, true);
            }
            for (std::size_t y = 0; y < simulation->grid().height(); ++y)
            {
                simulation->set_solid_cell(0, y, true);
                simulation->set_solid_cell(simulation->grid().width() - 1, y, true);
            }
        }

        for (std::size_t index = 0; index < 6; ++index)
        {
            const float x = 4.0f + static_cast<float>(index) * 0.02f;
            const float y = 4.0f + static_cast<float>(index % 2) * 0.02f;
            control.add_particle({Vec2{x, y}, Vec2{1.0f, 0.0f}, 1.0f, 1.0f});
            resampled.add_particle({Vec2{x, y}, Vec2{1.0f, 0.0f}, 1.0f, 1.0f});
        }

        auto settings = physics_sim::WaterSimulation2D::live_solver_settings();
        settings.resampling.enabled = true;
        settings.resampling.min_particles_per_fluid_cell = 2;
        settings.resampling.target_particles_per_fluid_cell = 4;
        settings.resampling.max_particles_per_fluid_cell = 4;
        settings.resampling.max_resampling_operations_per_step = 16;
        settings.resampling.split_offset_fraction = 0.20f;
        settings.resampling.min_split_particle_mass = 0.5f;
        resampled.set_solver_settings(settings);

        control.step(1.0 / 120.0);
        resampled.step(1.0 / 120.0);

        REQUIRE(resampled.particles().size() < control.particles().size(), "resampling did not reduce the crowded particle count");
        REQUIRE(nearly_equal(physics_sim::total_particle_mass(control.particles()), physics_sim::total_particle_mass(resampled.particles()), 0.000001), "resampling did not conserve total mass");
        REQUIRE(nearly_equal(sum_momentum(control.particles()).x, sum_momentum(resampled.particles()).x, 0.000001), "resampling did not conserve x momentum");
        REQUIRE(nearly_equal(sum_momentum(control.particles()).y, sum_momentum(resampled.particles()).y, 0.000001), "resampling did not conserve y momentum");
    }

    {
        physics_sim::WaterSimulation2D control{8, 8, 1.0f};
        physics_sim::WaterSimulation2D resampled{8, 8, 1.0f};
        control.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());
        resampled.set_solver_settings(physics_sim::WaterSimulation2D::live_solver_settings());

        for (auto* simulation : {&control, &resampled})
        {
            for (std::size_t x = 0; x < simulation->grid().width(); ++x)
            {
                simulation->set_solid_cell(x, 0, true);
                simulation->set_solid_cell(x, simulation->grid().height() - 1, true);
            }
            for (std::size_t y = 0; y < simulation->grid().height(); ++y)
            {
                simulation->set_solid_cell(0, y, true);
                simulation->set_solid_cell(simulation->grid().width() - 1, y, true);
            }
        }

        control.add_particle({Vec2{4.5f, 4.5f}, Vec2{0.5f, -0.25f}, 1.0f, 1.0f});
        resampled.add_particle({Vec2{4.5f, 4.5f}, Vec2{0.5f, -0.25f}, 1.0f, 1.0f});

        auto settings = physics_sim::WaterSimulation2D::live_solver_settings();
        settings.resampling.enabled = true;
        settings.resampling.min_particles_per_fluid_cell = 2;
        settings.resampling.target_particles_per_fluid_cell = 4;
        settings.resampling.max_particles_per_fluid_cell = 4;
        settings.resampling.max_resampling_operations_per_step = 16;
        settings.resampling.split_offset_fraction = 0.20f;
        settings.resampling.min_split_particle_mass = 0.25f;
        resampled.set_solver_settings(settings);

        control.step(1.0 / 120.0);
        resampled.step(1.0 / 120.0);

        REQUIRE(resampled.particles().size() > control.particles().size(), "resampling did not split the sparse particle");
        REQUIRE(nearly_equal(physics_sim::total_particle_mass(control.particles()), physics_sim::total_particle_mass(resampled.particles()), 0.000001), "resampling split did not conserve total mass");
        REQUIRE(nearly_equal(sum_momentum(control.particles()).x, sum_momentum(resampled.particles()).x, 0.000001), "resampling split did not conserve x momentum");
        REQUIRE(nearly_equal(sum_momentum(control.particles()).y, sum_momentum(resampled.particles()).y, 0.000001), "resampling split did not conserve y momentum");
    }

    {
        physics_sim::MacGrid2D grid{8, 8, 1.0f};
        physics_sim::FluidParticle particle{Vec2{4.5f, 4.5f}, Vec2{3.0f, 4.0f}};
        std::vector<float> u_weights(grid.u_count(), 0.0f);
        std::vector<float> v_weights(grid.v_count(), 0.0f);

        physics_sim::scatter_particle_velocity(grid, particle, u_weights, v_weights);
        physics_sim::normalize_mac_grid_faces(grid, u_weights, v_weights);

        REQUIRE(nearly_equal(grid.u(4, 4), 3.0f, 0.0002f), "scatter did not normalize the left u face");
        REQUIRE(nearly_equal(grid.u(5, 4), 3.0f, 0.0002f), "scatter did not normalize the right u face");
        REQUIRE(nearly_equal(grid.v(4, 4), 4.0f, 0.0002f), "scatter did not normalize the top v face");
        REQUIRE(nearly_equal(grid.v(4, 5), 4.0f, 0.0002f), "scatter did not normalize the bottom v face");
        REQUIRE(nearly_equal(grid.u(3, 4), 0.0f, 0.0002f), "scatter contaminated an unrelated u face");
        REQUIRE(nearly_equal(grid.v(5, 4), 0.0f, 0.0002f), "scatter contaminated an unrelated v face");

        grid.u(4, 4) = 2.0f;
        grid.u(5, 4) = 6.0f;
        grid.u(4, 5) = 10.0f;
        grid.u(5, 5) = 14.0f;
        grid.v(4, 4) = 1.0f;
        grid.v(5, 4) = 3.0f;
        grid.v(4, 5) = 5.0f;
        grid.v(5, 5) = 7.0f;

        const physics_sim::Vec2 sampled = physics_sim::sample_mac_grid_velocity(grid, Vec2{4.75f, 4.75f});
        REQUIRE(nearly_equal(sampled.x, 7.0f, 0.0002f), "grid-to-particle x sampling was incorrect");
        REQUIRE(nearly_equal(sampled.y, 4.5f, 0.0002f), "grid-to-particle y sampling was incorrect");
    }

    {
        physics_sim::FluidParticle rotating_particle{Vec2{2.0f, 2.0f}, Vec2{0.0f, 0.0f}};
        rotating_particle.affine_velocity = physics_sim::Mat2{0.0f, -2.0f, 2.0f, 0.0f};

        const physics_sim::Vec2 top_velocity = physics_sim::apic_velocity_at(rotating_particle, Vec2{2.0f, 2.5f});
        const physics_sim::Vec2 right_velocity = physics_sim::apic_velocity_at(rotating_particle, Vec2{2.5f, 2.0f});
        REQUIRE(nearly_equal(top_velocity.x, -1.0f, 0.000001), "APIC affine velocity did not preserve rotational x flow");
        REQUIRE(nearly_equal(top_velocity.y, 0.0f, 0.000001), "APIC affine velocity added unexpected rotational y flow");
        REQUIRE(nearly_equal(right_velocity.x, 0.0f, 0.000001), "APIC affine velocity added unexpected rotational x flow");
        REQUIRE(nearly_equal(right_velocity.y, 1.0f, 0.000001), "APIC affine velocity did not preserve rotational y flow");
    }

    {
        physics_sim::MacGrid2D grid{6, 6, 1.0f};
        std::vector<physics_sim::FluidParticle> particles{
            {Vec2{2.5f, 2.5f}, Vec2{2.0f, 3.0f}, 2.0f, 2.0f},
            {Vec2{3.5f, 2.5f}, Vec2{4.0f, -1.0f}, 3.0f, 3.0f},
        };
        std::vector<float> u_masses(grid.u_count(), 0.0f);
        std::vector<float> v_masses(grid.v_count(), 0.0f);

        const auto summary = physics_sim::scatter_particles_apic_to_grid(grid, particles, u_masses, v_masses);
        physics_sim::normalize_mac_grid_faces_by_mass(grid, u_masses, v_masses);
        const physics_sim::Vec2 grid_momentum = physics_sim::grid_momentum_from_faces(grid, u_masses, v_masses);

        REQUIRE(nearly_equal(summary.total_particle_mass, 5.0, 0.000001), "APIC scatter did not report total particle mass");
        REQUIRE(nearly_equal(summary.particle_momentum.x, 16.0, 0.000001), "APIC scatter reported incorrect x particle momentum");
        REQUIRE(nearly_equal(summary.particle_momentum.y, 3.0, 0.000001), "APIC scatter reported incorrect y particle momentum");
        REQUIRE(nearly_equal(grid_momentum.x, summary.particle_momentum.x, 0.0005), "mass-weighted scatter did not conserve x momentum");
        REQUIRE(nearly_equal(grid_momentum.y, summary.particle_momentum.y, 0.0005), "mass-weighted scatter did not conserve y momentum");
    }

    {
        physics_sim::WaterSimulation2D sim{12, 12, 1.0f};
        auto settings = physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Quality);
        settings.apic_affine_ratio = 1.0f;
        settings.density_correction_iterations = 0;
        settings.resampling.enabled = false;
        sim.set_solver_settings(settings);
        sim.particles().push_back({Vec2{5.25f, 5.25f}, Vec2{0.0f, -2.0f}});
        sim.particles().push_back({Vec2{5.75f, 5.25f}, Vec2{0.0f, 2.0f}});
        sim.particles().push_back({Vec2{5.25f, 5.75f}, Vec2{2.0f, 0.0f}});
        sim.particles().push_back({Vec2{5.75f, 5.75f}, Vec2{-2.0f, 0.0f}});

        sim.step(1.0 / 120.0);

        double max_affine_component = 0.0;
        for (const auto& particle : sim.particles())
        {
            max_affine_component = std::max(max_affine_component, std::abs(static_cast<double>(particle.affine_velocity.m00)));
            max_affine_component = std::max(max_affine_component, std::abs(static_cast<double>(particle.affine_velocity.m01)));
            max_affine_component = std::max(max_affine_component, std::abs(static_cast<double>(particle.affine_velocity.m10)));
            max_affine_component = std::max(max_affine_component, std::abs(static_cast<double>(particle.affine_velocity.m11)));
        }

        REQUIRE(max_affine_component > 0.001, "runtime APIC transfer did not update affine state");
        REQUIRE(max_affine_component < 20.0, "runtime APIC transfer produced unbounded affine state");
    }

    {
        const physics_sim::Vec2 blended = physics_sim::pic_flip_blend(
            Vec2{3.0f, 5.0f},
            Vec2{10.0f, 20.0f},
            Vec2{2.0f, 4.0f},
            Vec2{6.0f, 12.0f},
            0.25f);
        REQUIRE(nearly_equal(blended.x, 9.25, 0.000001), "PIC/FLIP blend produced incorrect x velocity");
        REQUIRE(nearly_equal(blended.y, 18.25, 0.000001), "PIC/FLIP blend produced incorrect y velocity");
    }

    {
        physics_sim::WaterSimulation2D sim{16, 16, 1.0f};
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, 0, true);
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        sim.particles().push_back({Vec2{8.0f, 8.0f}, Vec2{0.0f, -3.0f}});
        sim.step(0.25);

        for (const auto& particle : sim.particles())
        {
            const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x));
            const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y));
            REQUIRE(!sim.grid().solid(cell_x, cell_y), "particle penetrated a solid cell");
        }
    }

    {
        physics_sim::WaterSimulation2D sim{8, 8, 1.0f};
        sim.set_solid_cell(5, 4, true);
        sim.particles().push_back({Vec2{4.75f, 4.25f}, Vec2{6.0f, 2.0f}});

        sim.step(0.1);

        REQUIRE(sim.particles().size() == 1, "wall-slide particle was unexpectedly removed");
        const auto& particle = sim.particles().front();
        const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x));
        const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y));

        REQUIRE(!sim.grid().solid(cell_x, cell_y), "wall-slide particle penetrated a solid cell");
        REQUIRE(particle.position.x < 5.0f, "wall-slide particle crossed the blocking wall");
        REQUIRE(particle.position.y > 4.30f, "wall-slide particle did not preserve tangential motion");
        REQUIRE(std::fabs(particle.velocity.x) < 0.0005f, "wall-slide particle kept velocity into the wall");
        REQUIRE(particle.velocity.y > 0.1f, "wall-slide particle lost tangential velocity");
    }

    {
        physics_sim::WaterSimulation2D sim{8, 8, 1.0f};
        auto settings = sim.solver_settings();
        settings.wall_material = physics_sim::WallBoundaryMaterial::Damped;
        settings.wall_tangential_velocity_retention = 0.25f;
        sim.set_solver_settings(settings);
        sim.set_solid_cell(5, 4, true);
        sim.particles().push_back({Vec2{4.75f, 4.25f}, Vec2{6.0f, 2.0f}});

        sim.step(0.1);

        REQUIRE(sim.particles().size() == 1, "damped wall-slide particle was unexpectedly removed");
        const auto& particle = sim.particles().front();
        const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x));
        const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y));

        REQUIRE(!sim.grid().solid(cell_x, cell_y), "damped wall-slide particle penetrated a solid cell");
        REQUIRE(particle.position.x < 5.0f, "damped wall-slide particle crossed the blocking wall");
        REQUIRE(std::fabs(particle.velocity.x) < 0.0005f, "damped wall-slide particle kept velocity into the wall");
        REQUIRE(particle.velocity.y > 0.0f, "damped wall-slide particle reversed tangential velocity");
        REQUIRE(particle.velocity.y < 1.5f, "damped wall-slide particle did not reduce tangential velocity");
    }

    {
        physics_sim::WaterSimulation2D sim{8, 8, 1.0f};
        sim.particles().push_back({Vec2{-0.25f, 4.0f}, Vec2{-2.0f, 0.0f}});
        sim.step(0.1);

        REQUIRE(sim.particles().empty(), "out-of-domain particle was not culled");
        REQUIRE(sim.metrics().active_particles == 0, "active particle count did not clear");
        REQUIRE(sim.metrics().total_removed == 1, "removed particle count did not increment");
        REQUIRE(sim.metrics().total_outflow == 1, "outflow particle count did not increment");
    }

    {
        physics_sim::WaterSimulation2D sim{18, 18, 1.0f};
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, 0, true);
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        sim.particles().push_back({Vec2{9.0f, 9.0f}, Vec2{1.0f, 1.0f}});
        sim.particles().push_back({Vec2{10.0f, 9.0f}, Vec2{-1.0f, 1.0f}});
        sim.step(0.1);

        REQUIRE(sim.metrics().average_divergence_after_projection < 0.15, "average divergence too high");
        REQUIRE(sim.metrics().max_divergence_after_projection < 0.5, "max divergence too high");
        REQUIRE(sim.metrics().pressure_solve.active_cells > 0, "pressure solve did not report active cells");
        REQUIRE(sim.metrics().pressure_solve.visible_cells == sim.metrics().visible_fluid_cells, "pressure solve visible-cell count did not match metrics");
        REQUIRE(sim.metrics().pressure_solve.pressure_active_cells == sim.metrics().pressure_active_cells, "pressure solve active-cell count did not match metrics");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.active_cell_overreach_ratio), "pressure solve overreach ratio was non-finite");
        REQUIRE(sim.metrics().pressure_solve.iterations > 0, "pressure solve did not report iterations");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.initial_residual), "pressure solve initial residual was non-finite");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.final_residual), "pressure solve final residual was non-finite");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.absolute_residual), "pressure solve absolute residual was non-finite");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.relative_residual), "pressure solve relative residual was non-finite");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.rhs_l2), "pressure solve RHS norm was non-finite");
        REQUIRE(std::isfinite(sim.metrics().pressure_solve.solution_l2), "pressure solve solution norm was non-finite");
        REQUIRE(nearly_equal(sim.metrics().pressure_solve.pressure_dt, 0.1f, 0.000001f), "pressure solve did not report dt");
        REQUIRE(nearly_equal(sim.metrics().pressure_solve.rest_density, sim.solver_settings().rest_density, 0.000001f), "pressure solve did not report rest density");
        REQUIRE(sim.metrics().pressure_solve.final_residual <= sim.metrics().pressure_solve.initial_residual + 0.000001, "pressure solve did not reduce residual");
        REQUIRE(sim.metrics().pressure_solve.absolute_residual <= sim.metrics().pressure_solve.initial_residual + 0.000001, "pressure solve absolute residual did not match final residual");
        REQUIRE(sim.metrics().pressure_solve.relative_residual <= 1.0, "pressure solve relative residual did not improve");
    }

    {
        physics_sim::WaterSimulation2D sim{16, 16, 1.0f};
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, 0, true);
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = Vec2{8.0f, 8.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 5.0f;
        emitter.emission_rate = 12.0f;
        sim.add_emitter(emitter);

        constexpr int steps = 240;
        for (int i = 0; i < steps; ++i)
        {
            sim.step(1.0 / 120.0);
        }

        REQUIRE(sim.metrics().total_emitted > 0, "closed-box fill test never emitted fluid");
        const double expected_mass = sim.metrics().total_emitted_mass - sim.metrics().total_removed_mass - sim.metrics().total_outflow_mass;
        REQUIRE(nearly_equal(physics_sim::total_particle_mass(sim.particles()), expected_mass, 0.000001), "closed-box mass did not match emitted mass");
        for (const auto& particle : sim.particles())
        {
            const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x));
            const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y));
            REQUIRE(cell_x > 0 && cell_x < sim.grid().width() - 1, "closed-box particle leaked through a wall");
            REQUIRE(cell_y > 0 && cell_y < sim.grid().height() - 1, "closed-box particle leaked through a wall");
            REQUIRE(!sim.grid().solid(cell_x, cell_y), "closed-box particle ended inside a solid cell");
        }
    }

    {
        physics_sim::WaterSimulation2D sim{20, 20, 1.0f};
        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Omni;
        emitter.position = Vec2{10.0f, 10.0f};
        emitter.speed = 5.0f;
        emitter.emission_rate = 48.0f;
        sim.add_emitter(emitter);

        sim.step(0.5);

        REQUIRE(sim.particles().size() >= 8, "omni emitter did not spawn enough particles");

        bool has_left = false;
        bool has_right = false;
        for (const auto& particle : sim.particles())
        {
            has_left = has_left || particle.velocity.x < -0.25f;
            has_right = has_right || particle.velocity.x > 0.25f;
        }

        REQUIRE(has_left, "omni emitter never emitted a leftward particle");
        REQUIRE(has_right, "omni emitter never emitted a rightward particle");
    }

    {
        physics_sim::WaterSimulation2D sim{80, 45, 16.0f};
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
        emitter.position = Vec2{640.0f, 160.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 8.0f;
        emitter.emission_rate = 42.0f;
        sim.add_emitter(emitter);

        constexpr int steps = 240;
        for (int i = 0; i < steps; ++i)
        {
            sim.step(1.0 / 120.0);
        }

        REQUIRE(sim.metrics().total_emitted > 0, "U-container fill test never emitted fluid");
        double current_mass = 0.0;
        for (const auto& particle : sim.particles())
        {
            current_mass += static_cast<double>(particle.mass > 0.0f ? particle.mass : 0.0f);
        }
        const double emitted_mass = std::max(sim.metrics().total_emitted_mass, 1.0e-9);
        const double mass_error = std::abs(
            current_mass
            - sim.metrics().total_emitted_mass
            + sim.metrics().total_removed_mass
            + sim.metrics().total_outflow_mass) / emitted_mass;
        REQUIRE(mass_error < 1.0e-6, "U-container mass accounting drifted under the balanced profile");
        for (const auto& particle : sim.particles())
        {
            const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x / sim.grid().cell_size()));
            const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y / sim.grid().cell_size()));
            REQUIRE(cell_x > 20 && cell_x < 60, "U-container particle leaked through a side wall");
            REQUIRE(cell_y < 36, "U-container particle leaked through the floor");
            REQUIRE(!sim.grid().solid(cell_x, cell_y), "U-container particle ended inside a solid cell");
        }
    }

    {
        physics_sim::WaterSimulation2D sim{80, 45, 16.0f};
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
        emitter.position = Vec2{640.0f, 160.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 8.0f;
        emitter.emission_rate = 120.0f;
        sim.add_emitter(emitter);

        constexpr int steps = 240;
        for (int i = 0; i < steps; ++i)
        {
            sim.step(1.0 / 120.0);
        }

        float max_y = emitter.position.y;
        for (const auto& particle : sim.particles())
        {
            if (particle.position.y > max_y)
            {
                max_y = particle.position.y;
            }
        }

        REQUIRE(sim.metrics().total_emitted >= 200, "demo-scale emitter did not emit enough particles");
        REQUIRE(max_y > emitter.position.y + sim.grid().cell_size() * 4.0f, "demo-scale emitter did not visibly move water downstream");
    }

    {
        physics_sim::WaterSimulation2D sim{12, 12, 1.0f};
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
        emitter.position = Vec2{6.0f, 2.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 6.0f;
        emitter.emission_rate = 12.0f;
        sim.add_emitter(emitter);

        constexpr int steps = 6000;
        for (int i = 0; i < steps; ++i)
        {
            sim.step(1.0 / 120.0);

            REQUIRE(finite_values(sim.grid().pressure_values()), "pressure became non-finite during stress run");
            REQUIRE(finite_values(sim.grid().divergence_values()), "divergence became non-finite during stress run");
            REQUIRE(finite_values(sim.grid().u_values()), "u velocity became non-finite during stress run");
            REQUIRE(finite_values(sim.grid().v_values()), "v velocity became non-finite during stress run");

            for (const auto& particle : sim.particles())
            {
                REQUIRE(finite_vec2(particle.position), "particle position became non-finite during stress run");
                REQUIRE(finite_vec2(particle.velocity), "particle velocity became non-finite during stress run");
            }

            REQUIRE(std::isfinite(sim.metrics().average_divergence_after_projection), "average divergence became non-finite during stress run");
            REQUIRE(std::isfinite(sim.metrics().max_divergence_after_projection), "max divergence became non-finite during stress run");
        }

        REQUIRE(sim.metrics().total_emitted > 0, "stress run never emitted fluid");
        REQUIRE(!sim.particles().empty(), "stress run lost all particles");
    }

    {
        physics_sim::WaterSimulation2D sim{10, 10, 1.0f};
        sim.set_solid_cell(3, 3, true);
        sim.particles().push_back({Vec2{4.0f, 4.0f}, Vec2{2.0f, 2.0f}});
        sim.clear_scene();

        REQUIRE(sim.particles().empty(), "clear_scene did not remove particles");
        REQUIRE(sim.grid().valid(), "grid became invalid after clear_scene");
        REQUIRE(!sim.grid().solid(3, 3), "clear_scene did not clear solids");
        REQUIRE(nearly_equal(sim.metrics().average_divergence_after_projection, 0.0), "metrics were not reset");
    }

    return 0;
}
