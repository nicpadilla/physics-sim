#include <physics_sim/debug_overlay.hpp>
#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/simulation_state.hpp>
#include <physics_sim/visual_mode.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

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
} // namespace

int main()
{
    physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
    physics_sim::SceneController controller{simulation};
    physics_sim::FixedStepDriver driver{physics_sim::FixedStepDriver::duration{1.0 / 120.0}, 8};
    physics_sim::SimulationState state;

    state.tick_count = 42;
    state.simulated_time = physics_sim::SimulationState::duration{0.35};

    controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
    controller.place_fixture(physics_sim::Vec2{80.0f, 96.0f});

    const auto* selected = controller.selected_fixture();
    REQUIRE(selected != nullptr, "selected fixture pointer was null after placement");

    physics_sim::DebugOverlayMetrics metrics;
    metrics.fps = 144.0;
    metrics.driver = &driver;
    metrics.state = &state;
    metrics.simulation = &simulation;
    metrics.controller = &controller;
    metrics.visual_mode = physics_sim::visual_mode_name(physics_sim::VisualMode::Density);
    metrics.status_message = "RESET FLUID";

    const auto lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(lines[0].find("FPS ") == 0, "overlay line 0 did not report fps");
    REQUIRE(lines[1].find("STEP ") == 0, "overlay line 1 did not report step");
    REQUIRE(lines[4].find("REM 0") != std::string::npos, "overlay did not report removed particle metrics");
    REQUIRE(lines[8] == "MODE density", "overlay mode line incorrect");
    REQUIRE(lines[9] == "TOOL PIPE", "overlay tool line incorrect");
    REQUIRE(lines[10] == "OBJ NONE", "overlay objective line incorrect without sensors");
    REQUIRE(lines[11] == "SEL HOSE", "overlay selected fixture line incorrect");
    REQUIRE(lines[12].find("POS ") == 0, "overlay selection position line incorrect");
    REQUIRE(lines[13].find("SPD ") == 0, "overlay selection detail line incorrect");
    REQUIRE(lines[14] == "MSG RESET FLUID", "overlay feedback line incorrect");

    controller.clear_selection();
    metrics.status_message = nullptr;
    const auto cleared_lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(cleared_lines[10] == "OBJ NONE", "overlay objective line should remain empty when inactive");
    REQUIRE(cleared_lines[11] == "SEL NONE", "overlay did not reflect cleared selection");
    REQUIRE(cleared_lines[12].find("DEF DIR ") == 0, "overlay default direction line incorrect");
    REQUIRE(cleared_lines[13].find("DEF SPD ") == 0, "overlay default parameter line incorrect");
    REQUIRE(cleared_lines[14].empty(), "overlay feedback line should be empty when inactive");

    controller.set_tool(physics_sim::SceneTool::PointerWater);
    const auto pointer_lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(pointer_lines[9] == "TOOL POUR", "overlay tool line did not report pointer water");

    controller.set_tool(physics_sim::SceneTool::Gate);
    REQUIRE(controller.place_gate(physics_sim::Vec2{400.0f, 160.0f}), "gate placement was rejected in overlay test");
    const auto gate_lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(gate_lines[10] == "OBJ NONE", "overlay objective line incorrect for gate scene");
    REQUIRE(gate_lines[11] == "SEL GATE", "overlay did not report selected gate");
    REQUIRE(gate_lines[13] == "STATE CLOSED", "overlay did not report closed gate state");

    controller.set_tool(physics_sim::SceneTool::Sensor);
    REQUIRE(controller.place_sensor(physics_sim::Vec2{528.0f, 160.0f}), "sensor placement was rejected in overlay test");
    simulation.add_particle({physics_sim::Vec2{536.0f, 168.0f}, physics_sim::Vec2{0.0f, 0.0f}});
    simulation.step(0.016);
    const auto sensor_lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(sensor_lines[11] == "SEL SENSOR", "overlay did not report selected sensor");
    REQUIRE(sensor_lines[13].find("STATE ACTIVE") == 0, "overlay did not report active sensor state");

    controller.set_tool(physics_sim::SceneTool::Valve);
    REQUIRE(controller.place_valve(physics_sim::Vec2{592.0f, 160.0f}), "valve placement was rejected in overlay test");
    const auto valve_lines = physics_sim::build_debug_overlay_lines(metrics, 2.0f);
    REQUIRE(valve_lines[11] == "SEL VALVE", "overlay did not report selected valve");
    REQUIRE(valve_lines[13] == "STATE CLOSED", "overlay did not report closed valve state");

    return 0;
}
