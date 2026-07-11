#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <physics_sim/replay_script.hpp>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

int main()
{
    {
        const auto script = physics_sim::parse_replay_script_text(
            "physics-sim-replay 2\n"
            "scene-digest 0123456789ABCDEF\n"
            "fixed-timestep 0.008333333333333333\n"
            "solver-profile balanced\n"
            "# comment\n"
            "tick 20 action reset\n"
            "tick 10 tool directional\n"
            "tick 30 place 640 160\n"
            "tick 1200 clear-emitters\n");

        assert(script.has_value());
        assert(script->scene_digest == "0123456789ABCDEF");
        assert(script->solver_profile == physics_sim::FluidSolverProfile::Balanced);
        assert(physics_sim::replay_identity_matches(
            *script,
            "0123456789ABCDEF",
            1.0 / 120.0,
            physics_sim::FluidSolverProfile::Balanced));
        assert(!physics_sim::replay_identity_matches(
            *script,
            "FFFFFFFFFFFFFFFF",
            1.0 / 120.0,
            physics_sim::FluidSolverProfile::Balanced));
        assert(script->events.size() == 4);
        assert(script->events[0].tick == 10);
        assert(script->events[0].command == "tool");
        assert(script->events[0].arguments.size() == 1);
        assert(script->events[0].arguments[0] == "directional");
        assert(script->events[1].tick == 20);
        assert(script->events[1].command == "action");
        assert(script->events[1].arguments.size() == 1);
        assert(script->events[1].arguments[0] == "reset");
        assert(script->events[2].tick == 30);
        assert(script->events[2].command == "place");
        assert(script->events[2].arguments.size() == 2);
        assert(script->events[2].arguments[0] == "640");
        assert(script->events[2].arguments[1] == "160");
        assert(script->events[3].tick == 1200);
        assert(script->events[3].command == "clear-emitters");
        assert(script->events[3].arguments.empty());
    }

    {
        const auto invalid_version = physics_sim::parse_replay_script_text(
            "physics-sim-replay 1\n"
            "tick 1 reset\n");
        assert(!invalid_version.has_value());
    }

    {
        const auto invalid_action = physics_sim::parse_replay_script_text(
            "physics-sim-replay 2\n"
            "scene-digest 0123456789ABCDEF\n"
            "fixed-timestep 0.008333333333333333\n"
            "solver-profile balanced\n"
            "tick 1 tool\n");
        assert(!invalid_action.has_value());
    }

    {
        const auto missing_identity = physics_sim::parse_replay_script_text(
            "physics-sim-replay 2\n"
            "tick 1 action reset\n");
        assert(!missing_identity.has_value());
    }

    assert(physics_sim::stable_replay_source_digest("abc") == "E71FA2190541574B");
    assert(physics_sim::stable_replay_source_digest("a\r\nb\r\n") == physics_sim::stable_replay_source_digest("a\nb\n"));

    return 0;
}
