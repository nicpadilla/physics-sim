#include <physics_sim/math.hpp>
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
} // namespace

int main()
{
    physics_sim::WaterSimulation2D sim{24, 24, 1.0f};

    sim.paint_wall_line(physics_sim::Vec2{2.2f, 5.0f}, physics_sim::Vec2{7.8f, 5.0f}, true);
    for (std::size_t x = 2; x <= 7; ++x)
    {
        REQUIRE(sim.grid().solid(x, 5), "paint_wall_line did not set every cell on the stroke");
    }

    sim.paint_wall_line(physics_sim::Vec2{4.0f, 5.0f}, physics_sim::Vec2{6.0f, 5.0f}, false);
    REQUIRE(!sim.grid().solid(4, 5), "paint_wall_line failed to erase a cell");
    REQUIRE(!sim.grid().solid(5, 5), "paint_wall_line failed to erase a cell");
    REQUIRE(!sim.grid().solid(6, 5), "paint_wall_line failed to erase a cell");
    REQUIRE(sim.grid().solid(2, 5), "paint_wall_line erased unrelated cells");
    REQUIRE(sim.grid().solid(3, 5), "paint_wall_line erased unrelated cells");
    REQUIRE(sim.grid().solid(7, 5), "paint_wall_line erased unrelated cells");

    sim.clear_scene();
    REQUIRE(!sim.grid().solid(2, 5), "clear_scene did not clear wall cells");
    REQUIRE(sim.emitters().empty(), "clear_scene did not clear emitters");

    return 0;
}
