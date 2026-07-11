#include <physics_sim/scene_document.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

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

bool nearly_equal(float lhs, float rhs, float epsilon = 0.0001f) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}
} // namespace

int main()
{
    namespace fs = std::filesystem;
    using physics_sim::Vec2;

    physics_sim::WaterSimulation2D original{12, 9, 1.5f};
    original.set_solid_cell(2, 3, true);
    original.set_solid_cell(3, 3, true);
    original.set_solid_cell(4, 3, true);
    original.set_solver_settings(physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Quality));

    physics_sim::WaterEmitter directional;
    directional.kind = physics_sim::WaterEmitterKind::Directional;
    directional.position = Vec2{4.5f, 2.0f};
    directional.direction = Vec2{0.0f, 1.0f};
    directional.speed = 7.5f;
    directional.emission_rate = 22.0f;
    directional.enabled = false;
    original.add_emitter(directional);

    physics_sim::WaterEmitter omni;
    omni.kind = physics_sim::WaterEmitterKind::Omni;
    omni.position = Vec2{6.0f, 5.0f};
    omni.direction = Vec2{1.0f, 0.0f};
    omni.speed = 4.0f;
    omni.emission_rate = 12.0f;
    original.add_emitter(omni);

    const auto snapshot = physics_sim::capture_scene(original);
    REQUIRE(snapshot.grid_width == 12, "capture_scene lost grid width");
    REQUIRE(snapshot.grid_height == 9, "capture_scene lost grid height");
    REQUIRE(nearly_equal(snapshot.cell_size, 1.5f), "capture_scene lost cell size");
    REQUIRE(snapshot.solver_profile == physics_sim::FluidSolverProfile::Quality, "capture_scene lost solver profile");
    REQUIRE(snapshot.solid_cells.size() == 3, "capture_scene lost solid cells");
    REQUIRE(snapshot.emitters.size() == 2, "capture_scene lost emitters");

    {
        const fs::path metadata_path = fs::temp_directory_path() / "physics-sim-scene-metadata.pscene";
        physics_sim::SceneMetadata metadata;
        metadata.title = "Demo Basin";
        metadata.description = "Directional hose feeding a basin.";
        metadata.author = "Nic";
        metadata.tags = {"demo", "water", "basin"};
        metadata.notes = {"Use this for regression captures.", "The thumbnail lives next to the scene file."};
        metadata.challenge = physics_sim::SceneChallenge{"Hold the goal", 1, 120, 500.0, 10.0};
        auto metadata_simulation = original;
        metadata_simulation.add_sensor({1, 1, 2, 2, true, false, true, "Goal"});

        REQUIRE(physics_sim::save_scene(metadata_path, metadata_simulation, metadata), "save_scene failed for metadata snapshot");

        physics_sim::WaterSimulation2D metadata_restored;
        physics_sim::SceneMetadata loaded_metadata;
        REQUIRE(physics_sim::load_scene(metadata_path, metadata_restored, &loaded_metadata), "load_scene failed for metadata snapshot");
        REQUIRE(loaded_metadata.title == "Demo Basin", "load_scene lost scene title");
        REQUIRE(loaded_metadata.description == "Directional hose feeding a basin.", "load_scene lost scene description");
        REQUIRE(loaded_metadata.author == "Nic", "load_scene lost scene author");
        REQUIRE(loaded_metadata.tags.size() == 3, "load_scene lost scene tags");
        REQUIRE(loaded_metadata.tags[0] == "demo", "load_scene lost the first tag");
        REQUIRE(loaded_metadata.tags[2] == "basin", "load_scene lost the last tag");
        REQUIRE(loaded_metadata.notes.size() == 2, "load_scene lost scene notes");
        REQUIRE(loaded_metadata.notes[0] == "Use this for regression captures.", "load_scene lost the first note");
        REQUIRE(loaded_metadata.notes[1] == "The thumbnail lives next to the scene file.", "load_scene lost the second note");
        REQUIRE(loaded_metadata.challenge.has_value(), "load_scene lost challenge metadata");
        REQUIRE(loaded_metadata.challenge->hold_ticks == 120, "load_scene lost challenge hold duration");
        REQUIRE(loaded_metadata.challenge->maximum_emitted_mass == 500.0, "load_scene lost challenge emitted budget");
        REQUIRE(metadata_restored.grid().width() == 12, "load_scene lost grid width in metadata overload");
        REQUIRE(metadata_restored.emitters().size() == 2, "load_scene lost emitters in metadata overload");
        REQUIRE(metadata_restored.solver_settings().profile == physics_sim::FluidSolverProfile::Quality, "load_scene lost solver profile in metadata overload");

        fs::remove(metadata_path);
    }

    {
        const fs::path device_path = fs::temp_directory_path() / "physics-sim-scene-devices.pscene";
        physics_sim::SceneDocument device_document = physics_sim::capture_scene(original);
        device_document.gates.push_back({4, 5, true});
        device_document.sensors.push_back({6, 5, 3, 3, true, true, true, "Goal"});
        device_document.drains.push_back({7, 4, 3, 3, true});
        device_document.pumps.push_back({2, 5, 3, 3, true, physics_sim::Vec2{1.0f, 0.0f}, 11.0f});
        device_document.valves.push_back({9, 5, false});

        REQUIRE(physics_sim::save_scene(device_path, device_document), "save_scene failed for device snapshot");

        physics_sim::WaterSimulation2D device_restored;
        REQUIRE(physics_sim::load_scene(device_path, device_restored), "load_scene failed for device snapshot");
        REQUIRE(device_restored.gates().size() == 1, "load_scene lost scene gates");
        REQUIRE(device_restored.gates().front().open, "load_scene lost the gate open state");
        REQUIRE(device_restored.sensors().size() == 1, "load_scene lost scene sensors");
        REQUIRE(device_restored.sensors().front().objective, "load_scene lost the sensor objective flag");
        REQUIRE(device_restored.sensors().front().active, "load_scene lost the sensor active state");
        REQUIRE(device_restored.sensors().front().label == "Goal", "load_scene lost the sensor label");
        REQUIRE(device_restored.drains().size() == 1, "load_scene lost scene drains");
        REQUIRE(device_restored.drains().front().enabled, "load_scene lost the drain enabled state");
        REQUIRE(device_restored.pumps().size() == 1, "load_scene lost scene pumps");
        REQUIRE(nearly_equal(device_restored.pumps().front().direction.x, 1.0f), "load_scene lost the pump direction");
        REQUIRE(nearly_equal(device_restored.pumps().front().strength, 11.0f), "load_scene lost the pump strength");
        REQUIRE(device_restored.valves().size() == 1, "load_scene lost scene valves");
        REQUIRE(device_restored.valves().front().open == false, "load_scene lost the valve open state");

        fs::remove(device_path);
    }

    const fs::path temp_path = fs::temp_directory_path() / "physics-sim-scene-roundtrip.pscene";
    REQUIRE(physics_sim::save_scene(temp_path, snapshot), "save_scene failed");
    REQUIRE(fs::exists(temp_path), "save_scene did not create a file");

    const auto loaded = physics_sim::load_scene(temp_path);
    REQUIRE(loaded.has_value(), "load_scene failed");
    REQUIRE(loaded->solid_cells.size() == 3, "load_scene lost solid cells");
    REQUIRE(loaded->emitters.size() == 2, "load_scene lost emitters");
    REQUIRE(loaded->grid_width == 12, "load_scene lost grid width");
    REQUIRE(loaded->grid_height == 9, "load_scene lost grid height");
    REQUIRE(nearly_equal(loaded->cell_size, 1.5f), "load_scene lost cell size");
    REQUIRE(loaded->solver_profile == physics_sim::FluidSolverProfile::Quality, "load_scene lost solver profile");

    physics_sim::WaterSimulation2D restored;
    physics_sim::apply_scene(*loaded, restored);

    REQUIRE(restored.grid().width() == 12, "apply_scene lost grid width");
    REQUIRE(restored.grid().height() == 9, "apply_scene lost grid height");
    REQUIRE(nearly_equal(restored.grid().cell_size(), 1.5f), "apply_scene lost cell size");
    REQUIRE(restored.grid().solid(2, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.grid().solid(3, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.grid().solid(4, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.emitters().size() == 2, "apply_scene lost emitters");
    REQUIRE(restored.emitters().front().kind == physics_sim::WaterEmitterKind::Directional, "apply_scene changed emitter kind");
    REQUIRE(nearly_equal(restored.emitters().front().position.x, 4.5f), "apply_scene lost emitter position");
    REQUIRE(nearly_equal(restored.emitters().front().speed, 7.5f), "apply_scene lost emitter speed");
    REQUIRE(restored.emitters().back().kind == physics_sim::WaterEmitterKind::Omni, "apply_scene changed omni emitter kind");
    REQUIRE(nearly_equal(restored.emitters().back().emission_rate, 12.0f), "apply_scene lost omni emission rate");
    REQUIRE(restored.gates().empty(), "apply_scene unexpectedly created gates");
    REQUIRE(restored.sensors().empty(), "apply_scene unexpectedly created sensors");
    REQUIRE(restored.solver_settings().profile == physics_sim::FluidSolverProfile::Quality, "apply_scene lost solver profile");

    {
        const auto unsupported = physics_sim::parse_scene_text(
            "physics-sim-scene 1\n"
            "grid 2 2 1\n");
        REQUIRE(!unsupported.has_value(), "parse_scene_text accepted intentionally unsupported version 1");
    }

    {
        const auto supported = physics_sim::parse_scene_text(
            "physics-sim-scene 2\n"
            "solver-profile fast\n"
            "grid 2 2 1\n");
        REQUIRE(supported.has_value(), "parse_scene_text rejected supported version 2");
        REQUIRE(supported->solver_profile == physics_sim::FluidSolverProfile::Fast, "version 2 scene lost solver profile");

        physics_sim::WaterSimulation2D fallback_target;
        physics_sim::apply_scene(*supported, fallback_target, physics_sim::FluidSolverProfile::Quality);
        REQUIRE(fallback_target.solver_settings().profile == physics_sim::FluidSolverProfile::Fast, "scene solver profile did not override fallback");

        physics_sim::WaterSimulation2D forced_target;
        physics_sim::apply_scene(*supported, forced_target, physics_sim::FluidSolverProfile::Quality, physics_sim::FluidSolverProfile::Balanced);
        REQUIRE(forced_target.solver_settings().profile == physics_sim::FluidSolverProfile::Balanced, "forced solver profile did not override scene profile");
    }

    {
        const auto unsupported = physics_sim::parse_scene_text(
            "physics-sim-scene 3\n"
            "grid 2 2 1\n");
        REQUIRE(!unsupported.has_value(), "parse_scene_text accepted unsupported version 3");
    }

    {
        const fs::path atomic_path = fs::temp_directory_path() / "physics-sim-atomic-scene.pscene";
        fs::remove(atomic_path);
        fs::remove(fs::path{atomic_path.string() + ".bak"});
        REQUIRE(physics_sim::save_scene(atomic_path, snapshot), "initial atomic scene save failed");
        auto changed = snapshot;
        changed.metadata.title = "Atomic replacement";
        REQUIRE(physics_sim::save_scene(atomic_path, changed), "atomic scene replacement failed");
        REQUIRE(fs::exists(fs::path{atomic_path.string() + ".bak"}), "atomic replacement did not retain a backup");
        const auto replacement = physics_sim::load_scene(atomic_path);
        const auto backup = physics_sim::load_scene(fs::path{atomic_path.string() + ".bak"});
        REQUIRE(replacement.has_value() && replacement->metadata.title == "Atomic replacement", "atomic replacement did not publish the new scene");
        REQUIRE(backup.has_value() && backup->metadata.title.empty(), "atomic replacement backup did not preserve the previous scene");
        fs::remove(atomic_path);
        fs::remove(fs::path{atomic_path.string() + ".bak"});
    }

    {
        const auto invalid_profile = physics_sim::parse_scene_text(
            "physics-sim-scene 2\n"
            "solver-profile impossible\n"
            "grid 2 2 1\n");
        REQUIRE(!invalid_profile.has_value(), "parse_scene_text accepted invalid solver profile");
    }

    {
        const auto malformed = physics_sim::parse_scene_text(
            "physics-sim-scene not-a-number\n"
            "grid 2 2 1\n");
        REQUIRE(!malformed.has_value(), "parse_scene_text accepted malformed version token");
    }

    {
        const auto out_of_bounds_device = physics_sim::parse_scene_text(
            "physics-sim-scene 2\n"
            "solver-profile balanced\n"
            "grid 8 8 1\n"
            "pump 7 7 3 3 1 1 0 8\n");
        REQUIRE(!out_of_bounds_device.has_value(), "parse_scene_text accepted an out-of-bounds device region");
        const auto invalid_emitter = physics_sim::parse_scene_text(
            "physics-sim-scene 2\n"
            "solver-profile balanced\n"
            "grid 8 8 1\n"
            "emitter directional 9 2 1 0 5 10 1\n");
        REQUIRE(!invalid_emitter.has_value(), "parse_scene_text accepted an out-of-bounds emitter");
        const auto invalid_challenge = physics_sim::parse_scene_text(
            "physics-sim-scene 2\nsolver-profile balanced\ngrid 8 8 1\nchallenge 1 0 -1 -1 Invalid\n");
        REQUIRE(!invalid_challenge.has_value(), "parse_scene_text accepted a zero-duration challenge");
        const auto missing_objective = physics_sim::parse_scene_text(
            "physics-sim-scene 2\nsolver-profile balanced\ngrid 8 8 1\nchallenge 1 10 -1 -1 Missing target\n");
        REQUIRE(!missing_objective.has_value(), "parse_scene_text accepted a challenge without objective sensors");
    }

    {
        physics_sim::WaterSimulation2D missing_load_target{2, 2, 1.0f};
        const fs::path missing_path = fs::temp_directory_path() / "physics-sim-missing-scene.pscene";
        fs::remove(missing_path);
        REQUIRE(!physics_sim::load_scene(missing_path, missing_load_target), "load_scene succeeded on a missing file");
    }

    fs::remove(temp_path);
    return 0;
}
