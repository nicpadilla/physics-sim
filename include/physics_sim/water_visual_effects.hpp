#pragma once

#include <physics_sim/fluid_particle.hpp>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace physics_sim
{
struct FoamPoint
{
    Vec2 position{};
    float radius = 0.0f;
    float intensity = 0.0f;
};

struct SprayStreak
{
    Vec2 start{};
    Vec2 end{};
    float width = 0.0f;
    float intensity = 0.0f;
};

struct ImpactAccent
{
    Vec2 position{};
    float radius = 0.0f;
    float intensity = 0.0f;
};

struct WaterVisualEffectsSettings
{
    bool enabled = true;
    bool reduced_motion = false;
    std::size_t max_foam_points = 512;
    std::size_t max_spray_streaks = 256;
    std::size_t max_impact_accents = 128;
    float foam_speed_cells_per_second = 1.2f;
    float spray_speed_cells_per_second = 4.0f;
    float impact_speed_cells_per_second = 2.5f;
};

struct WaterVisualEffects
{
    std::vector<FoamPoint> foam{};
    std::vector<SprayStreak> spray{};
    std::vector<ImpactAccent> impacts{};
};

[[nodiscard]] WaterVisualEffects build_water_visual_effects(
    std::span<const FluidParticle> particles,
    std::span<const std::uint8_t> solid_cells,
    std::size_t grid_width,
    std::size_t grid_height,
    float cell_size,
    std::uint64_t simulation_tick,
    WaterVisualEffectsSettings settings = {});

[[nodiscard]] std::string water_visual_effects_digest(const WaterVisualEffects& effects);
} // namespace physics_sim
