#include <physics_sim/water_feel_metrics.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{
[[noreturn]] void fail(const char* message, int line)
{
    std::fprintf(stderr, "FAIL water_feel_metrics_tests.cpp:%d: %s\n", line, message);
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __LINE__); \
        } \
    } while (false)

physics_sim::FluidParticle particle(float x, float y)
{
    physics_sim::FluidParticle value;
    value.position = {x, y};
    value.mass = 1.0f;
    value.volume = 0.25f;
    return value;
}
} // namespace

int main()
{
    physics_sim::MacGrid2D grid{8, 6, 1.0f};
    const std::vector<physics_sim::FluidParticle> level{
        particle(1.25f, 3.25f), particle(1.75f, 3.75f),
        particle(2.25f, 3.25f), particle(2.75f, 3.75f),
        particle(3.25f, 3.25f), particle(3.75f, 3.75f),
    };
    const physics_sim::WaterFeelMetrics level_metrics = physics_sim::measure_water_feel(level, grid);
    REQUIRE(level_metrics.occupied_columns == 3, "level pool occupied-column count was incorrect");
    REQUIRE(std::abs(level_metrics.horizontal_footprint_cells - 2.5) < 1.0e-6, "level pool footprint was incorrect");
    REQUIRE(level_metrics.surface_rms_slope < 1.0e-6, "level pool reported a surface slope");
    REQUIRE(level_metrics.particle_components == 1, "level pool was not connected");
    REQUIRE(std::abs(level_metrics.largest_component_particle_fraction - 1.0) < 1.0e-6, "level pool largest-component fraction was incorrect");
    REQUIRE(level_metrics.particle_count_coefficient_of_variation < 1.0e-6, "uniform sampling reported variation");

    const std::vector<physics_sim::FluidParticle> stepped{
        particle(1.5f, 1.25f), particle(2.5f, 2.25f), particle(3.5f, 3.25f), particle(6.5f, 3.25f),
    };
    const physics_sim::WaterFeelMetrics stepped_metrics = physics_sim::measure_water_feel(stepped, grid);
    REQUIRE(std::abs(stepped_metrics.surface_rms_slope - 1.0) < 1.0e-6, "stepped surface RMS slope was incorrect");
    REQUIRE(std::abs(stepped_metrics.surface_max_slope - 1.0) < 1.0e-6, "stepped surface max slope was incorrect");
    REQUIRE(stepped_metrics.particle_components == 2, "separated support did not produce two components");
    REQUIRE(std::abs(stepped_metrics.largest_component_particle_fraction - 0.75) < 1.0e-6, "largest component fraction was incorrect");

    grid.velocity_v(1, 1) = -1.0f;
    grid.velocity_v(1, 2) = -1.0f;
    grid.velocity_v(3, 1) = 1.0f;
    grid.velocity_v(3, 2) = 1.0f;
    const std::vector<physics_sim::FluidParticle> rotating{particle(2.5f, 1.5f)};
    const physics_sim::WaterFeelMetrics rotating_metrics = physics_sim::measure_water_feel(rotating, grid);
    REQUIRE(rotating_metrics.vorticity_rms > 0.5, "rotating field did not report vorticity");

    return 0;
}
