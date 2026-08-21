#include <physics_sim/scene_controller.hpp>
#include <physics_sim/water_simulation.hpp>

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
        const std::uint64_t tick = simulation.simulation_tick();
        const double accumulator = simulation.emitters().front().emission_accumulator;
        const std::uint64_t phase = simulation.emitters().front().emitted_particles;

        for (int step = 0; step < 20; ++step)
        {
            simulation.step(dt);
        }
        REQUIRE(simulation.restore_checkpoint(checkpoint), "valid checkpoint was rejected");
        REQUIRE(simulation.state_digest() == digest, "checkpoint restore changed visible runtime state");
        REQUIRE(simulation.simulation_tick() == tick, "checkpoint restore lost the simulation tick");
        REQUIRE(nearly_equal(simulation.emitters().front().emission_accumulator, accumulator), "checkpoint restore lost emitter accumulation");
        REQUIRE(simulation.emitters().front().emitted_particles == phase, "checkpoint restore lost emitter phase");

        auto continued = simulation;
        auto repeated = expected;
        for (int step = 0; step < 24; ++step)
        {
            continued.step(dt);
            repeated.step(dt);
        }
        require_same_continuation_state(continued, repeated, "restored checkpoint did not continue deterministically");

        auto invalid = checkpoint;
        invalid.version = 99;
        const std::string before_invalid = simulation.state_digest();
        REQUIRE(!simulation.restore_checkpoint(invalid), "unsupported checkpoint version was accepted");
        REQUIRE(simulation.state_digest() == before_invalid, "failed checkpoint restore mutated the simulation");
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

    std::puts("runtime checkpoint tests passed");
    return 0;
}
