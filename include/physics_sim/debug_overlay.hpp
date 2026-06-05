#pragma once

#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/simulation_state.hpp>
#include <physics_sim/water_simulation.hpp>

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace physics_sim
{
struct DebugOverlayMetrics
{
    double fps = 0.0;
    const FixedStepDriver* driver = nullptr;
    const SimulationState* state = nullptr;
    const WaterSimulation2D* simulation = nullptr;
    const SceneController* controller = nullptr;
    const char* visual_mode = nullptr;
    const char* status_message = nullptr;
    std::array<std::uint8_t, 4> status_message_rgba{236, 244, 255, 255};
    float status_message_alpha = 1.0f;
};

[[nodiscard]] inline const char* tool_label(SceneTool tool) noexcept
{
    switch (tool)
    {
    case SceneTool::PaintWall:
        return "WALL";
    case SceneTool::EraseWall:
        return "ERAS";
    case SceneTool::DirectionalEmitter:
        return "PIPE";
    case SceneTool::OmniEmitter:
        return "OMNI";
    case SceneTool::Gate:
        return "GATE";
    case SceneTool::Sensor:
        return "SENS";
    case SceneTool::Drain:
        return "DRAIN";
    case SceneTool::Pump:
        return "PUMP";
    case SceneTool::Valve:
        return "VALVE";
    }

    return "UNK";
}

[[nodiscard]] inline std::string format_number(double value, int precision)
{
    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream << std::setprecision(precision) << value;
    return stream.str();
}

[[nodiscard]] inline std::string format_vec2(Vec2 value, int precision)
{
    return format_number(value.x, precision) + ", " + format_number(value.y, precision);
}

[[nodiscard]] inline const char* emitter_kind_label(WaterEmitterKind kind) noexcept
{
    switch (kind)
    {
    case WaterEmitterKind::Directional:
        return "HOSE";
    case WaterEmitterKind::Omni:
        return "OMNI";
    }

    return "UNK";
}

[[nodiscard]] inline std::array<std::string, 15> build_debug_overlay_lines(
    const DebugOverlayMetrics& metrics,
    float zoom)
{
    std::array<std::string, 15> lines{};

    const auto* driver = metrics.driver;
    const auto* state = metrics.state;
    const auto* simulation = metrics.simulation;
    const auto* controller = metrics.controller;
    const auto* selected = controller != nullptr ? controller->selected_fixture() : nullptr;

    lines[0] = "FPS " + format_number(metrics.fps, 2);
    lines[1] = "STEP " + (driver != nullptr ? format_number(driver->fixed_step().count(), 4) : std::string{"0.0000"});
    lines[2] = "TICK " + (state != nullptr ? std::to_string(state->tick_count) : std::string{"0"});
    lines[3] = "PART " + (simulation != nullptr ? std::to_string(simulation->particles().size()) : std::string{"0"});
    lines[4] = "EMIT " + (simulation != nullptr ? std::to_string(simulation->metrics().total_emitted) : std::string{"0"})
        + " REM " + (simulation != nullptr ? std::to_string(simulation->metrics().total_removed) : std::string{"0"});
    lines[5] = "DIV " + (simulation != nullptr ? format_number(simulation->metrics().average_divergence_after_projection, 3) : std::string{"0.000"});
    lines[6] = "MAX " + (simulation != nullptr ? format_number(simulation->metrics().max_divergence_after_projection, 3) : std::string{"0.000"});
    lines[7] = "ZOOM " + format_number(zoom, 2);
    lines[8] = "MODE " + std::string{metrics.visual_mode != nullptr ? metrics.visual_mode : "UNK"};
    lines[9] = "TOOL " + std::string{controller != nullptr ? tool_label(controller->tool()) : "UNK"};

    if (simulation != nullptr && simulation->metrics().objective_sensors > 0)
    {
        lines[10] = "OBJ " + std::to_string(simulation->metrics().objective_sensors)
            + " " + (simulation->metrics().objective_completed ? std::string{"COMPLETE"} : std::string{"RUNNING"});
    }
    else
    {
        lines[10] = "OBJ NONE";
    }

    const auto* selected_gate = controller != nullptr ? controller->selected_gate() : nullptr;
    const auto* selected_sensor = controller != nullptr ? controller->selected_sensor() : nullptr;
    const auto* selected_valve = controller != nullptr ? controller->selected_valve() : nullptr;

    if (selected != nullptr)
    {
        lines[11] = "SEL " + std::string{emitter_kind_label(selected->kind)};
        lines[12] = "POS " + format_vec2(selected->position, 1) + " DIR " + format_vec2(selected->direction, 2);
        lines[13] = "SPD " + format_number(selected->speed, 2) + " RATE " + format_number(selected->emission_rate, 1)
            + " EN " + std::string{selected->enabled ? "ON" : "OFF"};
    }
    else if (selected_gate != nullptr)
    {
        lines[11] = "SEL GATE";
        lines[12] = "POS " + std::to_string(selected_gate->x) + "," + std::to_string(selected_gate->y);
        lines[13] = "STATE " + std::string{selected_gate->open ? "OPEN" : "CLOSED"};
    }
    else if (selected_sensor != nullptr)
    {
        lines[11] = "SEL SENSOR";
        lines[12] = "RECT " + std::to_string(selected_sensor->x) + "," + std::to_string(selected_sensor->y)
            + " " + std::to_string(selected_sensor->width) + "x" + std::to_string(selected_sensor->height);
        lines[13] = "STATE " + std::string{selected_sensor->active ? "ACTIVE" : "IDLE"}
            + " EN " + std::string{selected_sensor->enabled ? "ON" : "OFF"}
            + " OBJ " + std::string{selected_sensor->objective ? "YES" : "NO"};
    }
    else if (selected_valve != nullptr)
    {
        lines[11] = "SEL VALVE";
        lines[12] = "POS " + std::to_string(selected_valve->x) + "," + std::to_string(selected_valve->y);
        lines[13] = "STATE " + std::string{selected_valve->open ? "OPEN" : "CLOSED"};
    }
    else
    {
        lines[11] = "SEL NONE";
        lines[12] = "DEF DIR " + format_vec2(controller != nullptr ? controller->emitter_direction() : Vec2{0.0f, 1.0f}, 2);
        lines[13] = "DEF SPD " + format_number(controller != nullptr ? controller->emitter_speed() : 0.0, 2)
            + " RATE " + format_number(controller != nullptr ? controller->emission_rate() : 0.0, 1);
    }

    if (metrics.status_message != nullptr && metrics.status_message[0] != '\0')
    {
        lines[14] = "MSG " + std::string{metrics.status_message};
    }

    return lines;
}

[[nodiscard]] inline std::array<std::uint8_t, 7> glyph_rows(char character)
{
    const unsigned char code = static_cast<unsigned char>(character);
    const char ch = static_cast<char>(std::toupper(code));

    switch (ch)
    {
    case ' ':
        return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
    case '-':
        return {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000};
    case '.':
        return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00110, 0b00110};
    case '/':
        return {0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b00000, 0b00000};
    case '0':
        return {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
    case '1':
        return {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
    case '2':
        return {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111};
    case '3':
        return {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110};
    case '4':
        return {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010};
    case '5':
        return {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110};
    case '6':
        return {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
    case '7':
        return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
    case '8':
        return {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
    case '9':
        return {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100};
    case 'A':
        return {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
    case 'B':
        return {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110};
    case 'C':
        return {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110};
    case 'D':
        return {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100};
    case 'E':
        return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
    case 'F':
        return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000};
    case 'G':
        return {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110};
    case 'H':
        return {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
    case 'I':
        return {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111};
    case 'J':
        return {0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100};
    case 'K':
        return {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001};
    case 'L':
        return {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
    case 'M':
        return {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001};
    case 'N':
        return {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001};
    case 'O':
        return {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
    case 'P':
        return {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000};
    case 'Q':
        return {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101};
    case 'R':
        return {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001};
    case 'S':
        return {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};
    case 'T':
        return {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
    case 'U':
        return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
    case 'V':
        return {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100};
    case 'W':
        return {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001};
    case 'X':
        return {0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b01010, 0b10001};
    case 'Y':
        return {0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
    case 'Z':
        return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111};
    case '?':
        return {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100};
    default:
        return glyph_rows('?');
    }
}

inline void draw_glyph(SDL_Renderer* renderer, int x, int y, int scale, char character)
{
    const auto glyph = glyph_rows(character);
    SDL_Rect pixel{0, 0, scale, scale};

    for (int row = 0; row < 7; ++row)
    {
        const std::uint8_t mask = glyph[static_cast<std::size_t>(row)];
        for (int column = 0; column < 5; ++column)
        {
            const int bit = 4 - column;
            if ((mask & (1U << bit)) == 0U)
            {
                continue;
            }

            pixel.x = x + column * scale;
            pixel.y = y + row * scale;
            SDL_RenderFillRect(renderer, &pixel);
        }
    }
}

inline void draw_text(SDL_Renderer* renderer, int x, int y, int scale, const std::string& text)
{
    const int advance = 6 * scale;
    int cursor_x = x;
    for (const char character : text)
    {
        if (character == '\n')
        {
            cursor_x = x;
            y += 8 * scale;
            continue;
        }

        draw_glyph(renderer, cursor_x, y, scale, character);
        cursor_x += advance;
    }
}

inline void draw_debug_overlay(SDL_Renderer* renderer, const DebugOverlayMetrics& metrics, float zoom, int x = 16, int y = 16)
{
    const auto lines = build_debug_overlay_lines(metrics, zoom);
    constexpr int scale = 3;
    constexpr int padding = 10;
    constexpr int line_height = 8 * scale;
    constexpr int char_advance = 6 * scale;

    std::size_t max_length = 0;
    for (const auto& line : lines)
    {
        max_length = std::max(max_length, line.size());
    }

    const int width = padding * 2 + static_cast<int>(max_length) * char_advance;
    const int height = padding * 2 + static_cast<int>(lines.size()) * line_height;

    SDL_Rect background{x, y, width, height};
    SDL_SetRenderDrawColor(renderer, 6, 10, 18, 180);
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, 80, 220, 255, 255);
    SDL_RenderDrawRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, 236, 244, 255, 255);
    int line_y = y + padding;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        if (index == lines.size() - 1 && metrics.status_message != nullptr && metrics.status_message[0] != '\0')
        {
            const auto alpha = static_cast<std::uint8_t>(std::clamp(
                static_cast<float>(metrics.status_message_rgba[3]) * std::clamp(metrics.status_message_alpha, 0.0f, 1.0f),
                0.0f,
                255.0f));
            SDL_SetRenderDrawColor(
                renderer,
                metrics.status_message_rgba[0],
                metrics.status_message_rgba[1],
                metrics.status_message_rgba[2],
                alpha);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 236, 244, 255, 255);
        }

        draw_text(renderer, x + padding, line_y, scale, lines[index]);
        line_y += line_height;
    }
}
} // namespace physics_sim
