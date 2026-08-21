#include <physics_sim/scene_controller.hpp>
#include <physics_sim/scene_state_digest.hpp>
#include <physics_sim/water_simulation.hpp>
#include <physics_sim/water_state_digest.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{
[[noreturn]] void fail(const char *message, const char *file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::exit(1);
}

#define REQUIRE(condition, message)                                                                                                                            \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if (!(condition))                                                                                                                                      \
        {                                                                                                                                                      \
            fail((message), __FILE__, __LINE__);                                                                                                               \
        }                                                                                                                                                      \
    } while (false)

bool nearly_equal(double lhs, double rhs, double epsilon = 1.0e-9)
{
    return std::abs(lhs - rhs) <= epsilon;
}

void add_test_basin(physics_sim::WaterSimulation2D &simulation)
{
    const std::size_t width = simulation.grid().width();
    const std::size_t height = simulation.grid().height();
    for (std::size_t x = 0; x < width; ++x)
    {
        simulation.set_solid_cell(x, height - 1, true);
    }
    simulation.set_solid_cell(0, height - 2, true);
    simulation.set_solid_cell(width - 1, height - 2, true);

    physics_sim::WaterEmitter emitter;
    emitter.position = {static_cast<float>(width) * 0.5f, 1.5f};
    emitter.direction = {0.0f, 1.0f};
    emitter.speed = 3.0f;
    emitter.emission_rate = 90.0f;
    simulation.add_emitter(emitter);
    simulation.add_sensor({2, height - 4, 3, 2, true, false, true, "goal"});
}

void require_same_continuation_state(const physics_sim::WaterSimulation2D &lhs, const physics_sim::WaterSimulation2D &rhs, const char *message)
{
    REQUIRE(lhs.state_digest() == rhs.state_digest(), message);
    REQUIRE(physics_sim::versioned_water_state_digest(lhs, 1.0 / 120.0) == physics_sim::versioned_water_state_digest(rhs, 1.0 / 120.0), message);
    REQUIRE(lhs.simulation_tick() == rhs.simulation_tick(), message);
    REQUIRE(lhs.particles().size() == rhs.particles().size(), message);
    REQUIRE(lhs.emitters().size() == rhs.emitters().size(), message);
    REQUIRE(nearly_equal(lhs.emitters().front().emission_accumulator, rhs.emitters().front().emission_accumulator), message);
    REQUIRE(lhs.emitters().front().emitted_particles == rhs.emitters().front().emitted_particles, message);
    REQUIRE(lhs.metrics().total_emitted == rhs.metrics().total_emitted, message);
}
} // namespace

int main()
{
    constexpr double dt = 1.0 / 120.0;
    REQUIRE(physics_sim::StateDigestFormatVersion == 2, "unexpected versioned digest format");

    {
        physics_sim::WaterSimulation2D simulation{12, 12, 1.0f};
        add_test_basin(simulation);
        for (int step = 0; step < 7; ++step)
        {
            simulation.step(dt);
        }

        const physics_sim::WaterSimulationCheckpoint checkpoint = simulation.capture_checkpoint();
        const auto expected = checkpoint.simulation;
        const std::string digest = simulation.state_digest();
        const std::string versioned_digest = physics_sim::versioned_water_state_digest(simulation, dt);
        const std::uint64_t tick = simulation.simulation_tick();
        const double accumulator = simulation.emitters().front().emission_accumulator;
        const std::uint64_t phase = simulation.emitters().front().emitted_particles;

        for (int step = 0; step < 20; ++step)
        {
            simulation.step(dt);
        }
        REQUIRE(simulation.restore_checkpoint(checkpoint), "valid checkpoint was rejected");
        REQUIRE(simulation.state_digest() == digest, "checkpoint restore changed visible runtime state");
        REQUIRE(physics_sim::versioned_water_state_digest(simulation, dt) == versioned_digest, "checkpoint restore changed versioned runtime identity");
        REQUIRE(simulation.simulation_tick() == tick, "checkpoint restore lost the simulation tick");
        REQUIRE(nearly_equal(simulation.emitters().front().emission_accumulator, accumulator), "checkpoint restore lost emitter accumulation");
        REQUIRE(simulation.emitters().front().emitted_particles == phase, "checkpoint restore lost emitter phase");

        auto changed = checkpoint.simulation;
        changed.emitters().front().emission_accumulator += 0.25;
        REQUIRE(physics_sim::versioned_water_state_digest(changed, dt) != versioned_digest,
                "emitter fractional accumulation did not affect versioned identity");
        changed = checkpoint.simulation;
        ++changed.emitters().front().emitted_particles;
        REQUIRE(physics_sim::versioned_water_state_digest(changed, dt) != versioned_digest, "emitter deterministic phase did not affect versioned identity");
        changed = checkpoint.simulation;
        auto settings = changed.solver_settings();
        settings.gravity_acceleration += 1.0f;
        changed.set_solver_settings(settings);
        REQUIRE(physics_sim::versioned_water_state_digest(changed, dt) != versioned_digest, "solver configuration did not affect versioned identity");
        changed = checkpoint.simulation;
        changed.grid().u(2, 2) += 1.0f;
        REQUIRE(physics_sim::versioned_water_state_digest(changed, dt) != versioned_digest, "grid velocity did not affect versioned identity");
        changed = checkpoint.simulation;
        changed.particles().front().affine_velocity.m01 += 0.5f;
        REQUIRE(physics_sim::versioned_water_state_digest(changed, dt) != versioned_digest, "particle affine state did not affect versioned identity");
        REQUIRE(physics_sim::versioned_water_state_digest(checkpoint.simulation, dt, true, false) != versioned_digest,
                "pause state did not affect versioned identity");
        REQUIRE(physics_sim::versioned_water_state_digest(checkpoint.simulation, dt, false, true) != versioned_digest,
                "pending single-step state did not affect versioned identity");
        REQUIRE(physics_sim::versioned_water_state_digest(checkpoint.simulation, 1.0 / 60.0) != versioned_digest,
                "fixed timestep did not affect versioned identity");

        auto continued = simulation;
        auto repeated = expected;
        for (int step = 0; step < 24; ++step)
        {
            continued.step(dt);
            repeated.step(dt);
            REQUIRE(physics_sim::versioned_water_state_digest(continued, dt) == physics_sim::versioned_water_state_digest(repeated, dt),
                    "equal restored states diverged under equal future steps");
        }
        require_same_continuation_state(continued, repeated, "restored checkpoint did not continue deterministically");

        auto invalid = checkpoint;
        invalid.version = 99;
        const std::string before_invalid = physics_sim::versioned_water_state_digest(simulation, dt);
        REQUIRE(!simulation.restore_checkpoint(invalid), "unsupported checkpoint version was accepted");
        REQUIRE(physics_sim::versioned_water_state_digest(simulation, dt) == before_invalid, "failed checkpoint restore mutated the simulation");
        REQUIRE(checkpoint.estimated_bytes() >= sizeof(checkpoint), "checkpoint byte estimate is smaller than its object");
    }

    {
        physics_sim::WaterSimulation2D simulation{12, 12, 1.0f};
        add_test_basin(simulation);
        physics_sim::SceneController controller{simulation};
        for (int step = 0; step < 7; ++step)
        {
            simulation.step(dt);
        }

        const physics_sim::WaterSimulationCheckpoint before_edit = simulation.capture_checkpoint();
        controller.set_tool(physics_sim::SceneTool::PaintWall);
        controller.begin_stroke({4.2f, 5.2f});
        controller.end_stroke({4.2f, 5.2f});
        const physics_sim::WaterSimulationCheckpoint after_edit = simulation.capture_checkpoint();
        REQUIRE(simulation.grid().solid(4, 5), "wall edit was not applied");

        REQUIRE(controller.undo_scene_edit(), "runtime checkpoint undo failed");
        require_same_continuation_state(simulation, before_edit.simulation, "undo did not restore the active runtime");
        REQUIRE(!simulation.grid().solid(4, 5), "undo retained the edited wall");

        REQUIRE(controller.redo_scene_edit(), "runtime checkpoint redo failed");
        require_same_continuation_state(simulation, after_edit.simulation, "redo did not restore the post-edit runtime");
        REQUIRE(simulation.grid().solid(4, 5), "redo lost the edited wall");

        REQUIRE(controller.undo_scene_edit(), "second undo failed");
        auto expected = before_edit.simulation;
        for (int step = 0; step < 16; ++step)
        {
            simulation.step(dt);
            expected.step(dt);
        }
        require_same_continuation_state(simulation, expected, "undo continuation diverged from the original runtime");

        controller.set_tool(physics_sim::SceneTool::PaintWall);
        controller.begin_stroke({6.2f, 5.2f});
        controller.end_stroke({6.2f, 5.2f});
        REQUIRE(!controller.redo_scene_edit(), "branch-after-undo retained redo history");
    }

    {
        physics_sim::WaterSimulation2D simulation{8, 8, 1.0f};
        physics_sim::SceneController controller{simulation};
        for (int edit = 0; edit < 80; ++edit)
        {
            controller.set_tool((edit % 2) == 0 ? physics_sim::SceneTool::PaintWall : physics_sim::SceneTool::EraseWall);
            controller.begin_stroke({3.2f, 3.2f});
            controller.end_stroke({3.2f, 3.2f});
        }
        REQUIRE(controller.history_size() <= physics_sim::SceneController::maximum_history_entries(), "history entry limit was exceeded");
        REQUIRE(controller.history_size() > 1, "history eviction removed the current history");
        REQUIRE(controller.history_bytes() <= physics_sim::SceneController::maximum_history_bytes(), "history byte limit was exceeded");
        REQUIRE(controller.history_index() + 1 == controller.history_size(), "history index did not follow deterministic eviction");
    }

    {
        physics_sim::SceneDocument scene;
        scene.grid_width = 8;
        scene.grid_height = 8;
        scene.cell_size = 1.0f;
        scene.solver_profile = physics_sim::FluidSolverProfile::Balanced;
        scene.metadata.title = "Digest scene";
        scene.solid_cells.push_back({2, 3});
        scene.sensors.push_back({1, 1, 2, 2, true, false, true, "goal"});
        const std::string scene_digest = physics_sim::versioned_scene_definition_digest(scene);

        auto active_only = scene;
        active_only.sensors.front().active = true;
        REQUIRE(physics_sim::versioned_scene_definition_digest(active_only) == scene_digest, "derived sensor activity changed authored scene identity");
        auto changed_scene = scene;
        changed_scene.solid_cells.push_back({3, 3});
        REQUIRE(physics_sim::versioned_scene_definition_digest(changed_scene) != scene_digest, "authored wall change did not affect scene identity");
        changed_scene = scene;
        changed_scene.metadata.title = "Different title";
        REQUIRE(physics_sim::versioned_scene_definition_digest(changed_scene) != scene_digest, "authored metadata change did not affect scene identity");

        physics_sim::WaterSimulation2D runtime{8, 8, 1.0f};
        REQUIRE(physics_sim::versioned_water_state_digest(runtime, dt) != scene_digest, "scene and runtime digest domains collided for the test state");
    }

    std::puts("runtime checkpoint and versioned digest tests passed");
    return 0;
}
