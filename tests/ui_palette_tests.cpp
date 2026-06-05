#define SDL_MAIN_HANDLED

#include <physics_sim/ui_palette.hpp>

#include <algorithm>
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

[[nodiscard]] double luminance(SDL_Color color) noexcept
{
    return 0.2126 * static_cast<double>(color.r)
        + 0.7152 * static_cast<double>(color.g)
        + 0.0722 * static_cast<double>(color.b);
}

[[nodiscard]] bool color_equals(SDL_Color lhs, SDL_Color rhs) noexcept
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
} // namespace

int main()
{
    const auto normal = physics_sim::ui_palette(false);
    const auto high_contrast = physics_sim::ui_palette(true);

    REQUIRE(normal.screen_background.r == 14 && normal.screen_background.g == 18 && normal.screen_background.b == 28,
        "normal palette background did not preserve the current look");
    REQUIRE(high_contrast.screen_background.r < normal.screen_background.r
            && high_contrast.screen_background.g < normal.screen_background.g
            && high_contrast.screen_background.b < normal.screen_background.b,
        "high-contrast background should be darker than the normal palette");
    REQUIRE(luminance(high_contrast.text) > luminance(normal.text),
        "high-contrast text should be brighter than the normal palette");
    REQUIRE(!color_equals(high_contrast.selection, normal.selection), "high-contrast selection color did not change");
    REQUIRE(!color_equals(high_contrast.warning, normal.warning), "high-contrast warning color did not change");
    REQUIRE(!color_equals(high_contrast.objective, normal.objective), "high-contrast objective color did not change");
    REQUIRE(luminance(high_contrast.text) > luminance(high_contrast.screen_background),
        "high-contrast text should remain readable against the background");

    std::cout << "ui palette tests passed" << std::endl;
    return 0;
}
