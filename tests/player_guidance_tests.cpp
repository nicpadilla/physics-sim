#define SDL_MAIN_HANDLED

#include <physics_sim/input_bindings.hpp>
#include <physics_sim/player_guidance.hpp>
#include <physics_sim/tutorial_progress.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

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

[[nodiscard]] bool contains_line(const std::vector<std::string>& lines, std::string_view expected)
{
    for (const auto& line : lines)
    {
        if (line == expected)
        {
            return true;
        }
    }

    return false;
}
} // namespace

int main()
{
    physics_sim::InputBindings bindings = physics_sim::default_input_bindings();
    bindings.pause_resume = SDLK_p;
    bindings.step_once = SDLK_t;
    bindings.reset = SDLK_f;
    bindings.reset_fluid = SDLK_g;
    bindings.toggle_help = SDLK_o;
    bindings.cycle_visual_mode = SDLK_c;
    bindings.tool_prev = SDLK_LEFTBRACKET;
    bindings.tool_next = SDLK_RIGHTBRACKET;
    bindings.speed_down = SDLK_MINUS;
    bindings.speed_up = SDLK_EQUALS;
    bindings.emission_down = SDLK_COMMA;
    bindings.emission_up = SDLK_PERIOD;
    bindings.undo = SDLK_u;
    bindings.redo = SDLK_r;
    bindings.rotate_counterclockwise = SDLK_j;
    bindings.rotate_clockwise = SDLK_l;

    const auto help_lines = physics_sim::build_help_overlay_lines(bindings);
    REQUIRE(contains_line(help_lines, "O toggle help"), "help overlay did not use the active help binding");
    REQUIRE(contains_line(help_lines, "P pause, T step"), "help overlay did not use the active pause/step bindings");
    REQUIRE(contains_line(help_lines, "G clear fluid, F retry current scene"), "help overlay did not use the active reset bindings");
    REQUIRE(contains_line(help_lines, "0 pour, 1 draw wall, 2 erase wall"), "help overlay did not describe the recovery tools");
    REQUIRE(contains_line(help_lines, "[/] cycle sandbox tools"), "help overlay did not use the active tool-cycle bindings");
    REQUIRE(contains_line(help_lines, "Pointer water: hold LMB to pour"), "help overlay did not describe pointer-water hold behavior");
    REQUIRE(contains_line(help_lines, "U undo, R redo"), "help overlay did not use the active undo/redo bindings");
    REQUIRE(!contains_line(help_lines, "Gate tool places a door"), "help overlay exposed a deferred device");

    physics_sim::TutorialProgress progress;
    const auto tutorial_lines = physics_sim::build_tutorial_overlay_lines(progress, bindings);
    REQUIRE(contains_line(tutorial_lines, "P ALSO PAUSES OR RESUMES"), "tutorial overlay did not use the active pause binding");
    REQUIRE(contains_line(tutorial_lines, "> Pour water"), "tutorial overlay did not start with pouring");

    std::cout << "player guidance tests passed" << std::endl;
    return 0;
}
