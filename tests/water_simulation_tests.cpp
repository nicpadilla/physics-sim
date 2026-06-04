#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/math.hpp>

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

// This test file intentionally compiles against the desired water-simulation API
// before the implementation exists. The first build should fail here.
#include <physics_sim/water_simulation.hpp>

namespace
{
bool nearly_equal(double lhs, double rhs, double epsilon = 0.0005) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}

[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}
} // namespace

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)

int main()
{
    using physics_sim::Vec2;

    {
        physics_sim::WaterSimulation2D sim{24, 24, 1.0f};
        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = Vec2{12.0f, 4.0f};
        emitter.direction = Vec2{0.0f, 1.0f};
        emitter.speed = 6.0f;
        emitter.emission_rate = 32.0f;
        sim.add_emitter(emitter);

        sim.step(0.5);

        REQUIRE(!sim.particles().empty(), "emitter did not spawn particles");
        REQUIRE(sim.particles().front().velocity.y > 0.0f, "emitted particles did not move downward");
        REQUIRE(sim.metrics().total_emitted > 0, "emitter count did not increment");
    }

    {
        physics_sim::WaterSimulation2D sim{16, 16, 1.0f};
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, 0, true);
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        sim.particles().push_back({Vec2{8.0f, 8.0f}, Vec2{0.0f, -3.0f}});
        sim.step(0.25);

        for (const auto& particle : sim.particles())
        {
            const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x));
            const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y));
            REQUIRE(!sim.grid().solid(cell_x, cell_y), "particle penetrated a solid cell");
        }
    }

    {
        physics_sim::WaterSimulation2D sim{18, 18, 1.0f};
        for (std::size_t x = 0; x < sim.grid().width(); ++x)
        {
            sim.set_solid_cell(x, 0, true);
            sim.set_solid_cell(x, sim.grid().height() - 1, true);
        }
        for (std::size_t y = 0; y < sim.grid().height(); ++y)
        {
            sim.set_solid_cell(0, y, true);
            sim.set_solid_cell(sim.grid().width() - 1, y, true);
        }

        sim.particles().push_back({Vec2{9.0f, 9.0f}, Vec2{1.0f, 1.0f}});
        sim.particles().push_back({Vec2{10.0f, 9.0f}, Vec2{-1.0f, 1.0f}});
        sim.step(0.1);

        REQUIRE(sim.metrics().average_divergence_after_projection < 0.15, "average divergence too high");
        REQUIRE(sim.metrics().max_divergence_after_projection < 0.5, "max divergence too high");
    }

    {
        physics_sim::WaterSimulation2D sim{20, 20, 1.0f};
        physics_sim::WaterEmitter emitter;
        emitter.kind = physics_sim::WaterEmitterKind::Omni;
        emitter.position = Vec2{10.0f, 10.0f};
        emitter.speed = 5.0f;
        emitter.emission_rate = 48.0f;
        sim.add_emitter(emitter);

        sim.step(0.5);

        REQUIRE(sim.particles().size() >= 8, "omni emitter did not spawn enough particles");

        bool has_left = false;
        bool has_right = false;
        for (const auto& particle : sim.particles())
        {
            has_left = has_left || particle.velocity.x < -0.25f;
            has_right = has_right || particle.velocity.x > 0.25f;
        }

        REQUIRE(has_left, "omni emitter never emitted a leftward particle");
        REQUIRE(has_right, "omni emitter never emitted a rightward particle");
    }

    {
        physics_sim::WaterSimulation2D sim{10, 10, 1.0f};
        sim.set_solid_cell(3, 3, true);
        sim.particles().push_back({Vec2{4.0f, 4.0f}, Vec2{2.0f, 2.0f}});
        sim.clear_scene();

        REQUIRE(sim.particles().empty(), "clear_scene did not remove particles");
        REQUIRE(sim.grid().valid(), "grid became invalid after clear_scene");
        REQUIRE(!sim.grid().solid(3, 3), "clear_scene did not clear solids");
        REQUIRE(nearly_equal(sim.metrics().average_divergence_after_projection, 0.0), "metrics were not reset");
    }

    return 0;
}
