#pragma once

#include <optional>
#include <string_view>

namespace physics_sim
{
enum class FluidSolverProfile
{
    Fast,
    Balanced,
    Quality,
};

[[nodiscard]] inline const char* solver_profile_name(FluidSolverProfile profile) noexcept
{
    switch (profile)
    {
    case FluidSolverProfile::Fast:
        return "fast";
    case FluidSolverProfile::Balanced:
        return "balanced";
    case FluidSolverProfile::Quality:
        return "quality";
    }

    return "balanced";
}

[[nodiscard]] inline std::optional<FluidSolverProfile> parse_solver_profile_token(std::string_view value) noexcept
{
    if (value == "fast" || value == "live")
    {
        return FluidSolverProfile::Fast;
    }

    if (value == "balanced")
    {
        return FluidSolverProfile::Balanced;
    }

    if (value == "quality" || value == "offline")
    {
        return FluidSolverProfile::Quality;
    }

    return std::nullopt;
}

[[nodiscard]] inline FluidSolverProfile next_solver_profile(FluidSolverProfile profile) noexcept
{
    switch (profile)
    {
    case FluidSolverProfile::Fast:
        return FluidSolverProfile::Balanced;
    case FluidSolverProfile::Balanced:
        return FluidSolverProfile::Quality;
    case FluidSolverProfile::Quality:
        return FluidSolverProfile::Fast;
    }

    return FluidSolverProfile::Balanced;
}
} // namespace physics_sim
