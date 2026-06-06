#include <physics_sim/visual_mode.hpp>

#include <cstdio>
#include <cstdlib>
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
    REQUIRE(std::string_view{physics_sim::visual_mode_name(physics_sim::VisualMode::Surface)} == "surface", "surface visual mode label incorrect");
    REQUIRE(std::string_view{physics_sim::visual_mode_name(physics_sim::VisualMode::Mixed)} == "mixed", "mixed visual mode label incorrect");
    REQUIRE(std::string_view{physics_sim::visual_mode_name(physics_sim::VisualMode::Density)} == "density", "density visual mode label incorrect");
    REQUIRE(std::string_view{physics_sim::visual_mode_name(physics_sim::VisualMode::Particles)} == "particles", "particles visual mode label incorrect");

    REQUIRE(physics_sim::next_visual_mode(physics_sim::VisualMode::Surface) == physics_sim::VisualMode::Mixed, "visual mode cycle from surface incorrect");
    REQUIRE(physics_sim::next_visual_mode(physics_sim::VisualMode::Mixed) == physics_sim::VisualMode::Density, "visual mode cycle from mixed incorrect");
    REQUIRE(physics_sim::next_visual_mode(physics_sim::VisualMode::Density) == physics_sim::VisualMode::Particles, "visual mode cycle from density incorrect");
    REQUIRE(physics_sim::next_visual_mode(physics_sim::VisualMode::Particles) == physics_sim::VisualMode::Surface, "visual mode cycle from particles incorrect");

    return 0;
}
