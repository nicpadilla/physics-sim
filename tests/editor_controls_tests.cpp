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
        REQUIRE(controller.undo_scene_edit(), "undo_scene_edit did not restore the erased wall");
        REQUIRE(simulation.grid().solid(3, 5), "undo_scene_edit did not restore a wall cell");
        REQUIRE(simulation.grid().solid(4, 5), "undo_scene_edit did not restore a wall cell");
        REQUIRE(simulation.grid().solid(5, 5), "undo_scene_edit did not restore a wall cell");
        REQUIRE(controller.redo_scene_edit(), "redo_scene_edit did not reapply the wall erase");
        REQUIRE(!simulation.grid().solid(3, 5), "redo_scene_edit did not reapply the erase");
        REQUIRE(!simulation.grid().solid(4, 5), "redo_scene_edit did not reapply the erase");
        REQUIRE(!simulation.grid().solid(5, 5), "redo_scene_edit did not reapply the erase");
    }

    {
        physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
        physics_sim::SceneController controller{simulation};
        controller.set_emission_rate(24.0f);
        controller.set_emitter_speed(8.0f);
        controller.set_emitter_direction(physics_sim::Vec2{1.0f, 0.0f});

        controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
        REQUIRE(controller.place_fixture(physics_sim::Vec2{80.0f, 96.0f}), "directional fixture placement was rejected");

        REQUIRE(simulation.emitters().size() == 1, "directional fixture did not create an emitter");
        REQUIRE(simulation.emitters().front().kind == physics_sim::WaterEmitterKind::Directional, "directional fixture kind incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().direction.x, 1.0f), "directional emitter direction x incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().direction.y, 0.0f), "directional emitter direction y incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().speed, 8.0f), "directional emitter speed incorrect");
        REQUIRE(nearly_equal(simulation.emitters().front().emission_rate, 24.0f), "directional emitter rate incorrect");
        REQUIRE(controller.has_selected_fixture(), "placing a fixture did not select it");
        REQUIRE(controller.selected_fixture() != nullptr, "selected fixture pointer was null after placement");
        REQUIRE(controller.selected_fixture()->kind == physics_sim::WaterEmitterKind::Directional, "selected fixture kind incorrect");

        simulation.set_solid_cell(10, 8, true);
        REQUIRE(!controller.place_fixture(physics_sim::Vec2{160.0f, 128.0f}), "invalid placement was not rejected");
        REQUIRE(simulation.emitters().size() == 1, "invalid placement added an emitter");

        controller.set_tool(physics_sim::SceneTool::OmniEmitter);
        REQUIRE(controller.place_fixture(physics_sim::Vec2{240.0f, 96.0f}), "omni fixture placement was rejected");

        REQUIRE(simulation.emitters().size() == 2, "omni fixture did not create an emitter");
        REQUIRE(simulation.emitters().back().kind == physics_sim::WaterEmitterKind::Omni, "omni fixture kind incorrect");
        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{80.0f, 96.0f}, 20.0f), "fixture selection missed the first emitter");
        REQUIRE(controller.selected_fixture() != nullptr, "selected fixture pointer was null after hit test");
        REQUIRE(controller.selected_fixture()->kind == physics_sim::WaterEmitterKind::Directional, "hit test selected the wrong fixture");

        REQUIRE(controller.delete_selected_fixture(), "delete_selected_fixture did not remove the selected emitter");
        REQUIRE(simulation.emitters().size() == 1, "delete_selected_fixture removed the wrong number of emitters");
        REQUIRE(simulation.emitters().front().kind == physics_sim::WaterEmitterKind::Omni, "delete_selected_fixture removed the wrong emitter");
        REQUIRE(controller.undo_scene_edit(), "undo_scene_edit did not restore a deleted emitter");
        REQUIRE(simulation.emitters().size() == 2, "undo_scene_edit did not restore the deleted emitter");
        REQUIRE(controller.redo_scene_edit(), "redo_scene_edit did not reapply the deletion");
        REQUIRE(simulation.emitters().size() == 1, "redo_scene_edit did not remove the emitter again");

        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{240.0f, 96.0f}, 20.0f), "reselecting the remaining emitter failed");
        REQUIRE(controller.selected_fixture() != nullptr, "reselecting the remaining emitter failed");
        const physics_sim::Vec2 original_position = controller.selected_fixture()->position;
        REQUIRE(controller.move_selected_fixture(physics_sim::Vec2{16.0f, -16.0f}), "move_selected_fixture failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->position.x, original_position.x + 16.0f), "move_selected_fixture did not update x");
        REQUIRE(nearly_equal(controller.selected_fixture()->position.y, original_position.y - 16.0f), "move_selected_fixture did not update y");
        REQUIRE(controller.undo_scene_edit(), "undo_scene_edit did not revert the move");
        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{240.0f, 96.0f}, 20.0f), "reselecting after undo failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->position.x, original_position.x), "undo_scene_edit did not restore x");
        REQUIRE(nearly_equal(controller.selected_fixture()->position.y, original_position.y), "undo_scene_edit did not restore y");
        REQUIRE(controller.redo_scene_edit(), "redo_scene_edit did not reapply the move");
        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{256.0f, 80.0f}, 20.0f), "reselecting after redo failed");
        REQUIRE(controller.rotate_selected_fixture(3.1415926f * 0.5f), "rotate_selected_fixture failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->direction.x, 0.0f, 0.0002f), "rotate_selected_fixture did not rotate x");
        REQUIRE(nearly_equal(controller.selected_fixture()->direction.y, 1.0f, 0.0002f), "rotate_selected_fixture did not rotate y");
        REQUIRE(controller.set_selected_fixture_speed(10.0f), "set_selected_fixture_speed failed");
        REQUIRE(controller.adjust_selected_fixture_emission_rate(6.0f), "adjust_selected_fixture_emission_rate failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->speed, 10.0f), "selected fixture speed incorrect");
        REQUIRE(nearly_equal(controller.selected_fixture()->emission_rate, 30.0f), "selected fixture emission rate incorrect");
        REQUIRE(controller.undo_scene_edit(), "undo_scene_edit did not revert the emission-rate edit");
        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{256.0f, 80.0f}, 20.0f), "reselecting after parameter undo failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->emission_rate, 24.0f), "undo_scene_edit did not restore the emission rate");
        REQUIRE(controller.redo_scene_edit(), "redo_scene_edit did not reapply the emission-rate edit");
        REQUIRE(controller.select_fixture_at(physics_sim::Vec2{256.0f, 80.0f}, 20.0f), "reselecting after parameter redo failed");
        REQUIRE(nearly_equal(controller.selected_fixture()->emission_rate, 30.0f), "redo_scene_edit did not restore the emission rate");
        REQUIRE(controller.toggle_selected_fixture_enabled(), "toggle_selected_fixture_enabled failed");
        REQUIRE(!controller.selected_fixture()->enabled, "selected fixture enabled state did not toggle");

        controller.set_tool(physics_sim::SceneTool::Gate);
        REQUIRE(controller.place_gate(physics_sim::Vec2{400.0f, 160.0f}), "gate placement was rejected");
        REQUIRE(simulation.gates().size() == 1, "gate placement did not create a gate");
        REQUIRE(controller.has_selected_gate(), "placing a gate did not select it");
        REQUIRE(controller.selected_gate() != nullptr, "selected gate pointer was null after placement");
        REQUIRE(simulation.grid().solid(25, 10), "closed gate did not block the grid cell");
        REQUIRE(controller.toggle_selected_gate_open(), "toggle_selected_gate_open failed");
        REQUIRE(!simulation.grid().solid(25, 10), "open gate remained solid");
        REQUIRE(controller.toggle_selected_gate_open(), "toggle_selected_gate_open did not re-close the gate");
        REQUIRE(simulation.grid().solid(25, 10), "closed gate did not restore solidity");
        REQUIRE(controller.delete_selected_gate(), "delete_selected_gate failed");
        REQUIRE(simulation.gates().empty(), "delete_selected_gate did not remove the gate");
        REQUIRE(controller.undo_scene_edit(), "undo_scene_edit did not restore the deleted gate");
        REQUIRE(simulation.gates().size() == 1, "undo_scene_edit did not restore the gate");

        controller.set_tool(physics_sim::SceneTool::Sensor);
        REQUIRE(controller.place_sensor(physics_sim::Vec2{528.0f, 160.0f}), "sensor placement was rejected");
        REQUIRE(simulation.sensors().size() == 1, "sensor placement did not create a sensor");
        REQUIRE(controller.has_selected_sensor(), "placing a sensor did not select it");
        REQUIRE(controller.selected_sensor() != nullptr, "selected sensor pointer was null after placement");
        REQUIRE(!simulation.sensors().front().active, "sensor should start inactive");
        simulation.add_particle({physics_sim::Vec2{536.0f, 168.0f}, physics_sim::Vec2{0.0f, 0.0f}});
        simulation.step(0.016);
        REQUIRE(simulation.sensors().front().active, "sensor did not become active when water entered");
        REQUIRE(!controller.place_sensor(physics_sim::Vec2{528.0f, 160.0f}), "overlapping sensor placement was not rejected");

        controller.set_tool(physics_sim::SceneTool::Drain);
        REQUIRE(controller.place_drain({160.0f, 320.0f}), "drain placement failed");
        REQUIRE(controller.has_selected_drain(), "placed drain was not selected");
        REQUIRE(controller.toggle_selected_drain_enabled(), "drain toggle failed");
        REQUIRE(!simulation.drains().front().enabled, "drain toggle did not change state");
        REQUIRE(controller.select_drain_at({168.0f, 328.0f}), "drain selection failed");
        REQUIRE(controller.delete_selected_drain(), "drain deletion failed");
        REQUIRE(simulation.drains().empty(), "drain deletion left the device");
        REQUIRE(controller.undo_scene_edit(), "drain deletion undo failed");
        REQUIRE(simulation.drains().size() == 1, "drain undo did not restore device");

        controller.set_tool(physics_sim::SceneTool::Pump);
        REQUIRE(controller.place_pump({320.0f, 320.0f}), "pump placement failed");
        REQUIRE(controller.has_selected_pump(), "placed pump was not selected");
        const float initial_strength = simulation.pumps().front().strength;
        REQUIRE(controller.adjust_selected_pump_strength(2.0f), "pump strength edit failed");
        REQUIRE(nearly_equal(simulation.pumps().front().strength, initial_strength + 2.0f), "pump strength did not change");
        REQUIRE(controller.rotate_selected_pump(3.1415926f * 0.5f), "pump rotation failed");
        REQUIRE(controller.toggle_selected_pump_enabled(), "pump toggle failed");
        REQUIRE(!simulation.pumps().front().enabled, "pump toggle did not change state");
        REQUIRE(controller.select_pump_at({328.0f, 328.0f}), "pump selection failed");
        REQUIRE(controller.delete_selected_pump(), "pump deletion failed");
        REQUIRE(simulation.pumps().empty(), "pump deletion left the device");

        controller.reset_scene();
        REQUIRE(simulation.emitters().empty(), "reset_scene did not clear emitters");
        REQUIRE(simulation.gates().empty(), "reset_scene did not clear gates");
        REQUIRE(simulation.sensors().empty(), "reset_scene did not clear sensors");
        REQUIRE(!simulation.grid().solid(5, 5), "reset_scene did not clear walls");
    }

    return 0;
}
