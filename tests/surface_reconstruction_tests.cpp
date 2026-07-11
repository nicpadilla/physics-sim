#include <physics_sim/surface_reconstruction.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        std::fprintf(stderr, "surface reconstruction test failed: %s\n", message);
        std::exit(1);
    }
}
}

int main()
{
    constexpr std::size_t width = 6;
    constexpr std::size_t height = 5;
    std::vector<float> fractions(width * height, 0.0f);
    std::vector<std::uint8_t> solids(width * height, 0);
    for (std::size_t y = 2; y <= 3; ++y)
    {
        for (std::size_t x = 1; x <= 4; ++x)
        {
            fractions[y * width + x] = 1.0f;
        }
    }

    auto triangles = physics_sim::reconstruct_surface(fractions, solids, width, height, 2.0f);
    require(!triangles.empty(), "occupied pool produced no geometry");
    for (const auto& triangle : triangles)
    {
        require(triangle.a.x >= 0.0f && triangle.a.x <= 12.0f, "geometry escaped horizontal grid bounds");
        require(triangle.a.y >= 0.0f && triangle.a.y <= 10.0f, "geometry escaped vertical grid bounds");
    }

    auto semantics = physics_sim::analyze_surface(fractions, solids, width, height, 0.05f, 1, 4, 2, 3);
    require(semantics.occupied_cells == 8, "occupied-cell count was incorrect");
    require(semantics.connected_components == 1, "pool was not one connected component");
    require(semantics.isolated_cells == 0, "pool reported isolated cells");
    require(semantics.outside_allowed_cells == 0, "pool escaped its allowed region");
    require(semantics.min_x == 1 && semantics.max_x == 4 && semantics.min_y == 2 && semantics.max_y == 3,
        "pool extent was incorrect");

    fractions[0] = 0.5f;
    semantics = physics_sim::analyze_surface(fractions, solids, width, height, 0.05f, 1, 4, 2, 3);
    require(semantics.connected_components == 2, "isolated fluid was not a separate component");
    require(semantics.isolated_cells == 1, "isolated fluid was not reported");
    require(semantics.outside_allowed_cells == 1, "out-of-bounds fluid was not reported");

    solids[3 * width + 2] = 1;
    triangles = physics_sim::reconstruct_surface(fractions, solids, width, height, 2.0f);
    require(!triangles.empty(), "solid masking removed all geometry");

    std::vector<std::uint8_t> particle_solids(16 * 12, 0);
    physics_sim::FluidParticle droplet;
    droplet.position = {64.0f, 48.0f};
    droplet.volume = 64.0f;
    droplet.mass = 64.0f;
    const auto droplet_field = physics_sim::build_particle_surface_field(
        std::span<const physics_sim::FluidParticle>{&droplet, 1}, particle_solids, 16, 12, 8.0f, 4);
    const auto droplet_surface = physics_sim::reconstruct_particle_surface(droplet_field);
    const double droplet_area = physics_sim::surface_area(droplet_surface);
    require(!droplet_surface.empty(), "single particle did not reconstruct as a droplet");
    require(std::abs(droplet_area - 64.0) / 64.0 <= 0.15, "single-particle surface was not area faithful");

    double minimum_translated_area = droplet_area;
    double maximum_translated_area = droplet_area;
    for (int offset = 1; offset <= 3; ++offset)
    {
        droplet.position.x = 64.0f + static_cast<float>(offset) * 0.5f;
        const auto translated_field = physics_sim::build_particle_surface_field(
            std::span<const physics_sim::FluidParticle>{&droplet, 1}, particle_solids, 16, 12, 8.0f, 4);
        const double translated_area = physics_sim::surface_area(physics_sim::reconstruct_particle_surface(translated_field));
        minimum_translated_area = std::min(minimum_translated_area, translated_area);
        maximum_translated_area = std::max(maximum_translated_area, translated_area);
    }
    require((maximum_translated_area - minimum_translated_area) / 64.0 <= 0.10,
        "sub-cell translation changed droplet area discontinuously");

    std::vector<physics_sim::FluidParticle> shallow_pool;
    for (int x = 3; x <= 10; ++x)
    {
        for (int sample = 0; sample < 4; ++sample)
        {
            physics_sim::FluidParticle particle;
            particle.position = {
                static_cast<float>(x * 8) + 2.0f + static_cast<float>(sample % 2) * 4.0f,
                70.0f + static_cast<float>(sample / 2) * 4.0f};
            particle.volume = 16.0f;
            particle.mass = 16.0f;
            shallow_pool.push_back(particle);
        }
    }
    const auto pool_field = physics_sim::build_particle_surface_field(shallow_pool, particle_solids, 16, 12, 8.0f, 4);
    const auto pool_surface = physics_sim::reconstruct_particle_surface(pool_field);
    const double pool_area = physics_sim::surface_area(pool_surface);
    const double pool_area_error = std::abs(pool_area - pool_field.particle_area) / pool_field.particle_area;
    if (pool_area_error > 0.20)
    {
        std::fprintf(stderr, "shallow pool area actual=%.3f expected=%.3f relative_error=%.3f\n",
            pool_area, pool_field.particle_area, pool_area_error);
    }
    require(pool_area_error <= 0.20, "shallow-pool surface did not preserve represented area");

    std::fill(particle_solids.begin(), particle_solids.end(), std::uint8_t{0});
    for (std::size_t x = 0; x < 16; ++x)
    {
        particle_solids[7 * 16 + x] = 1;
    }
    droplet.position = {64.0f, 52.0f};
    const auto wall_field = physics_sim::build_particle_surface_field(
        std::span<const physics_sim::FluidParticle>{&droplet, 1}, particle_solids, 16, 12, 8.0f, 4);
    const auto wall_surface = physics_sim::reconstruct_particle_surface(wall_field);
    for (const auto& triangle : wall_surface)
    {
        require(triangle.a.y <= 56.0f && triangle.b.y <= 56.0f && triangle.c.y <= 56.0f,
            "particle surface leaked through a solid floor");
    }
    return 0;
}
