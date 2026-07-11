#pragma once

#include <physics_sim/fluid_particle.hpp>

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

struct ParticleSurfaceField
{
    std::size_t width = 0;
    std::size_t height = 0;
    float sample_spacing = 1.0f;
    std::vector<float> vertex_values{};
    std::vector<std::uint8_t> solid_cells{};
    double particle_area = 0.0;
};

[[nodiscard]] ParticleSurfaceField build_particle_surface_field(
    std::span<const FluidParticle> particles,
    std::span<const std::uint8_t> coarse_solid_cells,
    std::size_t coarse_width,
    std::size_t coarse_height,
    float cell_size,
    std::size_t subdivisions = 4);

[[nodiscard]] std::vector<SurfaceTriangle> reconstruct_particle_surface(
    const ParticleSurfaceField& field,
    float threshold = -1.0f);

[[nodiscard]] float particle_surface_threshold(const ParticleSurfaceField& field);

[[nodiscard]] double surface_area(std::span<const SurfaceTriangle> triangles) noexcept;

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
