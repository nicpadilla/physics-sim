#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace physics_sim
{

struct SurfacePoint
{
    float x = 0.0f;
    float y = 0.0f;
};

struct SurfaceTriangle
{
    SurfacePoint a{};
    SurfacePoint b{};
    SurfacePoint c{};
    float coverage = 0.0f;
};

struct SurfaceSemantics
{
    std::size_t occupied_cells = 0;
    std::size_t connected_components = 0;
    std::size_t isolated_cells = 0;
    std::size_t outside_allowed_cells = 0;
    std::size_t min_x = 0;
    std::size_t max_x = 0;
    std::size_t min_y = 0;
    std::size_t max_y = 0;
};

[[nodiscard]] std::vector<SurfaceTriangle> reconstruct_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float cell_size,
    float threshold = 0.05f);

[[nodiscard]] SurfaceSemantics analyze_surface(
    std::span<const float> cell_volume_fractions,
    std::span<const std::uint8_t> solid_cells,
    std::size_t width,
    std::size_t height,
    float threshold = 0.05f,
    std::size_t allowed_min_x = 0,
    std::size_t allowed_max_x = static_cast<std::size_t>(-1),
    std::size_t allowed_min_y = 0,
    std::size_t allowed_max_y = static_cast<std::size_t>(-1));

} // namespace physics_sim
