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
            "physics-sim-replay 1\n"
            "# comment\n"
            "tick 20 action reset\n"
            "tick 10 tool directional\n"
            "tick 30 place 640 160\n");

        assert(script.has_value());
        assert(script->events.size() == 3);
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
    }

    {
        const auto invalid_version = physics_sim::parse_replay_script_text(
            "physics-sim-replay 2\n"
            "tick 1 reset\n");
        assert(!invalid_version.has_value());
    }

    {
        const auto invalid_action = physics_sim::parse_replay_script_text(
            "physics-sim-replay 1\n"
            "tick 1 tool\n");
        assert(!invalid_action.has_value());
    }

    return 0;
}
