#include <physics_sim/math.hpp>
#include <physics_sim/water_simulation.hpp>

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
} // namespace

int main()
{
    {
        physics_sim::WaterSimulation2D simulation{24, 24, 1.0f};
        simulation.add_gate(physics_sim::WaterGate{10, 10, false});
        REQUIRE(simulation.grid().solid(10, 10), "closed gate did not mark the grid solid");
        REQUIRE(simulation.toggle_gate_open(0), "toggle_gate_open failed");
        REQUIRE(!simulation.grid().solid(10, 10), "open gate remained solid");
        REQUIRE(simulation.toggle_gate_open(0), "toggle_gate_open failed to reclose the gate");
        REQUIRE(simulation.grid().solid(10, 10), "closed gate did not restore solidity");
    }

    {
        physics_sim::WaterSimulation2D simulation{24, 24, 1.0f};
        simulation.add_sensor(physics_sim::WaterSensor{5, 5, 3, 3, true, false, true, {}});
        simulation.add_particle({physics_sim::Vec2{5.5f, 5.5f}, physics_sim::Vec2{0.0f, 0.0f}});
        simulation.step(0.016);

        REQUIRE(!simulation.sensors().empty(), "sensor was not stored on the simulation");
        REQUIRE(simulation.sensors().front().active, "sensor did not become active");
        REQUIRE(simulation.metrics().active_sensors == 1, "active sensor count incorrect");
        REQUIRE(simulation.metrics().objective_sensors == 1, "objective sensor count incorrect");
        REQUIRE(simulation.metrics().objective_completed, "objective was not marked complete");
    }

    {
        physics_sim::WaterSimulation2D simulation{24, 24, 1.0f};
        simulation.add_drain(physics_sim::WaterDrain{5, 5, 3, 3, true});
        simulation.add_particle({physics_sim::Vec2{5.5f, 5.5f}, physics_sim::Vec2{0.0f, 0.0f}});
        simulation.add_particle({physics_sim::Vec2{15.5f, 15.5f}, physics_sim::Vec2{0.0f, 0.0f}});
        simulation.step(0.016);

        REQUIRE(simulation.particles().size() == 1, "drain did not remove water in its region");
        REQUIRE(simulation.metrics().total_removed == 1, "drain removal was not visible in metrics");
    }

    {
        physics_sim::WaterSimulation2D simulation{24, 24, 1.0f};
        simulation.add_pump(physics_sim::WaterPump{5, 5, 3, 3, true, physics_sim::Vec2{1.0f, 0.0f}, 12.0f});
        simulation.add_particle({physics_sim::Vec2{5.5f, 5.5f}, physics_sim::Vec2{0.0f, 0.0f}});
        simulation.step(0.016);

        REQUIRE(!simulation.particles().empty(), "pump removed the only particle");
        REQUIRE(simulation.particles().front().velocity.x > 0.0f, "pump did not accelerate water in the chosen direction");
    }

    {
        physics_sim::WaterSimulation2D simulation{24, 24, 1.0f};
        simulation.add_valve(physics_sim::WaterValve{10, 10, false});
        REQUIRE(simulation.grid().solid(10, 10), "closed valve did not mark the grid solid");
        REQUIRE(simulation.toggle_valve_open(0), "toggle_valve_open failed");
        REQUIRE(!simulation.grid().solid(10, 10), "open valve remained solid");
    }

    return 0;
}
