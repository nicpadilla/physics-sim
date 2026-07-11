#include "physics_sim/water_visual_effects.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>

namespace physics_sim
{
namespace
{
[[nodiscard]] std::uint64_t mix(std::uint64_t value) noexcept
{
    value ^= value >> 30U;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27U;
    value *= 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

[[nodiscard]] bool adjacent_to_solid(
    Vec2 position,
    std::span<const std::uint8_t> solids,
    std::size_t width,
    std::size_t height,
    float cell_size) noexcept
{
    if (solids.size() != width * height || cell_size <= 0.0f)
    {
        return false;
    }
    const int center_x = static_cast<int>(std::floor(position.x / cell_size));
    const int center_y = static_cast<int>(std::floor(position.y / cell_size));
    for (int offset_y = -1; offset_y <= 1; ++offset_y)
    {
        for (int offset_x = -1; offset_x <= 1; ++offset_x)
        {
            const int x = center_x + offset_x;
            const int y = center_y + offset_y;
            if (x >= 0 && y >= 0 && x < static_cast<int>(width) && y < static_cast<int>(height)
                && solids[static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)] != 0)
            {
                return true;
            }
        }
    }
    return false;
}

void hash_value(std::uint64_t& hash, std::uint32_t value) noexcept
{
    hash ^= value;
    hash *= 1099511628211ULL;
}

void hash_float(std::uint64_t& hash, float value) noexcept
{
    hash_value(hash, std::bit_cast<std::uint32_t>(value));
}
} // namespace

WaterVisualEffects build_water_visual_effects(
    std::span<const FluidParticle> particles,
    std::span<const std::uint8_t> solid_cells,
    std::size_t grid_width,
    std::size_t grid_height,
    float cell_size,
    std::uint64_t simulation_tick,
    WaterVisualEffectsSettings settings)
{
    WaterVisualEffects effects;
    if (!settings.enabled || particles.empty() || grid_width == 0 || grid_height == 0 || cell_size <= 0.0f)
    {
        return effects;
    }

    settings.max_foam_points = std::min<std::size_t>(settings.max_foam_points, 512);
    settings.max_spray_streaks = std::min<std::size_t>(settings.max_spray_streaks, 256);
    settings.max_impact_accents = std::min<std::size_t>(settings.max_impact_accents, 128);
    effects.foam.reserve(std::min(particles.size(), settings.max_foam_points));
    effects.spray.reserve(std::min(particles.size(), settings.max_spray_streaks));
    effects.impacts.reserve(std::min(particles.size(), settings.max_impact_accents));

    std::vector<std::uint16_t> cell_counts(grid_width * grid_height, 0);
    for (const FluidParticle& particle : particles)
    {
        if (particle.position.x < 0.0f || particle.position.y < 0.0f)
        {
            continue;
        }
        const std::size_t x = static_cast<std::size_t>(particle.position.x / cell_size);
        const std::size_t y = static_cast<std::size_t>(particle.position.y / cell_size);
        if (x < grid_width && y < grid_height)
        {
            auto& count = cell_counts[y * grid_width + x];
            count = count == 65535 ? count : static_cast<std::uint16_t>(count + 1);
        }
    }

    const std::uint64_t phase = settings.reduced_motion ? 0 : simulation_tick / 6U;
    for (std::size_t index = 0; index < particles.size(); ++index)
    {
        const FluidParticle& particle = particles[index];
        if (!std::isfinite(particle.position.x) || !std::isfinite(particle.position.y)
            || !std::isfinite(particle.velocity.x) || !std::isfinite(particle.velocity.y)
            || particle.position.x < 0.0f || particle.position.y < 0.0f)
        {
            continue;
        }
        const std::size_t cell_x = static_cast<std::size_t>(particle.position.x / cell_size);
        const std::size_t cell_y = static_cast<std::size_t>(particle.position.y / cell_size);
        if (cell_x >= grid_width || cell_y >= grid_height
            || (solid_cells.size() == grid_width * grid_height && solid_cells[cell_y * grid_width + cell_x] != 0))
        {
            continue;
        }

        const float speed = length(particle.velocity) / cell_size;
        const std::uint16_t local_count = cell_counts[cell_y * grid_width + cell_x];
        const bool exposed = local_count <= 3 || particle.neighbor_count <= 3;
        const std::uint64_t selection = mix(static_cast<std::uint64_t>(index) ^ (phase * 0x9e3779b97f4a7c15ULL));

        if (exposed && speed >= settings.foam_speed_cells_per_second
            && effects.foam.size() < settings.max_foam_points && (selection & 3ULL) == 0ULL)
        {
            const float intensity = std::clamp((speed - settings.foam_speed_cells_per_second) / 5.0f, 0.12f, 1.0f);
            effects.foam.push_back({particle.position, cell_size * (0.055f + intensity * 0.045f), intensity});
        }

        if (exposed && local_count <= 2 && speed >= settings.spray_speed_cells_per_second
            && effects.spray.size() < settings.max_spray_streaks)
        {
            const float intensity = std::clamp((speed - settings.spray_speed_cells_per_second) / 8.0f, 0.15f, 1.0f);
            const Vec2 direction = particle.velocity / std::max(length(particle.velocity), 1.0e-6f);
            const float streak_length = cell_size * (settings.reduced_motion ? 0.12f : 0.18f + intensity * 0.20f);
            effects.spray.push_back({particle.position, particle.position - direction * streak_length, cell_size * 0.035f, intensity});
        }

        if (speed >= settings.impact_speed_cells_per_second
            && adjacent_to_solid(particle.position, solid_cells, grid_width, grid_height, cell_size)
            && effects.impacts.size() < settings.max_impact_accents)
        {
            const float intensity = std::clamp((speed - settings.impact_speed_cells_per_second) / 7.0f, 0.15f, 1.0f);
            effects.impacts.push_back({particle.position, cell_size * (0.10f + intensity * 0.14f), intensity});
        }
    }
    return effects;
}

std::string water_visual_effects_digest(const WaterVisualEffects& effects)
{
    std::uint64_t hash = 1469598103934665603ULL;
    hash_value(hash, static_cast<std::uint32_t>(effects.foam.size()));
    hash_value(hash, static_cast<std::uint32_t>(effects.spray.size()));
    hash_value(hash, static_cast<std::uint32_t>(effects.impacts.size()));
    for (const FoamPoint& point : effects.foam)
    {
        hash_float(hash, point.position.x);
        hash_float(hash, point.position.y);
        hash_float(hash, point.radius);
        hash_float(hash, point.intensity);
    }
    for (const SprayStreak& streak : effects.spray)
    {
        hash_float(hash, streak.start.x);
        hash_float(hash, streak.start.y);
        hash_float(hash, streak.end.x);
        hash_float(hash, streak.end.y);
        hash_float(hash, streak.width);
        hash_float(hash, streak.intensity);
    }
    for (const ImpactAccent& accent : effects.impacts)
    {
        hash_float(hash, accent.position.x);
        hash_float(hash, accent.position.y);
        hash_float(hash, accent.radius);
        hash_float(hash, accent.intensity);
    }
    char buffer[17]{};
    std::snprintf(buffer, sizeof(buffer), "%016llX", static_cast<unsigned long long>(hash));
    return buffer;
}
} // namespace physics_sim
