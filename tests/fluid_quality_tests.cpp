#include <physics_sim/fluid_quality.hpp>
#include <physics_sim/math.hpp>
#include <physics_sim/solver_profile.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using physics_sim::Vec2;
physics_sim::FluidSolverProfile selected_profile = physics_sim::FluidSolverProfile::Balanced;

[[noreturn]] void fail(const std::string& message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message.c_str());
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

bool nearly_equal(double lhs, double rhs, double epsilon = 1.0e-6) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}

bool nearly_equal(const Vec2& lhs, const Vec2& rhs, double epsilon = 1.0e-6) noexcept
{
    return nearly_equal(lhs.x, rhs.x, epsilon) && nearly_equal(lhs.y, rhs.y, epsilon);
}

std::string snapshot_label(const physics_sim::FluidQualitySnapshot& snapshot)
{
    std::ostringstream stream;
    stream << snapshot.scenario << "@tick=" << snapshot.tick;
    return stream.str();
}

std::string format_value(double value)
{
    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream << std::setprecision(6) << value;
    return stream.str();
}

void require_in_range(
    const physics_sim::FluidQualitySnapshot& snapshot,
    const char* metric,
    double actual,
    double minimum,
    double maximum)
{
    std::ostringstream stream;
    stream << snapshot_label(snapshot)
           << " metric=" << metric
           << " expected=[" << format_value(minimum) << ", " << format_value(maximum) << "]"
           << " actual=" << format_value(actual);
    REQUIRE(actual >= minimum && actual <= maximum, stream.str());
}

void require_less_than(
    const physics_sim::FluidQualitySnapshot& snapshot,
    const char* metric,
    double actual,
    double maximum)
{
    std::ostringstream stream;
    stream << snapshot_label(snapshot)
           << " metric=" << metric
           << " expected<= " << format_value(maximum)
           << " actual=" << format_value(actual);
    REQUIRE(actual <= maximum, stream.str());
}

void require_greater_than(
    const physics_sim::FluidQualitySnapshot& snapshot,
    const char* metric,
    double actual,
    double minimum)
{
    std::ostringstream stream;
    stream << snapshot_label(snapshot)
           << " metric=" << metric
           << " expected> " << format_value(minimum)
           << " actual=" << format_value(actual);
    REQUIRE(actual > minimum, stream.str());
}

void require_greater_than_or_equal(
    const physics_sim::FluidQualitySnapshot& snapshot,
    const char* metric,
    double actual,
    double minimum)
{
    std::ostringstream stream;
    stream << snapshot_label(snapshot)
           << " metric=" << metric
           << " expected>= " << format_value(minimum)
           << " actual=" << format_value(actual);
    REQUIRE(actual >= minimum, stream.str());
}

void require_count(
    const physics_sim::FluidQualitySnapshot& snapshot,
    const char* metric,
    std::size_t actual,
    std::size_t expected)
{
    std::ostringstream stream;
    stream << snapshot_label(snapshot)
           << " metric=" << metric
           << " expected=" << expected
           << " actual=" << actual;
    REQUIRE(actual == expected, stream.str());
}

void require_snapshot_close(const physics_sim::FluidQualitySnapshot& lhs, const physics_sim::FluidQualitySnapshot& rhs)
{
    REQUIRE(lhs.scenario == rhs.scenario, "snapshot scenarios differed between repeated runs");
    REQUIRE(lhs.tick == rhs.tick, "snapshot ticks differed between repeated runs");
    REQUIRE(lhs.particle_count == rhs.particle_count, "particle count differed between repeated runs");
    REQUIRE(lhs.metrics.total_emitted == rhs.metrics.total_emitted, "total emitted differed between repeated runs");
    REQUIRE(lhs.metrics.total_removed == rhs.metrics.total_removed, "total removed differed between repeated runs");
    REQUIRE(lhs.metrics.total_outflow == rhs.metrics.total_outflow, "total outflow differed between repeated runs");
    REQUIRE(lhs.metrics.active_particles == rhs.metrics.active_particles, "active particle count differed between repeated runs");
    REQUIRE(lhs.metrics.active_cells == rhs.metrics.active_cells, "active cell count differed between repeated runs");
    REQUIRE(lhs.metrics.active_sensors == rhs.metrics.active_sensors, "active sensor count differed between repeated runs");
    REQUIRE(lhs.metrics.objective_sensors == rhs.metrics.objective_sensors, "objective sensor count differed between repeated runs");
    REQUIRE(lhs.metrics.objective_completed == rhs.metrics.objective_completed, "objective completion differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.average_divergence_after_projection, rhs.metrics.average_divergence_after_projection), "average divergence differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.max_divergence_after_projection, rhs.metrics.max_divergence_after_projection), "max divergence differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.min_density, rhs.metrics.min_density), "minimum density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.max_density, rhs.metrics.max_density), "maximum density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.average_density, rhs.metrics.average_density), "average density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.max_density_error, rhs.metrics.max_density_error), "density error differed between repeated runs");
    REQUIRE(nearly_equal(lhs.metrics.average_neighbor_count, rhs.metrics.average_neighbor_count), "average neighbor count differed between repeated runs");
    REQUIRE(lhs.metrics.max_neighbor_count == rhs.metrics.max_neighbor_count, "max neighbor count differed between repeated runs");
    REQUIRE(nearly_equal(lhs.total_particle_mass, rhs.total_particle_mass), "particle mass differed between repeated runs");
    REQUIRE(nearly_equal(lhs.total_particle_volume, rhs.total_particle_volume), "particle volume differed between repeated runs");
    REQUIRE(nearly_equal(lhs.mass_error, rhs.mass_error), "mass error differed between repeated runs");
    REQUIRE(nearly_equal(lhs.momentum_x, rhs.momentum_x), "momentum x differed between repeated runs");
    REQUIRE(nearly_equal(lhs.momentum_y, rhs.momentum_y), "momentum y differed between repeated runs");
    REQUIRE(nearly_equal(lhs.divergence_l2, rhs.divergence_l2), "divergence l2 differed between repeated runs");
    REQUIRE(nearly_equal(lhs.pressure_l2, rhs.pressure_l2), "pressure l2 differed between repeated runs");
    REQUIRE(nearly_equal(lhs.hydrostatic_pressure_shape_error, rhs.hydrostatic_pressure_shape_error), "hydrostatic pressure shape error differed between repeated runs");
    REQUIRE(nearly_equal(lhs.surface_height, rhs.surface_height), "surface height differed between repeated runs");
    REQUIRE(nearly_equal(lhs.surface_height_jitter, rhs.surface_height_jitter), "surface height jitter differed between repeated runs");
    REQUIRE(nearly_equal(lhs.max_volume_fraction, rhs.max_volume_fraction), "max volume fraction differed between repeated runs");
    REQUIRE(nearly_equal(lhs.average_volume_fraction, rhs.average_volume_fraction), "average volume fraction differed between repeated runs");
    REQUIRE(nearly_equal(lhs.average_speed, rhs.average_speed), "average speed differed between repeated runs");
    REQUIRE(nearly_equal(lhs.max_speed, rhs.max_speed), "max speed differed between repeated runs");
    REQUIRE(nearly_equal(lhs.kinetic_energy, rhs.kinetic_energy), "kinetic energy differed between repeated runs");
    REQUIRE(nearly_equal(lhs.center_of_mass, rhs.center_of_mass), "center of mass differed between repeated runs");
    REQUIRE(nearly_equal(lhs.min_position, rhs.min_position), "minimum position differed between repeated runs");
    REQUIRE(nearly_equal(lhs.max_position, rhs.max_position), "maximum position differed between repeated runs");
    REQUIRE(lhs.particles_in_solids == rhs.particles_in_solids, "solid-penetration count differed between repeated runs");
    REQUIRE(lhs.particles_out_of_domain == rhs.particles_out_of_domain, "out-of-domain count differed between repeated runs");
    REQUIRE(nearly_equal(lhs.pooled_height, rhs.pooled_height), "pooled height differed between repeated runs");
    REQUIRE(nearly_equal(lhs.min_density, rhs.min_density), "snapshot minimum density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.max_density, rhs.max_density), "snapshot maximum density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.average_density, rhs.average_density), "snapshot average density differed between repeated runs");
    REQUIRE(nearly_equal(lhs.max_density_error, rhs.max_density_error), "snapshot density error differed between repeated runs");
    REQUIRE(nearly_equal(lhs.average_neighbor_count, rhs.average_neighbor_count), "snapshot average neighbor count differed between repeated runs");
    REQUIRE(lhs.max_neighbor_count == rhs.max_neighbor_count, "snapshot max neighbor count differed between repeated runs");
}

std::size_t count_particles_in_region(
    const physics_sim::WaterSimulation2D& simulation,
    std::size_t left,
    std::size_t top,
    std::size_t width,
    std::size_t height)
{
    const float cell_size = simulation.grid().cell_size();
    const float region_left = static_cast<float>(left) * cell_size;
    const float region_top = static_cast<float>(top) * cell_size;
    const float region_right = region_left + static_cast<float>(width) * cell_size;
    const float region_bottom = region_top + static_cast<float>(height) * cell_size;

    std::size_t count = 0;
    for (const auto& particle : simulation.particles())
    {
        if (particle.position.x >= region_left
            && particle.position.x < region_right
            && particle.position.y >= region_top
            && particle.position.y < region_bottom)
        {
            ++count;
        }
    }

    return count;
}

void add_border_walls(physics_sim::WaterSimulation2D& simulation)
{
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
}

void add_full_cell_particle(physics_sim::WaterSimulation2D& simulation, Vec2 position, Vec2 velocity = {})
{
    const float cell_area = simulation.grid().cell_size() * simulation.grid().cell_size();
    const std::size_t particle_count = std::max<std::size_t>(1, simulation.solver_settings().particles_per_full_cell);
    const std::size_t columns = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(particle_count))));
    const std::size_t rows = (particle_count + columns - 1) / columns;
    const float particle_volume = cell_area / static_cast<float>(particle_count);
    const float particle_mass = simulation.solver_settings().rest_density * particle_volume;
    const float spacing_x = simulation.grid().cell_size() * 0.60f / static_cast<float>(columns);
    const float spacing_y = simulation.grid().cell_size() * 0.60f / static_cast<float>(rows);

    for (std::size_t index = 0; index < particle_count; ++index)
    {
        const std::size_t column = index % columns;
        const std::size_t row = index / columns;
        const float offset_x = (static_cast<float>(column) + 0.5f - static_cast<float>(columns) * 0.5f) * spacing_x;
        const float offset_y = (static_cast<float>(row) + 0.5f - static_cast<float>(rows) * 0.5f) * spacing_y;
        simulation.add_particle({position + Vec2{offset_x, offset_y}, velocity, particle_mass, particle_volume});
    }
}

struct ScenarioCase
{
    std::string name;
    std::function<physics_sim::WaterSimulation2D()> make_simulation;
    std::vector<std::uint64_t> sample_ticks;
    std::optional<float> pool_floor_y;
    std::optional<std::size_t> pool_width_cells;
    physics_sim::FluidQualitySetupCallback setup;
    physics_sim::FluidQualityTickCallback tick_callback;
    std::function<void(const physics_sim::FluidQualityScenarioResult&, const physics_sim::FluidQualityScenarioResult&)> verify;
};

physics_sim::WaterEmitter directional_emitter(Vec2 position, Vec2 direction, float speed, float rate)
{
    physics_sim::WaterEmitter emitter;
    emitter.kind = physics_sim::WaterEmitterKind::Directional;
    emitter.position = position;
    emitter.direction = direction;
    emitter.speed = speed;
    emitter.emission_rate = rate;
    return emitter;
}

physics_sim::WaterEmitter omni_emitter(Vec2 position, float speed, float rate)
{
    physics_sim::WaterEmitter emitter;
    emitter.kind = physics_sim::WaterEmitterKind::Omni;
    emitter.position = position;
    emitter.direction = Vec2{0.0f, 1.0f};
    emitter.speed = speed;
    emitter.emission_rate = rate;
    return emitter;
}

physics_sim::FluidQualityScenarioResult run_case(const ScenarioCase& scenario, physics_sim::FluidSolverProfile profile)
{
    return physics_sim::run_fluid_quality_scenario(
        scenario.name,
        scenario.make_simulation(),
        scenario.sample_ticks,
        scenario.setup,
        scenario.tick_callback,
        scenario.pool_floor_y,
        scenario.pool_width_cells,
        profile);
}

void verify_repeatable(
    const ScenarioCase& scenario,
    const physics_sim::FluidQualityScenarioResult& first,
    const physics_sim::FluidQualityScenarioResult& second)
{
    REQUIRE(first.snapshots.size() == second.snapshots.size(), "snapshot count changed between repeated runs");
    for (std::size_t i = 0; i < first.snapshots.size(); ++i)
    {
        require_snapshot_close(first.snapshots[i], second.snapshots[i]);
    }

    scenario.verify(first, second);
}

ScenarioCase make_still_pool_case()
{
    return ScenarioCase{
        "still-pool",
        []()
        {
            return physics_sim::WaterSimulation2D{16, 24, 1.0f};
        },
        {0, 400, 800, 1200},
        20.0f,
        4,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t x = 5; x <= 10; ++x)
            {
                simulation.set_solid_cell(x, 20, true);
            }

            for (std::size_t y = 4; y <= 20; ++y)
            {
                simulation.set_solid_cell(5, y, true);
                simulation.set_solid_cell(10, y, true);
            }

            for (std::size_t y = 5; y <= 19; ++y)
            {
                for (std::size_t x = 6; x <= 9; ++x)
                {
                    add_full_cell_particle(simulation, Vec2{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f});
                }
            }
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& settled = first.snapshots.back();
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(settled, "average_speed", settled.average_speed, 1.5);
                require_less_than(settled, "max_speed", settled.max_speed, 4.5);
                require_less_than(settled, "kinetic_energy", settled.kinetic_energy, 80.0);
            }
            else if (selected_profile == physics_sim::FluidSolverProfile::Quality)
            {
                require_less_than(settled, "average_speed", settled.average_speed, 2.5);
                require_less_than(settled, "max_speed", settled.max_speed, 15.0);
                require_less_than(settled, "kinetic_energy", settled.kinetic_energy, 250.0);
            }
            else
            {
                require_less_than(settled, "average_speed", settled.average_speed, 3.0);
                require_less_than(settled, "max_speed", settled.max_speed, 10.0);
                require_less_than(settled, "kinetic_energy", settled.kinetic_energy, 200.0);
            }
            require_less_than(settled, "mass_error", settled.mass_error, 1.0e-6);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(settled, "surface_height_jitter", settled.surface_height_jitter, 3.75);
            }
            else if (selected_profile == physics_sim::FluidSolverProfile::Quality)
            {
                require_less_than(settled, "surface_height_jitter", settled.surface_height_jitter, 2.5);
            }
            else
            {
                require_less_than(settled, "surface_height_jitter", settled.surface_height_jitter, 4.5);
            }
            require_less_than(settled, "divergence_l2", settled.divergence_l2, 0.2);
            require_count(settled, "particles_in_solids", settled.particles_in_solids, 0);
            require_greater_than_or_equal(settled, "pooled_height", settled.pooled_height, 1.0);
            require_less_than(settled, "pooled_height", settled.pooled_height, 18.0);
        },
    };
}

ScenarioCase make_hydrostatic_column_case()
{
    return ScenarioCase{
        "hydrostatic-column",
        []()
        {
            return physics_sim::WaterSimulation2D{16, 24, 1.0f};
        },
        {0, 240, 480, 1200},
        20.0f,
        4,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);

            for (std::size_t y = 4; y <= 20; ++y)
            {
                simulation.set_solid_cell(5, y, true);
                simulation.set_solid_cell(10, y, true);
            }
            for (std::size_t x = 5; x <= 10; ++x)
            {
                simulation.set_solid_cell(x, 20, true);
            }

            for (std::size_t y = 5; y <= 19; ++y)
            {
                for (std::size_t x = 6; x <= 9; ++x)
                {
                    add_full_cell_particle(simulation, Vec2{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f});
                }
            }
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_less_than(final, "divergence_l2", final.divergence_l2, 0.15);
            require_less_than(final, "hydrostatic_pressure_shape_error", final.hydrostatic_pressure_shape_error, 0.5);
            REQUIRE(std::isfinite(final.pressure_l2), "pressure l2 became non-finite in hydrostatic column");
        },
    };
}

ScenarioCase make_particle_overcrowding_case()
{
    return ScenarioCase{
        "particle-overcrowding",
        []()
        {
            return physics_sim::WaterSimulation2D{16, 16, 1.0f};
        },
        {0, 2, 4, 8},
        std::nullopt,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);

            auto settings = simulation.solver_settings();
            settings.resampling.enabled = true;
            settings.resampling.min_particles_per_fluid_cell = 2;
            settings.resampling.target_particles_per_fluid_cell = 4;
            settings.resampling.max_particles_per_fluid_cell = 8;
            settings.resampling.max_resampling_operations_per_step = 64;
            settings.resampling.split_offset_fraction = 0.20f;
            settings.resampling.min_split_particle_mass = 1.0e-6f;
            settings.density_correction_iterations = 1;
            settings.max_density_correction_fraction = 0.15f;
            simulation.set_solver_settings(settings);

            for (std::size_t index = 0; index < 16; ++index)
            {
                const float x = 7.0f + static_cast<float>(index % 4) * 0.02f;
                const float y = 7.0f + static_cast<float>(index / 4) * 0.02f;
                simulation.add_particle({Vec2{x, y}, Vec2{0.0f, 0.0f}});
            }
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& initial = first.snapshots.front();
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_less_than(final, "particles_out_of_domain", static_cast<double>(final.particles_out_of_domain), 0.5);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(final, "average_density_error", final.average_density_error, 0.55);
                require_less_than(final, "kinetic_energy", final.kinetic_energy, 250.0);
                require_less_than(final, "surface_height_jitter", final.surface_height_jitter, 0.9);
            }
            else if (selected_profile == physics_sim::FluidSolverProfile::Quality)
            {
                require_less_than(final, "average_density_error", final.average_density_error, 1.0);
                require_less_than(final, "max_density_error", final.max_density_error, 1.25);
            }

            const double drift_x = static_cast<double>(final.center_of_mass.x - initial.center_of_mass.x);
            const double drift_y = static_cast<double>(final.center_of_mass.y - initial.center_of_mass.y);
            const double drift = std::sqrt(drift_x * drift_x + drift_y * drift_y);
            std::ostringstream stream;
            stream << snapshot_label(final)
                   << " metric=center_of_mass_drift expected<=0.25 actual=" << format_value(drift);
            REQUIRE(drift <= 0.25, stream.str());
        },
    };
}

ScenarioCase make_u_container_case()
{
    return ScenarioCase{
        "u-container-fill",
        []()
        {
            return physics_sim::WaterSimulation2D{80, 45, 16.0f};
        },
        {0, 120, 720},
        36.0f * 16.0f,
        32,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            for (std::size_t y = 24; y <= 35; ++y)
            {
                simulation.set_solid_cell(20, y, true);
                simulation.set_solid_cell(60, y, true);
            }
            for (std::size_t x = 20; x <= 60; ++x)
            {
                simulation.set_solid_cell(x, 36, true);
            }

            simulation.add_emitter(directional_emitter(Vec2{640.0f, 160.0f}, Vec2{0.0f, 1.0f}, 8.0f, 42.0f));
        },
        [](physics_sim::WaterSimulation2D& simulation, std::uint64_t tick)
        {
            if (tick == 120 && !simulation.emitters().empty())
            {
                simulation.emitters().front().enabled = false;
            }
        },
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_greater_than(final, "total_emitted", static_cast<double>(final.metrics.total_emitted), 0.0);
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_less_than(final, "average_divergence", final.metrics.average_divergence_after_projection, 0.2);
            require_less_than(final, "max_divergence", final.metrics.max_divergence_after_projection, 0.6);
            require_greater_than(final, "pooled_height", final.pooled_height, 0.4);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(final, "pooled_height", final.pooled_height, 27.0);
            }
            else
            {
                require_less_than(final, "pooled_height", final.pooled_height, 22.0);
            }
        },
    };
}

ScenarioCase make_dam_break_case()
{
    return ScenarioCase{
        "dam-break",
        []()
        {
            return physics_sim::WaterSimulation2D{24, 12, 1.0f};
        },
        {0, 60, 120, 480},
        10.0f,
        9,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t y = 1; y <= 10; ++y)
            {
                if (y != 4 && y != 5 && y != 6)
                {
                    simulation.set_solid_cell(12, y, true);
                }
            }

            simulation.add_gate(physics_sim::WaterGate{12, 4, false});
            simulation.add_gate(physics_sim::WaterGate{12, 5, false});
            simulation.add_gate(physics_sim::WaterGate{12, 6, false});

            for (std::size_t y = 1; y <= 9; ++y)
            {
                for (std::size_t x = 2; x <= 11; ++x)
                {
                    add_full_cell_particle(simulation, Vec2{static_cast<float>(x) + 0.3f, static_cast<float>(y) + 0.3f});
                }
            }
        },
        [](physics_sim::WaterSimulation2D& simulation, std::uint64_t tick)
        {
            if (tick == 30)
            {
                static_cast<void>(simulation.toggle_gate_open(0));
                static_cast<void>(simulation.toggle_gate_open(1));
                static_cast<void>(simulation.toggle_gate_open(2));
            }
        },
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& initial = first.snapshots.front();
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "particles_out_of_domain", static_cast<double>(final.particles_out_of_domain), 0.5);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_greater_than(final, "water_front_x", final.max_position.x, initial.max_position.x + 0.1);
        },
    };
}

ScenarioCase make_hose_wall_impact_case()
{
    return ScenarioCase{
        "hose-wall-impact",
        []()
        {
            return physics_sim::WaterSimulation2D{80, 45, 16.0f};
        },
        {0, 120, 360},
        std::nullopt,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            for (std::size_t y = 18; y <= 30; ++y)
            {
                simulation.set_solid_cell(28, y, true);
            }

            simulation.add_emitter(directional_emitter(Vec2{192.0f, 352.0f}, Vec2{1.0f, 0.0f}, 8.0f, 48.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(final, "wall_penetration", final.max_position.x, 500.0);
            }
            else
            {
                require_less_than(final, "wall_penetration", final.max_position.x, 28.0 * 16.0f);
            }
            require_greater_than(final, "deflection", final.max_position.y, 352.0 - 16.0f);
        },
    };
}

ScenarioCase make_hose_wall_pooling_case()
{
    return ScenarioCase{
        "hose-wall-pooling",
        []()
        {
            return physics_sim::WaterSimulation2D{40, 28, 1.0f};
        },
        {0, 180, 360},
        22.0f,
        13,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);

            for (std::size_t y = 8; y <= 22; ++y)
            {
                simulation.set_solid_cell(24, y, true);
            }
            for (std::size_t x = 10; x <= 24; ++x)
            {
                simulation.set_solid_cell(x, 22, true);
            }
            for (std::size_t y = 16; y <= 22; ++y)
            {
                simulation.set_solid_cell(10, y, true);
            }

            simulation.add_emitter(directional_emitter(Vec2{12.0f, 11.0f}, Vec2{1.0f, 0.35f}, 7.0f, 36.0f));
        },
        [](physics_sim::WaterSimulation2D& simulation, std::uint64_t tick)
        {
            if (tick == 180 && !simulation.emitters().empty())
            {
                simulation.emitters().front().enabled = false;
            }
        },
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_count(final, "particles_out_of_domain", final.particles_out_of_domain, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_greater_than(final, "pooled_height", final.pooled_height, 0.2);

            const std::size_t lower_pool_particles = count_particles_in_region(first.final_state, 11, 17, 13, 5);
            std::ostringstream pool_stream;
            pool_stream << snapshot_label(final)
                        << " metric=lower_pool_particles expected>=24 actual="
                        << lower_pool_particles;
            REQUIRE(lower_pool_particles >= 24, pool_stream.str());

            const std::size_t wall_pool_particles = count_particles_in_region(first.final_state, 20, 17, 4, 5);
            std::ostringstream wall_stream;
            wall_stream << snapshot_label(final)
                        << " metric=combined_pool_particles expected>=48 actual="
                        << (wall_pool_particles + lower_pool_particles)
                        << " wall_pool_particles=" << wall_pool_particles
                        << " lower_pool_particles=" << lower_pool_particles
                        << " pool_height=" << format_value(final.pooled_height)
                        << " bounds_x=[" << format_value(final.min_position.x) << ", " << format_value(final.max_position.x) << "]"
                        << " bounds_y=[" << format_value(final.min_position.y) << ", " << format_value(final.max_position.y) << "]";
            REQUIRE(wall_pool_particles + lower_pool_particles >= 48, wall_stream.str());
        },
    };
}

ScenarioCase make_corner_impact_case()
{
    return ScenarioCase{
        "corner-impact",
        []()
        {
            return physics_sim::WaterSimulation2D{80, 45, 16.0f};
        },
        {0, 120, 360},
        std::nullopt,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            for (std::size_t y = 18; y <= 30; ++y)
            {
                simulation.set_solid_cell(28, y, true);
            }

            for (std::size_t x = 18; x <= 28; ++x)
            {
                simulation.set_solid_cell(x, 30, true);
            }

            simulation.add_emitter(directional_emitter(Vec2{192.0f, 352.0f}, Vec2{1.0f, 0.0f}, 8.0f, 48.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(final, "corner_penetration", final.max_position.x, 500.0);
            }
            else
            {
                require_less_than(final, "corner_penetration", final.max_position.x, 28.0 * 16.0f);
            }
            require_greater_than(final, "corner_deflection", final.max_position.y, 352.0 - 16.0f);
        },
    };
}

ScenarioCase make_narrow_channel_case()
{
    return ScenarioCase{
        "narrow-channel",
        []()
        {
            return physics_sim::WaterSimulation2D{32, 24, 1.0f};
        },
        {0, 120, 360},
        18.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t y = 1; y <= 22; ++y)
            {
                if (y != 10 && y != 11)
                {
                    simulation.set_solid_cell(14, y, true);
                    simulation.set_solid_cell(17, y, true);
                }
            }

            simulation.add_emitter(directional_emitter(Vec2{4.5f, 5.5f}, Vec2{0.0f, 1.0f}, 4.0f, 28.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_greater_than(final, "channel_progress", final.center_of_mass.y, 8.0);
            require_greater_than(final, "throughput", final.max_position.y, 12.0);
        },
    };
}

ScenarioCase make_obstacle_field_case()
{
    return ScenarioCase{
        "obstacle-field",
        []()
        {
            return physics_sim::WaterSimulation2D{28, 18, 1.0f};
        },
        {0, 120, 360},
        14.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t y = 4; y <= 13; ++y)
            {
                simulation.set_solid_cell(9, y, true);
            }
            for (std::size_t y = 2; y <= 11; ++y)
            {
                simulation.set_solid_cell(14, y, true);
            }
            for (std::size_t y = 6; y <= 15; ++y)
            {
                simulation.set_solid_cell(19, y, true);
            }

            simulation.add_emitter(directional_emitter(Vec2{4.5f, 6.5f}, Vec2{1.0f, 0.0f}, 4.0f, 32.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "out_of_domain", static_cast<double>(final.particles_out_of_domain), 0.5);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_greater_than(final, "obstacle_reach", final.center_of_mass.x, 6.4);
            }
            else
            {
                require_greater_than(final, "obstacle_reach", final.center_of_mass.x, 6.8);
            }
        },
    };
}

ScenarioCase make_drain_basin_case()
{
    return ScenarioCase{
        "drain-basin",
        []()
        {
            return physics_sim::WaterSimulation2D{24, 18, 1.0f};
        },
        {0, 120, 240},
        14.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t x = 2; x <= 21; ++x)
            {
                simulation.set_solid_cell(x, 14, true);
            }
            simulation.add_drain(physics_sim::WaterDrain{9, 7, 6, 4, true});
            simulation.add_emitter(directional_emitter(Vec2{11.5f, 4.5f}, Vec2{0.0f, 1.0f}, 5.0f, 24.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_greater_than(final, "removed_water", static_cast<double>(final.metrics.total_removed), 0.0);
            require_greater_than(final, "removed_mass", final.metrics.total_removed_mass, 0.0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_less_than(final, "particles_in_solids", static_cast<double>(final.particles_in_solids), 0.5);
        },
    };
}

ScenarioCase make_pumped_loop_case()
{
    return ScenarioCase{
        "pumped-loop",
        []()
        {
            return physics_sim::WaterSimulation2D{22, 22, 1.0f};
        },
        {0, 120, 240},
        16.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t x = 3; x <= 18; ++x)
            {
                simulation.set_solid_cell(x, 6, true);
                simulation.set_solid_cell(x, 15, true);
            }
            for (std::size_t y = 6; y <= 15; ++y)
            {
                simulation.set_solid_cell(3, y, true);
                simulation.set_solid_cell(18, y, true);
            }

            simulation.add_pump(physics_sim::WaterPump{6, 8, 4, 4, true, Vec2{1.0f, 0.0f}, 7.5f});
            simulation.add_emitter(omni_emitter(Vec2{10.5f, 10.5f}, 4.0f, 16.0f));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_greater_than(final, "average_speed", final.average_speed, 0.4);
            require_less_than(final, "max_speed", final.max_speed, 20.0);
        },
    };
}

ScenarioCase make_valve_basin_case()
{
    return ScenarioCase{
        "valve-basin",
        []()
        {
            return physics_sim::WaterSimulation2D{26, 18, 1.0f};
        },
        {0, 120, 240},
        14.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            add_border_walls(simulation);
            for (std::size_t y = 1; y <= 16; ++y)
            {
                if (y != 8)
                {
                    simulation.set_solid_cell(13, y, true);
                }
            }

            simulation.add_valve(physics_sim::WaterValve{13, 8, false});
            simulation.add_emitter(directional_emitter(Vec2{5.5f, 4.5f}, Vec2{0.0f, 1.0f}, 5.0f, 18.0f));
        },
        [](physics_sim::WaterSimulation2D& simulation, std::uint64_t tick)
        {
            if (tick == 120)
            {
                static_cast<void>(simulation.toggle_valve_open(0));
            }
        },
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& initial = first.snapshots.front();
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            require_greater_than(final, "valve_flow", final.center_of_mass.x, initial.center_of_mass.x + 0.25);
        },
    };
}

ScenarioCase make_long_run_stress_case()
{
    return ScenarioCase{
        "long-run-stress",
        []()
        {
            return physics_sim::WaterSimulation2D{12, 12, 1.0f};
        },
        {0, 3000, 6000},
        10.0f,
        std::nullopt,
        [](physics_sim::WaterSimulation2D& simulation)
        {
            if (selected_profile == physics_sim::FluidSolverProfile::Quality)
            {
                auto settings = simulation.solver_settings();
                settings.particles_per_full_cell = 8;
                settings.density_kernel_radius_cells = 1.25f;
                settings.density_correction_iterations = 4;
                settings.max_density_correction_fraction = 0.08f;
                settings.resampling.enabled = true;
                settings.resampling.min_particles_per_fluid_cell = 1;
                settings.resampling.target_particles_per_fluid_cell = 8;
                settings.resampling.max_particles_per_fluid_cell = 16;
                settings.resampling.max_resampling_operations_per_step = 256;
                settings.resampling.split_offset_fraction = 0.16f;
                simulation.set_solver_settings(settings);
            }

            add_border_walls(simulation);
            for (std::size_t x = 0; x < simulation.grid().width(); ++x)
            {
                simulation.set_solid_cell(x, simulation.grid().height() - 1, true);
            }
            for (std::size_t y = 3; y < simulation.grid().height(); ++y)
            {
                simulation.set_solid_cell(0, y, true);
                simulation.set_solid_cell(simulation.grid().width() - 1, y, true);
            }

            const float emission_rate =
                selected_profile == physics_sim::FluidSolverProfile::Balanced ? 3.0f :
                selected_profile == physics_sim::FluidSolverProfile::Quality ? 6.0f :
                12.0f;
            simulation.add_emitter(directional_emitter(Vec2{6.0f, 2.0f}, Vec2{0.0f, 1.0f}, 6.0f, emission_rate));
        },
        {},
        [](const physics_sim::FluidQualityScenarioResult& first, const physics_sim::FluidQualityScenarioResult&)
        {
            const auto& final = first.snapshots.back();
            require_count(final, "particles_in_solids", final.particles_in_solids, 0);
            require_greater_than(final, "total_emitted", static_cast<double>(final.metrics.total_emitted), 0.0);
            require_less_than(final, "mass_error", final.mass_error, 1.0e-6);
            if (selected_profile == physics_sim::FluidSolverProfile::Balanced)
            {
                require_less_than(final, "average_density_error", final.average_density_error, 3.0);
                require_less_than(final, "max_density_error", final.max_density_error, 10.0);
            }
            else if (selected_profile == physics_sim::FluidSolverProfile::Quality)
            {
                require_less_than(final, "average_density_error", final.average_density_error, 1.0);
                require_less_than(final, "max_density_error", final.max_density_error, 1.25);
            }
            REQUIRE(std::isfinite(final.metrics.average_divergence_after_projection), "average divergence became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.metrics.max_divergence_after_projection), "max divergence became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.mass_error), "mass error became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.average_speed), "average speed became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.kinetic_energy), "kinetic energy became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.pressure_l2), "pressure l2 became non-finite in long-run stress");
            REQUIRE(std::isfinite(final.divergence_l2), "divergence l2 became non-finite in long-run stress");
        },
    };
}

std::vector<ScenarioCase> build_cases()
{
    return {
        make_still_pool_case(),
        make_hydrostatic_column_case(),
        make_particle_overcrowding_case(),
        make_u_container_case(),
        make_dam_break_case(),
        make_hose_wall_impact_case(),
        make_hose_wall_pooling_case(),
        make_corner_impact_case(),
        make_narrow_channel_case(),
        make_obstacle_field_case(),
        make_drain_basin_case(),
        make_pumped_loop_case(),
        make_valve_basin_case(),
        make_long_run_stress_case(),
    };
}

std::vector<const ScenarioCase*> select_cases(
    const std::vector<ScenarioCase>& all_cases,
    const std::vector<std::string>& requested)
{
    if (requested.empty())
    {
        std::vector<const ScenarioCase*> selected;
        selected.reserve(all_cases.size());
        for (const auto& scenario : all_cases)
        {
            selected.push_back(&scenario);
        }
        return selected;
    }

    std::vector<const ScenarioCase*> selected;
    for (const auto& request : requested)
    {
        const auto iter = std::find_if(
            all_cases.begin(),
            all_cases.end(),
            [&](const ScenarioCase& scenario) { return scenario.name == request; });

        if (iter == all_cases.end())
        {
            std::ostringstream stream;
            stream << "unknown scenario: " << request;
            fail(stream.str(), __FILE__, __LINE__);
        }

        selected.push_back(&*iter);
    }

    return selected;
}

void verify_snapshot_uses_mass_weighted_kinetic_energy()
{
    physics_sim::WaterSimulation2D simulation{4, 4, 1.0f};
    simulation.add_particle({Vec2{1.5f, 1.5f}, Vec2{2.0f, 0.0f}, 3.0f, 3.0f});
    simulation.add_particle({Vec2{2.5f, 1.5f}, Vec2{0.0f, 2.0f}, 5.0f, 5.0f});

    const auto snapshot = physics_sim::capture_fluid_quality_snapshot(simulation, "kinetic-energy", 0);
    REQUIRE(nearly_equal(snapshot.kinetic_energy, 16.0, 0.000001), "snapshot kinetic energy was not mass weighted");
}

physics_sim::FluidSolverProfile parse_profile(int argc, char* argv[])
{
    physics_sim::FluidSolverProfile profile = physics_sim::FluidSolverProfile::Balanced;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if (arg == "--profile" && i + 1 < argc)
        {
            const std::string_view value = argv[++i];
            const auto parsed = physics_sim::parse_solver_profile_token(value);
            if (!parsed.has_value())
            {
                std::ostringstream stream;
                stream << "unknown solver profile: " << value;
                fail(stream.str(), __FILE__, __LINE__);
            }

            profile = *parsed;
            continue;
        }
    }

    return profile;
}
} // namespace

int main(int argc, char* argv[])
{
    verify_snapshot_uses_mass_weighted_kinetic_energy();

    std::vector<std::string> requested_scenarios;
    const physics_sim::FluidSolverProfile profile = parse_profile(argc, argv);
    selected_profile = profile;

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg = argv[i];
        if (arg == "--scenario" && i + 1 < argc)
        {
            requested_scenarios.emplace_back(argv[++i]);
            continue;
        }

        if (arg == "--profile" && i + 1 < argc)
        {
            ++i;
            continue;
        }

        if (arg == "--list-scenarios")
        {
            for (const auto& scenario : build_cases())
            {
                std::printf("%s\n", scenario.name.c_str());
            }
            return 0;
        }

        std::ostringstream stream;
        stream << "unknown command-line argument: " << arg;
        fail(stream.str(), __FILE__, __LINE__);
    }

    const std::vector<ScenarioCase> all_cases = build_cases();
    const std::vector<const ScenarioCase*> selected_cases = select_cases(all_cases, requested_scenarios);

    for (const ScenarioCase* scenario : selected_cases)
    {
        const auto first_start = std::chrono::steady_clock::now();
        const physics_sim::FluidQualityScenarioResult first = run_case(*scenario, profile);
        const double first_elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - first_start).count();

        const auto second_start = std::chrono::steady_clock::now();
        const physics_sim::FluidQualityScenarioResult second = run_case(*scenario, profile);
        const double second_elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - second_start).count();
        const double elapsed_ms = (first_elapsed_ms + second_elapsed_ms) * 0.5;

        verify_repeatable(*scenario, first, second);
        REQUIRE(first.final_state.solver_settings().profile == profile, "fluid quality scenario did not run with the selected solver profile");
        REQUIRE(second.final_state.solver_settings().profile == profile, "fluid quality scenario did not preserve the selected solver profile on the repeated run");

        const auto& final_snapshot = first.snapshots.back();
        const char* profile_name = physics_sim::solver_profile_name(profile);
        const char* tier = first.final_state.solver_settings().tier == physics_sim::FluidSolverQualityTier::Offline ? "offline" : "live";
        std::printf(
            "scenario=%s profile=%s tier=%s ticks=%zu final_tick=%llu particles=%zu mass_error=%.6f div_l2=%.6f avg_div=%.6f max_div=%.6f density_error=%.6f kinetic_energy=%.6f pressure_residual=%.6f surface_height=%.6f surface_jitter=%.6f elapsed_ms=%.2f removed=%llu outflow=%llu\n",
            scenario->name.c_str(),
            profile_name,
            tier,
            first.snapshots.size(),
            static_cast<unsigned long long>(final_snapshot.tick),
            final_snapshot.particle_count,
            final_snapshot.mass_error,
            final_snapshot.divergence_l2,
            final_snapshot.metrics.average_divergence_after_projection,
            final_snapshot.metrics.max_divergence_after_projection,
            final_snapshot.metrics.max_density_error,
            final_snapshot.kinetic_energy,
            final_snapshot.metrics.pressure_solve.relative_residual,
            final_snapshot.surface_height,
            final_snapshot.surface_height_jitter,
            elapsed_ms,
            static_cast<unsigned long long>(final_snapshot.metrics.total_removed),
            static_cast<unsigned long long>(final_snapshot.metrics.total_outflow));
    }

    return 0;
}
