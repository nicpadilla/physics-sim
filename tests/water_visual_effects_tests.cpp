#include <physics_sim/water_visual_effects.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
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
    constexpr std::size_t width = 80;
    constexpr std::size_t height = 45;
    constexpr float cell_size = 16.0f;
    std::vector<std::uint8_t> solids(width * height, 0);
    for (std::size_t x = 10; x < 70; ++x)
    {
        solids[32 * width + x] = 1;
    }

    std::vector<physics_sim::FluidParticle> still_particles;
    for (std::size_t y = 28; y < 32; ++y)
    {
        for (std::size_t x = 20; x < 60; ++x)
        {
            still_particles.push_back({{(static_cast<float>(x) + 0.5f) * cell_size, (static_cast<float>(y) + 0.5f) * cell_size}, {}});
        }
    }
    const auto still = physics_sim::build_water_visual_effects(still_particles, solids, width, height, cell_size, 2400);
    REQUIRE(still.foam.empty(), "still pool generated foam");
    REQUIRE(still.spray.empty(), "still pool generated spray");
    REQUIRE(still.impacts.empty(), "still pool generated impact accents");

    std::vector<physics_sim::FluidParticle> moving_particles;
    for (std::size_t index = 0; index < 96; ++index)
    {
        const float x = (20.0f + static_cast<float>(index % 24) * 0.35f) * cell_size;
        const float y = (18.0f + static_cast<float>(index / 24) * 0.42f) * cell_size;
        const physics_sim::Vec2 velocity{
            (index % 2 == 0 ? 5.5f : -5.0f) * cell_size,
            (index % 3 == 0 ? 7.0f : 3.5f) * cell_size};
        moving_particles.push_back({{x, y}, velocity});
    }
    for (std::size_t index = 0; index < 24; ++index)
    {
        moving_particles.push_back({
            {(12.0f + static_cast<float>(index) * 0.18f) * cell_size, 31.25f * cell_size},
            {4.0f * cell_size, 5.0f * cell_size}});
    }
    for (std::size_t index = 0; index < 8; ++index)
    {
        moving_particles.push_back({
            {(30.5f + static_cast<float>(index) * 2.0f) * cell_size, 12.5f * cell_size},
            {5.5f * cell_size, -4.5f * cell_size}});
    }

    physics_sim::WaterVisualEffectsSettings settings;
    settings.max_foam_points = 24;
    settings.max_spray_streaks = 12;
    settings.max_impact_accents = 8;
    const auto first = physics_sim::build_water_visual_effects(moving_particles, solids, width, height, cell_size, 480, settings);
    const auto second = physics_sim::build_water_visual_effects(moving_particles, solids, width, height, cell_size, 480, settings);
    REQUIRE(!first.foam.empty(), "moving surface generated no foam");
    REQUIRE(!first.spray.empty(), "fast sparse surface generated no spray");
    REQUIRE(!first.impacts.empty(), "wall-adjacent flow generated no impact accents");
    REQUIRE(first.foam.size() <= settings.max_foam_points, "foam cap was exceeded");
    REQUIRE(first.spray.size() <= settings.max_spray_streaks, "spray cap was exceeded");
    REQUIRE(first.impacts.size() <= settings.max_impact_accents, "impact cap was exceeded");
    REQUIRE(
        physics_sim::water_visual_effects_digest(first) == physics_sim::water_visual_effects_digest(second),
        "identical effect inputs produced different digests");

    settings.reduced_motion = true;
    const auto reduced_a = physics_sim::build_water_visual_effects(moving_particles, solids, width, height, cell_size, 480, settings);
    const auto reduced_b = physics_sim::build_water_visual_effects(moving_particles, solids, width, height, cell_size, 960, settings);
    REQUIRE(
        physics_sim::water_visual_effects_digest(reduced_a) == physics_sim::water_visual_effects_digest(reduced_b),
        "reduced-motion effects changed phase with simulation tick");

    const auto start = std::chrono::steady_clock::now();
    for (int iteration = 0; iteration < 200; ++iteration)
    {
        const auto measured = physics_sim::build_water_visual_effects(
            moving_particles, solids, width, height, cell_size, static_cast<std::uint64_t>(iteration), settings);
        REQUIRE(measured.foam.size() <= settings.max_foam_points, "measured foam exceeded cap");
    }
    const double average_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() / 200.0;
    REQUIRE(average_ms < 1.5, "water visual effects exceeded the presentation CPU budget");
    return 0;
}
