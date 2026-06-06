#pragma once

namespace physics_sim
{
enum class VisualMode
{
    Surface,
    Mixed,
    Density,
    Particles,
};

[[nodiscard]] inline const char* visual_mode_name(VisualMode mode) noexcept
{
    switch (mode)
    {
    case VisualMode::Surface:
        return "surface";
    case VisualMode::Mixed:
        return "mixed";
    case VisualMode::Density:
        return "density";
    case VisualMode::Particles:
        return "particles";
    }

    return "unknown";
}

[[nodiscard]] inline VisualMode next_visual_mode(VisualMode mode) noexcept
{
    switch (mode)
    {
    case VisualMode::Surface:
        return VisualMode::Mixed;
    case VisualMode::Mixed:
        return VisualMode::Density;
    case VisualMode::Density:
        return VisualMode::Particles;
    case VisualMode::Particles:
        return VisualMode::Surface;
    }

    return VisualMode::Surface;
}
} // namespace physics_sim
