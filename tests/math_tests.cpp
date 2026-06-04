#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <physics_sim/action.hpp>
#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/indexing.hpp>
#include <physics_sim/math.hpp>
#include <physics_sim/water_state.hpp>
#include <physics_sim/simulation_state.hpp>

#include <cassert>
#include <cmath>
#include <cstddef>

namespace
{
bool nearly_equal(double lhs, double rhs, double epsilon = 0.0000001) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}
} // namespace

int main()
{
    using physics_sim::Vec2;

    {
        const Vec2 a{1.0f, 2.0f};
        const Vec2 b{3.0f, 4.0f};
        const Vec2 sum = a + b;
        const Vec2 diff = b - a;
        const Vec2 scaled = a * 2.0f;
        const Vec2 unit = physics_sim::normalize(Vec2{3.0f, 4.0f});

        assert(nearly_equal(sum.x, 4.0));
        assert(nearly_equal(sum.y, 6.0));
        assert(nearly_equal(diff.x, 2.0));
        assert(nearly_equal(diff.y, 2.0));
        assert(nearly_equal(scaled.x, 2.0));
        assert(nearly_equal(scaled.y, 4.0));
        assert(nearly_equal(physics_sim::length(Vec2{3.0f, 4.0f}), 5.0));
        assert(nearly_equal(unit.x, 0.6));
        assert(nearly_equal(unit.y, 0.8));
        assert(physics_sim::clamp(5, 0, 4) == 4);
        assert(physics_sim::clamp(-1, 0, 4) == 0);
    }

    {
        assert(physics_sim::flatten_index(2, 1, 5) == 7);
        assert(physics_sim::in_bounds(0, 0, 3, 4));
        assert(!physics_sim::in_bounds(3, 0, 3, 4));
        assert(physics_sim::safe_flatten_index(2, 1, 5, 5) == 7);
        assert(physics_sim::safe_flatten_index(5, 1, 5, 5) == physics_sim::index_npos);
    }

    {
        physics_sim::FixedStepDriver driver{physics_sim::FixedStepDriver::duration{0.1}, 3};
        std::size_t stepCount = 0;

        auto result = driver.advance(physics_sim::FixedStepDriver::duration{0.25}, [&](physics_sim::FixedStepDriver::duration step)
        {
            assert(nearly_equal(step.count(), 0.1));
            ++stepCount;
        });

        assert(result.steps == 2);
        assert(!result.clamped);
        assert(stepCount == 2);
        assert(nearly_equal(driver.accumulator().count(), 0.05));

        result = driver.advance(physics_sim::FixedStepDriver::duration{1.0}, [&](physics_sim::FixedStepDriver::duration)
        {
            ++stepCount;
        });

        assert(result.steps == 3);
        assert(result.clamped);
        assert(stepCount == 5);
        assert(nearly_equal(driver.accumulator().count(), 0.0));

        driver.reset();
        assert(nearly_equal(driver.accumulator().count(), 0.0));
        assert(!driver.paused());
        assert(!driver.step_requested());

        driver.set_paused(true);
        stepCount = 0;
        result = driver.advance(physics_sim::FixedStepDriver::duration{1.0}, [&](physics_sim::FixedStepDriver::duration)
        {
            ++stepCount;
        });

        assert(result.steps == 0);
        assert(stepCount == 0);
        assert(nearly_equal(driver.accumulator().count(), 0.0));

        driver.request_step();
        result = driver.advance(physics_sim::FixedStepDriver::duration{0.016}, [&](physics_sim::FixedStepDriver::duration step)
        {
            assert(nearly_equal(step.count(), 0.1));
            ++stepCount;
        });

        assert(result.steps == 1);
        assert(stepCount == 1);
        assert(driver.paused());
        assert(!driver.step_requested());
    }

    {
        physics_sim::SimulationState state;
        state.advance(physics_sim::SimulationState::duration{0.1});
        state.advance(physics_sim::SimulationState::duration{0.1});
        assert(state.tick_count == 2);
        assert(nearly_equal(state.simulated_time.count(), 0.2));
        state.reset();
        assert(state.tick_count == 0);
        assert(nearly_equal(state.simulated_time.count(), 0.0));
    }

    {
        physics_sim::MacGrid2D grid;
        grid.resize(4, 3, 0.5f);
        assert(grid.valid());
        assert(grid.width() == 4);
        assert(grid.height() == 3);
        assert(nearly_equal(grid.cell_size(), 0.5));
        assert(grid.cell_count() == 12);
        assert(grid.u_count() == 15);
        assert(grid.v_count() == 16);

        grid.pressure(1, 2) = 4.5f;
        grid.divergence(0, 0) = 1.25f;
        grid.velocity_u(2, 1) = 3.0f;
        grid.velocity_v(1, 2) = 2.0f;
        grid.set_solid(3, 1, true);

        assert(grid.solid(3, 1));
        grid.clear_fields();
        assert(nearly_equal(grid.pressure(1, 2), 0.0));
        assert(nearly_equal(grid.divergence(0, 0), 0.0));
        assert(nearly_equal(grid.velocity_u(2, 1), 0.0));
        assert(nearly_equal(grid.velocity_v(1, 2), 0.0));
        assert(grid.solid(3, 1));

        grid.clear_solids();
        assert(!grid.solid(3, 1));
    }

    {
        physics_sim::WaterState water;
        water.grid.resize(2, 2, 1.0f);
        water.grid.set_solid(1, 1, true);
        water.particles.push_back({ physics_sim::Vec2{1.0f, 2.0f}, physics_sim::Vec2{3.0f, 4.0f} });

        water.reset_fluid();
        assert(water.particles.empty());
        assert(nearly_equal(water.grid.pressure(0, 0), 0.0));
        assert(water.grid.solid(1, 1));

        water.reset_scene();
        assert(!water.grid.solid(1, 1));
    }

    {
        assert(physics_sim::action_from_keycode(SDLK_SPACE).has_value());
        assert(*physics_sim::action_from_keycode(SDLK_SPACE) == physics_sim::Action::TogglePause);
        assert(*physics_sim::action_from_keycode(SDLK_s) == physics_sim::Action::StepOnce);
        assert(*physics_sim::action_from_keycode(SDLK_r) == physics_sim::Action::Reset);
        assert(*physics_sim::action_from_keycode(SDLK_ESCAPE) == physics_sim::Action::Quit);
        assert(!physics_sim::action_from_keycode(SDLK_a).has_value());
    }

    return 0;
}
