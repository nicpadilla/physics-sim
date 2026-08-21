from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CHANGED: list[str] = []


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def write(path: str, content: str) -> None:
    target = ROOT / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8", newline="\n")
    CHANGED.append(path)


def replace_once(path: str, old: str, new: str) -> None:
    text = read(path)
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{path}: expected one occurrence, found {count}: {old[:120]!r}")
    write(path, text.replace(old, new, 1))


def function_range(text: str, signature: str) -> tuple[int, int]:
    start = text.find(signature)
    if start < 0:
        raise RuntimeError(f"missing function: {signature}")
    brace = text.find("{", start + len(signature))
    if brace < 0:
        raise RuntimeError(f"missing opening brace: {signature}")
    depth = 0
    in_string = False
    in_char = False
    escaped = False
    line_comment = False
    block_comment = False
    index = brace
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            index += 1
            continue
        if in_char:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == "'":
                in_char = False
            index += 1
            continue
        if char == "/" and following == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and following == "*":
            block_comment = True
            index += 2
            continue
        if char == '"':
            in_string = True
        elif char == "'":
            in_char = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return start, index + 1
        index += 1
    raise RuntimeError(f"missing closing brace: {signature}")


def replace_function(path: str, signature: str, replacement: str) -> None:
    text = read(path)
    start, end = function_range(text, signature)
    write(path, text[:start] + replacement.rstrip() + text[end:])


write(
    "include/physics_sim/state_digest.hpp",
    r'''#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace physics_sim
{
inline constexpr std::uint32_t StateDigestFormatVersion = 2;

enum class StateDigestDomain : std::uint32_t
{
    SceneDefinition = 1,
    PhysicsRuntime = 2,
    RenderedOutput = 3,
};

class StateDigestBuilder
{
public:
    explicit StateDigestBuilder(StateDigestDomain domain) noexcept
    {
        mix_u64(0x5053494D44494745ULL); // "PSIMDIGE"
        mix_u64(StateDigestFormatVersion);
        mix_enum(domain);
    }

    void mix_u64(std::uint64_t value) noexcept
    {
        for (int byte = 0; byte < 8; ++byte)
        {
            hash_ ^= static_cast<std::uint8_t>((value >> (byte * 8)) & 0xffU);
            hash_ *= 1099511628211ULL;
        }
    }

    void mix_i64(std::int64_t value) noexcept { mix_u64(static_cast<std::uint64_t>(value)); }
    void mix_bool(bool value) noexcept { mix_u64(value ? 1U : 0U); }
    void mix_float(float value) noexcept { mix_u64(std::bit_cast<std::uint32_t>(value)); }
    void mix_double(double value) noexcept { mix_u64(std::bit_cast<std::uint64_t>(value)); }

    template <typename Enum>
    void mix_enum(Enum value) noexcept
    {
        static_assert(std::is_enum_v<Enum>);
        using Underlying = std::underlying_type_t<Enum>;
        mix_u64(static_cast<std::uint64_t>(static_cast<Underlying>(value)));
    }

    void mix_string(std::string_view value) noexcept
    {
        mix_u64(value.size());
        for (const unsigned char byte : value)
        {
            hash_ ^= byte;
            hash_ *= 1099511628211ULL;
        }
    }

    [[nodiscard]] std::string finish() const
    {
        std::ostringstream stream;
        stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash_;
        return stream.str();
    }

private:
    std::uint64_t hash_ = 14695981039346656037ULL;
};
} // namespace physics_sim
''',
)

# state_digest.hpp uses bit_cast.
replace_once(
    "include/physics_sim/state_digest.hpp",
    "#include <cstdint>\n",
    "#include <bit>\n#include <cstdint>\n",
)

water = "include/physics_sim/water_simulation.hpp"
replace_once(
    water,
    "#include <physics_sim/solver_profile.hpp>\n",
    "#include <physics_sim/solver_profile.hpp>\n#include <physics_sim/state_digest.hpp>\n",
)
replace_once(
    water,
    "    [[nodiscard]] std::string state_digest() const\n",
    "    [[nodiscard]] static constexpr std::uint32_t state_digest_version() noexcept\n"
    "    {\n"
    "        return StateDigestFormatVersion;\n"
    "    }\n\n"
    "    [[nodiscard]] std::string state_digest() const\n",
)
replace_function(
    water,
    "    [[nodiscard]] std::string state_digest() const",
    r'''    [[nodiscard]] std::string state_digest() const
    {
        StateDigestBuilder digest{StateDigestDomain::PhysicsRuntime};
        digest.mix_string("WaterSimulation2D");
        const auto mix_size = [&digest](std::size_t value) { digest.mix_u64(static_cast<std::uint64_t>(value)); };
        const auto mix_float_vector = [&digest, &mix_size](const std::vector<float>& values)
        {
            mix_size(values.size());
            for (const float value : values) digest.mix_float(value);
        };
        const auto mix_double_vector = [&digest, &mix_size](const std::vector<double>& values)
        {
            mix_size(values.size());
            for (const double value : values) digest.mix_double(value);
        };
        const auto mix_int_vector = [&digest, &mix_size](const std::vector<int>& values)
        {
            mix_size(values.size());
            for (const int value : values) digest.mix_i64(value);
        };
        const auto mix_size_vector = [&digest, &mix_size](const std::vector<size_type>& values)
        {
            mix_size(values.size());
            for (const size_type value : values) mix_size(value);
        };

        digest.mix_u64(simulation_tick_);
        mix_size(grid_.width());
        mix_size(grid_.height());
        digest.mix_float(grid_.cell_size());

        const FluidSolverSettings& settings = solver_settings_;
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

        mix_size(particles_.size());
        for (const FluidParticle& particle : particles_)
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

        mix_float_vector(grid_.pressure_values());
        mix_float_vector(grid_.divergence_values());
        mix_float_vector(grid_.u_values());
        mix_float_vector(grid_.v_values());
        for (std::size_t y = 0; y < grid_.height(); ++y)
        {
            for (std::size_t x = 0; x < grid_.width(); ++x)
            {
                digest.mix_bool(grid_.solid(x, y));
            }
        }

        mix_size(emitters_.size());
        for (const WaterEmitter& emitter : emitters_)
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
        mix_size(gates_.size());
        for (const WaterGate& gate : gates_)
        {
            mix_size(gate.x);
            mix_size(gate.y);
            digest.mix_bool(gate.open);
        }
        mix_size(sensors_.size());
        for (const WaterSensor& sensor : sensors_)
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
        mix_size(drains_.size());
        for (const WaterDrain& drain : drains_)
        {
            mix_size(drain.x);
            mix_size(drain.y);
            mix_size(drain.width);
            mix_size(drain.height);
            digest.mix_bool(drain.enabled);
        }
        mix_size(pumps_.size());
        for (const WaterPump& pump : pumps_)
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
        mix_size(valves_.size());
        for (const WaterValve& valve : valves_)
        {
            mix_size(valve.x);
            mix_size(valve.y);
            digest.mix_bool(valve.open);
        }

        mix_size(fluid_cells_.size());
        for (const std::uint8_t value : fluid_cells_) digest.mix_u64(value);
        mix_size(cell_states_.size());
        for (const FluidCellState value : cell_states_) digest.mix_enum(value);
        mix_float_vector(cell_volume_fractions_);
        mix_float_vector(cell_densities_);
        mix_float_vector(u_weights_);
        mix_float_vector(v_weights_);
        mix_float_vector(u_previous_);
        mix_float_vector(v_previous_);
        mix_float_vector(pressure_next_);
        mix_double_vector(pressure_workspace_);
        mix_double_vector(pressure_rhs_workspace_);
        mix_double_vector(pressure_residual_workspace_);
        mix_double_vector(pressure_direction_workspace_);
        mix_double_vector(pressure_preconditioned_workspace_);
        mix_double_vector(pressure_applied_workspace_);
        mix_double_vector(pressure_inverse_diagonal_workspace_);
        mix_int_vector(pressure_cell_to_system_workspace_);
        mix_size_vector(pressure_system_to_cell_workspace_);

        digest.mix_u64(total_emitted_);
        digest.mix_double(total_emitted_mass_);
        digest.mix_u64(total_removed_);
        digest.mix_double(total_removed_mass_);
        digest.mix_u64(total_outflow_);
        digest.mix_double(total_outflow_mass_);

        const WaterSimulationMetrics& metrics = metrics_;
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

        const PressureSolveResult& pressure = metrics.pressure_solve;
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
    }''',
)

scene = "include/physics_sim/scene_document.hpp"
replace_once(
    scene,
    "#include <physics_sim/math.hpp>\n",
    "#include <physics_sim/math.hpp>\n#include <physics_sim/state_digest.hpp>\n",
)
scene_text = read(scene)
anchor = "// Recovery scene v2 is intentionally incompatible with the pre-recovery v1 format.\ninline constexpr int SceneFormatVersion = 2;\n"
if anchor not in scene_text:
    raise RuntimeError("scene format anchor missing")
scene_digest = r'''
// Recovery scene v2 is intentionally incompatible with the pre-recovery v1 format.
inline constexpr int SceneFormatVersion = 2;

[[nodiscard]] inline std::string scene_definition_digest(const SceneDocument& document)
{
    StateDigestBuilder digest{StateDigestDomain::SceneDefinition};
    digest.mix_string("SceneDocument");
    const auto mix_size = [&digest](std::size_t value) { digest.mix_u64(static_cast<std::uint64_t>(value)); };
    mix_size(document.grid_width);
    mix_size(document.grid_height);
    digest.mix_float(document.cell_size);
    digest.mix_bool(document.solver_profile.has_value());
    if (document.solver_profile.has_value()) digest.mix_enum(*document.solver_profile);

    digest.mix_string(document.metadata.title);
    digest.mix_string(document.metadata.description);
    digest.mix_string(document.metadata.author);
    mix_size(document.metadata.tags.size());
    for (const std::string& tag : document.metadata.tags) digest.mix_string(tag);
    mix_size(document.metadata.notes.size());
    for (const std::string& note : document.metadata.notes) digest.mix_string(note);
    digest.mix_bool(document.metadata.challenge.has_value());
    if (document.metadata.challenge.has_value())
    {
        const SceneChallenge& challenge = *document.metadata.challenge;
        digest.mix_string(challenge.title);
        mix_size(challenge.required_objective_sensors);
        digest.mix_u64(challenge.hold_ticks);
        digest.mix_bool(challenge.maximum_emitted_mass.has_value());
        if (challenge.maximum_emitted_mass.has_value()) digest.mix_double(*challenge.maximum_emitted_mass);
        digest.mix_bool(challenge.maximum_outflow_mass.has_value());
        if (challenge.maximum_outflow_mass.has_value()) digest.mix_double(*challenge.maximum_outflow_mass);
    }

    mix_size(document.solid_cells.size());
    for (const SceneCell& cell : document.solid_cells)
    {
        mix_size(cell.x);
        mix_size(cell.y);
    }
    mix_size(document.emitters.size());
    for (const SceneEmitter& emitter : document.emitters)
    {
        digest.mix_enum(emitter.kind);
        digest.mix_float(emitter.position.x);
        digest.mix_float(emitter.position.y);
        digest.mix_float(emitter.direction.x);
        digest.mix_float(emitter.direction.y);
        digest.mix_float(emitter.speed);
        digest.mix_float(emitter.emission_rate);
        digest.mix_bool(emitter.enabled);
    }
    mix_size(document.gates.size());
    for (const SceneGate& gate : document.gates)
    {
        mix_size(gate.x);
        mix_size(gate.y);
        digest.mix_bool(gate.open);
    }
    mix_size(document.sensors.size());
    for (const SceneSensor& sensor : document.sensors)
    {
        mix_size(sensor.x);
        mix_size(sensor.y);
        mix_size(sensor.width);
        mix_size(sensor.height);
        digest.mix_bool(sensor.enabled);
        digest.mix_bool(sensor.objective);
        digest.mix_string(sensor.label);
    }
    mix_size(document.drains.size());
    for (const SceneDrain& drain : document.drains)
    {
        mix_size(drain.x);
        mix_size(drain.y);
        mix_size(drain.width);
        mix_size(drain.height);
        digest.mix_bool(drain.enabled);
    }
    mix_size(document.pumps.size());
    for (const ScenePump& pump : document.pumps)
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
    mix_size(document.valves.size());
    for (const SceneValve& valve : document.valves)
    {
        mix_size(valve.x);
        mix_size(valve.y);
        digest.mix_bool(valve.open);
    }
    return digest.finish();
}
'''
write(scene, scene_text.replace(anchor, scene_digest, 1))

simulation_header = "include/physics_sim/simulation.hpp"
replace_once(
    simulation_header,
    "#include <physics_sim/solver_profile.hpp>\n",
    "#include <physics_sim/solver_profile.hpp>\n#include <physics_sim/state_digest.hpp>\n",
)
replace_once(simulation_header, "    std::uint64_t deterministic_seed = 0;\n", "")
replace_once(
    simulation_header,
    "    [[nodiscard]] SimulationMetrics metrics() const noexcept;\n    [[nodiscard]] std::string state_digest() const;\n",
    "    [[nodiscard]] SimulationMetrics metrics() const noexcept;\n"
    "    [[nodiscard]] static constexpr std::uint32_t state_digest_version() noexcept\n"
    "    {\n"
    "        return StateDigestFormatVersion;\n"
    "    }\n"
    "    [[nodiscard]] std::string state_digest() const;\n",
)

simulation_source = "src/core/simulation.cpp"
replace_once(
    simulation_source,
    "#include <physics_sim/water_simulation.hpp>\n",
    "#include <physics_sim/state_digest.hpp>\n#include <physics_sim/water_simulation.hpp>\n",
)
replace_function(
    simulation_source,
    "std::string Simulation::state_digest() const",
    r'''std::string Simulation::state_digest() const
{
    StateDigestBuilder digest{StateDigestDomain::PhysicsRuntime};
    digest.mix_string("SimulationFacade");
    digest.mix_u64(impl_->config.grid_width);
    digest.mix_u64(impl_->config.grid_height);
    digest.mix_float(impl_->config.cell_size);
    digest.mix_double(impl_->config.fixed_timestep);
    digest.mix_float(impl_->config.gravity_acceleration);
    digest.mix_enum(impl_->config.solver_profile);
    digest.mix_u64(impl_->tick);
    digest.mix_bool(impl_->paused);
    digest.mix_bool(impl_->single_step_pending);
    digest.mix_string(impl_->water.state_digest());
    return digest.finish();
}''',
)

write(
    "tests/state_digest_tests.cpp",
    r'''#include <physics_sim/scene_document.hpp>
#include <physics_sim/simulation.hpp>
#include <physics_sim/state_digest.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cstdio>
#include <cstdlib>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
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

physics_sim::WaterSimulation2D make_water()
{
    physics_sim::WaterSimulation2D simulation{8, 8, 1.0f};
    physics_sim::WaterEmitter emitter;
    emitter.position = {4.0f, 1.0f};
    emitter.direction = {0.0f, 1.0f};
    emitter.speed = 3.0f;
    emitter.emission_rate = 60.0f;
    simulation.add_emitter(emitter);
    simulation.add_particle({{3.5f, 3.5f}, {0.25f, -0.5f}, 1.0f, 1.0f});
    return simulation;
}
} // namespace

int main()
{
    using namespace physics_sim;
    REQUIRE(StateDigestFormatVersion == 2, "unexpected state digest format version");
    REQUIRE(WaterSimulation2D::state_digest_version() == StateDigestFormatVersion, "water digest version is not reported");
    REQUIRE(Simulation::state_digest_version() == StateDigestFormatVersion, "facade digest version is not reported");

    const auto baseline = make_water();
    REQUIRE(baseline.state_digest() == make_water().state_digest(), "identical water states produced different digests");

    auto changed = baseline;
    changed.emitters().front().emission_accumulator = 0.25;
    REQUIRE(changed.state_digest() != baseline.state_digest(), "emitter accumulation did not affect the runtime digest");
    changed = baseline;
    changed.emitters().front().emitted_particles = 7;
    REQUIRE(changed.state_digest() != baseline.state_digest(), "emitter phase did not affect the runtime digest");
    changed = baseline;
    auto settings = changed.solver_settings();
    settings.gravity_acceleration += 1.0f;
    changed.set_solver_settings(settings);
    REQUIRE(changed.state_digest() != baseline.state_digest(), "solver settings did not affect the runtime digest");
    changed = baseline;
    changed.grid().u(2, 2) = 1.0f;
    REQUIRE(changed.state_digest() != baseline.state_digest(), "grid velocity did not affect the runtime digest");
    changed = baseline;
    changed.particles().front().affine_velocity.m01 = 0.5f;
    REQUIRE(changed.state_digest() != baseline.state_digest(), "particle affine state did not affect the runtime digest");

    auto stepped = baseline;
    stepped.step(1.0 / 120.0);
    const auto checkpoint = stepped.capture_checkpoint();
    auto restored = baseline;
    REQUIRE(restored.restore_checkpoint(checkpoint), "checkpoint restore failed");
    REQUIRE(restored.state_digest() == stepped.state_digest(), "checkpoint restore did not recover the exact runtime digest");
    for (int index = 0; index < 10; ++index)
    {
        restored.step(1.0 / 120.0);
        stepped.step(1.0 / 120.0);
        REQUIRE(restored.state_digest() == stepped.state_digest(), "equal runtime digests diverged under equal future steps");
    }

    SceneDocument scene;
    scene.grid_width = 8;
    scene.grid_height = 8;
    scene.cell_size = 1.0f;
    scene.solver_profile = FluidSolverProfile::Balanced;
    scene.metadata.title = "Digest scene";
    scene.solid_cells.push_back({2, 3});
    scene.sensors.push_back({1, 1, 2, 2, true, false, true, "goal"});
    const std::string scene_digest = scene_definition_digest(scene);
    auto active_only = scene;
    active_only.sensors.front().active = true;
    REQUIRE(scene_definition_digest(active_only) == scene_digest, "derived sensor activity changed authored scene identity");
    auto changed_scene = scene;
    changed_scene.solid_cells.push_back({3, 3});
    REQUIRE(scene_definition_digest(changed_scene) != scene_digest, "authored wall change did not affect scene identity");
    changed_scene = scene;
    changed_scene.metadata.title = "Different title";
    REQUIRE(scene_definition_digest(changed_scene) != scene_digest, "authored metadata change did not affect scene identity");
    REQUIRE(scene_definition_digest(scene) != baseline.state_digest(), "scene and runtime digest domains collided for the test state");

    SimulationConfig config;
    config.grid_width = 8;
    config.grid_height = 8;
    config.cell_size = 1.0f;
    Simulation facade{config};
    Simulation repeated{config};
    REQUIRE(facade.state_digest() == repeated.state_digest(), "identical facade states produced different digests");
    facade.apply(SetPausedCommand{true});
    REQUIRE(facade.state_digest() != repeated.state_digest(), "pause state did not affect facade runtime identity");
    const std::string paused_digest = facade.state_digest();
    facade.apply(SingleStepCommand{});
    REQUIRE(facade.state_digest() != paused_digest, "pending single step did not affect facade runtime identity");

    SimulationConfig different_gravity = config;
    different_gravity.gravity_acceleration += 1.0f;
    REQUIRE(Simulation{different_gravity}.state_digest() != Simulation{config}.state_digest(), "facade configuration did not affect runtime identity");

    std::puts("state digest tests passed");
    return 0;
}
''',
)

cmake = "CMakeLists.txt"
replace_once(
    cmake,
    "    add_test(NAME physics_sim_runtime_checkpoint_tests COMMAND physics_sim_runtime_checkpoint_tests)\n\n"
    "    add_executable(physics_sim_ui_tests",
    "    add_test(NAME physics_sim_runtime_checkpoint_tests COMMAND physics_sim_runtime_checkpoint_tests)\n\n"
    "    add_executable(physics_sim_state_digest_tests\n"
    "        tests/state_digest_tests.cpp\n"
    "    )\n"
    "    target_link_libraries(physics_sim_state_digest_tests PRIVATE physics_sim_core)\n"
    "    target_compile_features(physics_sim_state_digest_tests PRIVATE cxx_std_20)\n"
    "    if(MSVC)\n"
    "        target_compile_options(physics_sim_state_digest_tests PRIVATE /W4 /permissive- /utf-8)\n"
    "        target_compile_definitions(physics_sim_state_digest_tests PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)\n"
    "    endif()\n"
    "    add_test(NAME physics_sim_state_digest_tests COMMAND physics_sim_state_digest_tests)\n\n"
    "    add_executable(physics_sim_ui_tests",
)
replace_once(
    cmake,
    "        physics_sim_water_visual_effects_tests\n"
    "        physics_sim_challenge_objective_tests",
    "        physics_sim_water_visual_effects_tests\n"
    "        physics_sim_state_digest_tests\n"
    "        physics_sim_challenge_objective_tests",
)

write(".agent/changed_paths.txt", "\n".join(sorted(set(CHANGED))) + "\n")
print(f"applied {len(set(CHANGED))} files")
