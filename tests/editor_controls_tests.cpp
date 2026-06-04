#include <physics_sim/math.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/scene_viewport.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>

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
    {
        physics_sim::SceneViewport viewport;
        viewport.set_world_size(physics_sim::Vec2{1280.0f, 720.0f});
        viewport.set_window_size(1280, 720);

        const physics_sim::Vec2 center_world = viewport.window_to_world(physics_sim::Vec2{640.0f, 360.0f});
        REQUIRE(nearly_equal(center_world.x, 640.0f), "viewport did not map screen center to world center");
        REQUIRE(nearly_equal(center_world.y, 360.0f), "viewport did not map screen center to world center");

        viewport.pan_pixels(physics_sim::Vec2{64.0f, -32.0f});
        const physics_sim::Vec2 panned_world = viewport.window_to_world(physics_sim::Vec2{640.0f, 360.0f});
        REQUIRE(nearly_equal(panned_world.x, 576.0f), "viewport pan did not shift x world position");
        REQUIRE(nearly_equal(panned_world.y, 392.0f), "viewport pan did not shift y world position");

        viewport.zoom_at(2.0f, physics_sim::Vec2{640.0f, 360.0f});
        REQUIRE(nearly_equal(viewport.window_to_world(physics_sim::Vec2{640.0f, 360.0f}).x, panned_world.x), "zoom_at moved the anchor point");
        REQUIRE(nearly_equal(viewport.window_to_world(physics_sim::Vec2{640.0f, 360.0f}).y, panned_world.y), "zoom_at moved the anchor point");
        REQUIRE(nearly_equal(viewport.scale(), 2.0f), "viewport zoom did not update scale");
    }

    {
        physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
        physics_sim::SceneController controller{simulation};

        controller.set_tool(physics_sim::SceneTool::PaintWall);
        controller.begin_stroke(physics_sim::Vec2{32.0f, 80.0f});
        controller.drag_stroke(physics_sim::Vec2{111.0f, 80.0f});
        controller.end_stroke(physics_sim::Vec2{111.0f, 80.0f});

        REQUIRE(simulation.grid().solid(2, 5), "wall stroke did not start at the expected cell");
        REQUIRE(simulation.grid().solid(3, 5), "wall stroke skipped a cell");
        REQUIRE(simulation.grid().solid(4, 5), "wall stroke skipped a cell");
        REQUIRE(simulation.grid().solid(5, 5), "wall stroke skipped a cell");
        REQUIRE(simulation.grid().solid(6, 5), "wall stroke skipped a cell");
        REQUIRE(!simulation.grid().solid(7, 5), "wall stroke extended past the expected end cell");

        controller.set_tool(physics_sim::SceneTool::EraseWall);
        controller.begin_stroke(physics_sim::Vec2{48.0f, 80.0f});
        controller.drag_stroke(physics_sim::Vec2{95.0f, 80.0f});
        controller.end_stroke(physics_sim::Vec2{95.0f, 80.0f});

        REQUIRE(simulation.grid().solid(2, 5), "erase stroke removed unrelated wall cells");
        REQUIRE(!simulation.grid().solid(3, 5), "erase stroke did not remove the target cell");
        REQUIRE(!simulation.grid().solid(4, 5), "erase stroke did not remove the target cell");
        REQUIRE(!simulation.grid().solid(5, 5), "erase stroke did not remove the target cell");
        REQUIRE(simulation.grid().solid(6, 5), "erase stroke removed unrelated wall cells");
    }

    {
        physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
        physics_sim::SceneController controller{simulation};
        controller.set_emission_rate(24.0f);
        controller.set_emitter_speed(8.0f);
        controller.set_emitter_direction(physics_sim::Vec2{1.0f, 0.0f});

        controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
        controller.place_fixture(physics_sim::Vec2{80.0f, 96.0f});

        REQUIRE(simulation.emitters().size() == 1, "directional fixture did not create an emitter");
        REQUIRE(simulation.emitters().front().kind == physics_sim::WaterEmitterKind::Directional, "directional fixture kind incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().direction.x, 1.0f), "directional emitter direction x incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().direction.y, 0.0f), "directional emitter direction y incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().speed, 8.0f), "directional emitter speed incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().emission_rate, 24.0f), "directional emitter rate incorrect");

        controller.set_tool(physics_sim::SceneTool::OmniEmitter);
        controller.place_fixture(physics_sim::Vec2{240.0f, 96.0f});

        REQUIRE(simulation.emitters().size() == 2, "omni fixture did not create an emitter");
        REQUIRE(simulation.emitters().back().kind == physics_sim::WaterEmitterKind::Omni, "omni fixture kind incorrect");

        controller.reset_scene();
        REQUIRE(simulation.emitters().empty(), "reset_scene did not clear emitters");
        REQUIRE(!simulation.grid().solid(5, 5), "reset_scene did not clear walls");
    }

    return 0;
}
