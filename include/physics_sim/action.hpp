#pragma once

#include <SDL.h>

#include <optional>

namespace physics_sim
{
enum class Action
{
    Quit,
    TogglePause,
    StepOnce,
    Reset,
    ResetFluid,
};

inline std::optional<Action> action_from_keycode(SDL_Keycode keycode) noexcept
{
    switch (keycode)
    {
    case SDLK_ESCAPE:
        return Action::Quit;
    case SDLK_SPACE:
        return Action::TogglePause;
    case SDLK_s:
        return Action::StepOnce;
    case SDLK_r:
        return Action::ResetFluid;
    case SDLK_F10:
        return Action::Reset;
    default:
        return std::nullopt;
    }
}
} // namespace physics_sim
