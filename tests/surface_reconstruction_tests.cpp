#include <physics_sim/surface_reconstruction.hpp>

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
    return 0;
}
