#pragma once

#include <SDL.h>

#include <cstdint>

namespace physics_sim
{
struct UiPalette
{
    SDL_Color screen_background{};
    SDL_Color grid{};
    SDL_Color water{};
    SDL_Color water_density{};
    SDL_Color wall{};
    SDL_Color emitter_directional{};
    SDL_Color emitter_omni{};
    SDL_Color gate_open{};
    SDL_Color gate_closed{};
    SDL_Color drain_active{};
    SDL_Color drain_inactive{};
    SDL_Color pump_active{};
    SDL_Color pump_inactive{};
    SDL_Color valve_open{};
    SDL_Color valve_closed{};
    SDL_Color sensor_objective_active{};
    SDL_Color sensor_objective_inactive{};
    SDL_Color sensor_normal_active{};
    SDL_Color sensor_normal_inactive{};
    SDL_Color crosshair{};
    SDL_Color tool_preview_valid{};
    SDL_Color tool_preview_invalid{};
    SDL_Color panel_background{};
    SDL_Color panel_border{};
    SDL_Color text{};
    SDL_Color muted_text{};
    SDL_Color selection{};
    SDL_Color success{};
    SDL_Color warning{};
    SDL_Color error{};
    SDL_Color objective{};
    SDL_Color invalid{};
};

[[nodiscard]] inline constexpr SDL_Color rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept
{
    return SDL_Color{r, g, b, a};
}

[[nodiscard]] inline constexpr UiPalette ui_palette(bool high_contrast) noexcept
{
    if (high_contrast)
    {
        return {
            rgba(0, 0, 0),
            rgba(96, 96, 96),
            rgba(0, 210, 255),
            rgba(0, 160, 255),
            rgba(255, 255, 255),
            rgba(255, 255, 0),
            rgba(255, 160, 0),
            rgba(0, 255, 128),
            rgba(255, 255, 255),
            rgba(255, 64, 64),
            rgba(160, 160, 160),
            rgba(0, 200, 255),
            rgba(120, 120, 120),
            rgba(255, 0, 255),
            rgba(200, 200, 200),
            rgba(255, 255, 0, 160),
            rgba(255, 128, 0, 140),
            rgba(0, 255, 255, 160),
            rgba(0, 128, 255, 120),
            rgba(255, 255, 255),
            rgba(255, 255, 255, 180),
            rgba(255, 64, 64, 180),
            rgba(0, 0, 0, 230),
            rgba(255, 255, 255),
            rgba(255, 255, 255),
            rgba(210, 210, 210),
            rgba(255, 255, 0),
            rgba(0, 255, 128),
            rgba(255, 200, 0),
            rgba(255, 64, 64),
            rgba(255, 255, 0),
            rgba(255, 64, 64),
        };
    }

    return {
        rgba(14, 18, 28),
        rgba(38, 48, 62),
        rgba(78, 214, 255),
        rgba(46, 180, 255),
        rgba(54, 58, 67),
        rgba(255, 214, 87),
        rgba(255, 126, 87),
        rgba(170, 196, 114, 160),
        rgba(90, 160, 104, 220),
        rgba(220, 96, 96, 72),
        rgba(110, 90, 90, 72),
        rgba(82, 188, 255, 72),
        rgba(90, 100, 112, 72),
        rgba(184, 148, 255, 160),
        rgba(128, 96, 224, 220),
        rgba(255, 214, 87, 80),
        rgba(255, 128, 128, 80),
        rgba(90, 220, 255, 140),
        rgba(90, 120, 180, 80),
        rgba(122, 226, 255),
        rgba(255, 255, 255, 120),
        rgba(255, 96, 96, 120),
        rgba(8, 12, 20, 230),
        rgba(255, 194, 102),
        rgba(236, 244, 255),
        rgba(210, 214, 224),
        rgba(255, 240, 128),
        rgba(120, 255, 170),
        rgba(255, 194, 102),
        rgba(255, 112, 112),
        rgba(255, 214, 87),
        rgba(255, 96, 96),
    };
}
} // namespace physics_sim
