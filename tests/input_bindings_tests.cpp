#define SDL_MAIN_HANDLED

#include <physics_sim/input_bindings.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>

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
    const physics_sim::InputBindings defaults = physics_sim::default_input_bindings();
    REQUIRE(defaults.pause_resume == SDLK_SPACE, "default pause/resume binding was incorrect");
    REQUIRE(defaults.step_once == SDLK_s, "default step binding was incorrect");
    REQUIRE(defaults.reset == SDLK_F10, "default reset binding was incorrect");
    REQUIRE(physics_sim::keycode_token(defaults.pause_resume) == "space", "pause/resume token was incorrect");
    REQUIRE(physics_sim::keycode_label(defaults.reset) == "F10", "reset binding label was incorrect");
    REQUIRE(physics_sim::parse_keycode_token("space").has_value(), "parse_keycode_token rejected space");
    REQUIRE(*physics_sim::parse_keycode_token("space") == SDLK_SPACE, "parse_keycode_token did not round-trip space");
    REQUIRE(physics_sim::validate_input_bindings(defaults), "default bindings should validate");

    physics_sim::InputBindings conflicting = defaults;
    conflicting.reset = SDLK_SPACE;
    REQUIRE(!physics_sim::validate_input_bindings(conflicting), "conflicting bindings were accepted");

    std::cout << "input binding tests passed" << std::endl;
    return 0;
}
