#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <array>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <system_error>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <physics_sim/action.hpp>
#include <physics_sim/debug_overlay.hpp>
#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/math.hpp>
#include <physics_sim/replay_script.hpp>
#include <physics_sim/session_shell.hpp>
#include <physics_sim/scene_document.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/scene_viewport.hpp>
#include <physics_sim/user_settings.hpp>
#include <physics_sim/simulation_state.hpp>
#include <physics_sim/visual_mode.hpp>
#include <physics_sim/water_simulation.hpp>

namespace
{
using Clock = std::chrono::steady_clock;
namespace fs = std::filesystem;
using physics_sim::VisualMode;
using physics_sim::next_visual_mode;
using physics_sim::visual_mode_name;

struct RuntimeOptions
{
    std::optional<std::chrono::milliseconds> autoExitAfter;
    std::optional<fs::path> dumpFramePath;
    std::optional<std::chrono::milliseconds> dumpFrameAfter;
    std::optional<std::uint64_t> dumpFrameAfterTicks;
    std::optional<physics_sim::WindowSize> initialWindowSize;
    std::optional<bool> initialHelpOverlayVisible;
    std::optional<fs::path> logFilePath;
    std::optional<fs::path> replayFilePath;
    std::optional<fs::path> settingsFilePath;
    std::optional<fs::path> startupScenePath;
    std::optional<VisualMode> initialVisualMode;
    bool skipSessionShell = false;
};

class AppLogger
{
public:
    explicit AppLogger(fs::path path)
        : path_(std::move(path))
    {
        const auto parent = path_.parent_path();
        if (!parent.empty())
        {
            std::error_code ec;
            static_cast<void>(std::filesystem::create_directories(parent, ec));
        }
    }

    [[nodiscard]] const fs::path& path() const noexcept
    {
        return path_;
    }

    void log(std::string message) const
    {
        std::ofstream file(path_, std::ios::app);
        if (!file.is_open())
        {
            return;
        }

        file << timestamp() << ' ' << message << '\n';
    }

private:
    [[nodiscard]] static std::string timestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_time{};
        localtime_s(&local_time, &now_time);

        std::ostringstream stream;
        stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    fs::path path_{};
};

[[nodiscard]] std::optional<fs::path> default_settings_path()
{
    char* pref_path = SDL_GetPrefPath("Nic", "physics-sim");
    if (pref_path == nullptr)
    {
        return std::nullopt;
    }

    fs::path path = fs::path{pref_path} / "physics-sim-settings.txt";
    SDL_free(pref_path);
    return path;
}

void show_error(const char* title, const std::string& message, const AppLogger* logger = nullptr)
{
    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "fatal " << title << ": " << message;
        logger->log(stream.str());
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message.c_str(), nullptr);
}

std::optional<std::chrono::milliseconds> parse_milliseconds(std::string_view value)
{
    int parsed = 0;
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(begin, end, parsed);
    if (ec != std::errc{} || ptr != end || parsed < 0)
    {
        return std::nullopt;
    }

    return std::chrono::milliseconds(parsed);
}

std::optional<VisualMode> parse_visual_mode(std::string_view value)
{
    if (value == "mixed")
    {
        return VisualMode::Mixed;
    }

    if (value == "density")
    {
        return VisualMode::Density;
    }

    if (value == "particles")
    {
        return VisualMode::Particles;
    }

    return std::nullopt;
}

std::optional<physics_sim::SceneTool> parse_replay_tool(std::string_view value)
{
    if (value == "wall")
    {
        return physics_sim::SceneTool::PaintWall;
    }

    if (value == "erase")
    {
        return physics_sim::SceneTool::EraseWall;
    }

    if (value == "directional")
    {
        return physics_sim::SceneTool::DirectionalEmitter;
    }

    if (value == "omni")
    {
        return physics_sim::SceneTool::OmniEmitter;
    }

    if (value == "gate")
    {
        return physics_sim::SceneTool::Gate;
    }

    if (value == "sensor")
    {
        return physics_sim::SceneTool::Sensor;
    }

    if (value == "drain")
    {
        return physics_sim::SceneTool::Drain;
    }

    if (value == "pump")
    {
        return physics_sim::SceneTool::Pump;
    }

    if (value == "valve")
    {
        return physics_sim::SceneTool::Valve;
    }

    return std::nullopt;
}

std::optional<float> parse_float_token(std::string_view value)
{
    float parsed = 0.0f;
    const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (ec != std::errc{} || ptr != value.data() + value.size())
    {
        return std::nullopt;
    }

    return parsed;
}

std::optional<physics_sim::WindowSize> parse_window_size(std::string_view value)
{
    const auto separator = value.find_first_of("xX");
    if (separator == std::string_view::npos)
    {
        return std::nullopt;
    }

    const std::string_view width_value = value.substr(0, separator);
    const std::string_view height_value = value.substr(separator + 1);
    if (width_value.empty() || height_value.empty())
    {
        return std::nullopt;
    }

    int width = 0;
    int height = 0;

    const auto [width_ptr, width_ec] = std::from_chars(width_value.data(), width_value.data() + width_value.size(), width);
    const auto [height_ptr, height_ec] = std::from_chars(height_value.data(), height_value.data() + height_value.size(), height);
    if (width_ec != std::errc{} || width_ptr != width_value.data() + width_value.size() || width <= 0)
    {
        return std::nullopt;
    }

    if (height_ec != std::errc{} || height_ptr != height_value.data() + height_value.size() || height <= 0)
    {
        return std::nullopt;
    }

    return physics_sim::WindowSize{width, height};
}

RuntimeOptions parse_command_line(int argc, char* argv[])
{
    RuntimeOptions options;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--auto-exit-ms" && i + 1 < argc)
        {
            options.autoExitAfter = parse_milliseconds(argv[++i]);
        }
        else if (arg.starts_with("--auto-exit-ms="))
        {
            options.autoExitAfter = parse_milliseconds(arg.substr(std::string_view("--auto-exit-ms=").size()));
        }
        else if (arg == "--dump-frame" && i + 1 < argc)
        {
            options.dumpFramePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--dump-frame="))
        {
            options.dumpFramePath = fs::path{arg.substr(std::string_view("--dump-frame=").size())};
        }
        else if (arg == "--dump-frame-after-ms" && i + 1 < argc)
        {
            options.dumpFrameAfter = parse_milliseconds(argv[++i]);
        }
        else if (arg.starts_with("--dump-frame-after-ms="))
        {
            options.dumpFrameAfter = parse_milliseconds(arg.substr(std::string_view("--dump-frame-after-ms=").size()));
        }
        else if (arg == "--dump-frame-after-ticks" && i + 1 < argc)
        {
            std::uint64_t parsed = 0;
            const auto value = std::string_view{argv[++i]};
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (ec == std::errc{} && ptr == value.data() + value.size())
            {
                options.dumpFrameAfterTicks = parsed;
            }
        }
        else if (arg.starts_with("--dump-frame-after-ticks="))
        {
            std::uint64_t parsed = 0;
            const std::string_view value = arg.substr(std::string_view("--dump-frame-after-ticks=").size());
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (ec == std::errc{} && ptr == value.data() + value.size())
            {
                options.dumpFrameAfterTicks = parsed;
            }
        }
        else if (arg == "--visual-mode" && i + 1 < argc)
        {
            options.initialVisualMode = parse_visual_mode(argv[++i]);
        }
        else if (arg.starts_with("--visual-mode="))
        {
            options.initialVisualMode = parse_visual_mode(arg.substr(std::string_view("--visual-mode=").size()));
        }
        else if (arg == "--window-size" && i + 1 < argc)
        {
            options.initialWindowSize = parse_window_size(argv[++i]);
        }
        else if (arg.starts_with("--window-size="))
        {
            options.initialWindowSize = parse_window_size(arg.substr(std::string_view("--window-size=").size()));
        }
        else if (arg == "--show-help")
        {
            options.initialHelpOverlayVisible = true;
        }
        else if (arg == "--log-file" && i + 1 < argc)
        {
            options.logFilePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--log-file="))
        {
            options.logFilePath = fs::path{arg.substr(std::string_view("--log-file=").size())};
        }
        else if (arg == "--replay-file" && i + 1 < argc)
        {
            options.replayFilePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--replay-file="))
        {
            options.replayFilePath = fs::path{arg.substr(std::string_view("--replay-file=").size())};
        }
        else if (arg == "--settings-file" && i + 1 < argc)
        {
            options.settingsFilePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--settings-file="))
        {
            options.settingsFilePath = fs::path{arg.substr(std::string_view("--settings-file=").size())};
        }
        else if (arg == "--scene-path" && i + 1 < argc)
        {
            options.startupScenePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--scene-path="))
        {
            options.startupScenePath = fs::path{arg.substr(std::string_view("--scene-path=").size())};
        }
        else if (arg == "--skip-session-shell")
        {
            options.skipSessionShell = true;
        }
    }

    return options;
}

const char* tool_name(physics_sim::SceneTool tool) noexcept
{
    switch (tool)
    {
    case physics_sim::SceneTool::PaintWall:
        return "wall";
    case physics_sim::SceneTool::EraseWall:
        return "erase";
    case physics_sim::SceneTool::DirectionalEmitter:
        return "hose";
    case physics_sim::SceneTool::OmniEmitter:
        return "omni";
    case physics_sim::SceneTool::Gate:
        return "gate";
    case physics_sim::SceneTool::Sensor:
        return "sensor";
    case physics_sim::SceneTool::Drain:
        return "drain";
    case physics_sim::SceneTool::Pump:
        return "pump";
    case physics_sim::SceneTool::Valve:
        return "valve";
    }

    return "unknown";
}

[[nodiscard]] std::string scene_label(const fs::path& path, const physics_sim::SceneMetadata& metadata)
{
    if (!metadata.title.empty())
    {
        return metadata.title;
    }

    if (!path.empty())
    {
        return path.stem().string();
    }

    return "unknown";
}

void draw_grid(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, 38, 48, 62, 255);

    for (std::size_t x = 0; x <= grid.width(); ++x)
    {
        const physics_sim::Vec2 top = viewport.world_to_window({static_cast<float>(x) * cell_size, 0.0f});
        const physics_sim::Vec2 bottom = viewport.world_to_window({static_cast<float>(x) * cell_size, static_cast<float>(grid.height()) * cell_size});
        SDL_RenderDrawLineF(renderer, top.x, top.y, bottom.x, bottom.y);
    }

    for (std::size_t y = 0; y <= grid.height(); ++y)
    {
        const physics_sim::Vec2 left = viewport.world_to_window({0.0f, static_cast<float>(y) * cell_size});
        const physics_sim::Vec2 right = viewport.world_to_window({static_cast<float>(grid.width()) * cell_size, static_cast<float>(y) * cell_size});
        SDL_RenderDrawLineF(renderer, left.x, left.y, right.x, right.y);
    }
}

SDL_FRect world_rect(
    const physics_sim::SceneViewport& viewport,
    physics_sim::Vec2 world_top_left,
    physics_sim::Vec2 world_size)
{
    const physics_sim::Vec2 top_left = viewport.world_to_window(world_top_left);
    const physics_sim::Vec2 bottom_right = viewport.world_to_window(world_top_left + world_size);
    SDL_FRect rect{};
    rect.x = std::min(top_left.x, bottom_right.x);
    rect.y = std::min(top_left.y, bottom_right.y);
    rect.w = std::abs(bottom_right.x - top_left.x);
    rect.h = std::abs(bottom_right.y - top_left.y);
    return rect;
}

void draw_walls(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, 54, 58, 67, 255);
    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            if (!grid.solid(x, y))
            {
                continue;
            }

            const SDL_FRect rect = world_rect(
                viewport,
                {static_cast<float>(x) * cell_size, static_cast<float>(y) * cell_size},
                {cell_size, cell_size});
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}

void draw_filled_circle(SDL_Renderer* renderer, SDL_FRect bounds)
{
    const float radius = std::max(1.0f, std::min(bounds.w, bounds.h) * 0.5f);
    const float center_x = bounds.x + bounds.w * 0.5f;
    const float center_y = bounds.y + bounds.h * 0.5f;

    const int y_min = static_cast<int>(std::floor(center_y - radius));
    const int y_max = static_cast<int>(std::ceil(center_y + radius));
    for (int y = y_min; y <= y_max; ++y)
    {
        const float dy = (static_cast<float>(y) + 0.5f) - center_y;
        const float span_squared = radius * radius - dy * dy;
        if (span_squared <= 0.0f)
        {
            continue;
        }

        const float dx = std::sqrt(span_squared);
        SDL_RenderDrawLineF(renderer, center_x - dx, static_cast<float>(y), center_x + dx, static_cast<float>(y));
    }
}

void draw_particles(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const float cell_size = simulation.grid().cell_size();
    const float particle_size = std::max(8.0f, cell_size * 0.6f);

    SDL_SetRenderDrawColor(renderer, 78, 214, 255, 255);
    for (const auto& particle : simulation.particles())
    {
        const SDL_FRect rect = world_rect(
            viewport,
            {particle.position.x - particle_size * 0.5f, particle.position.y - particle_size * 0.5f},
            {particle_size, particle_size});
        draw_filled_circle(renderer, rect);
    }
}

void draw_fluid_density(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    if (grid.width() == 0 || grid.height() == 0)
    {
        return;
    }

    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            if (grid.solid(x, y))
            {
                continue;
            }

            const float volume_fraction = std::clamp(simulation.cell_volume_fraction(x, y), 0.0f, 1.0f);
            if (volume_fraction <= 0.0f)
            {
                continue;
            }

            const std::uint8_t alpha = static_cast<std::uint8_t>(std::min(220.0f, 32.0f + volume_fraction * 188.0f));
            SDL_SetRenderDrawColor(renderer, 46, 180, 255, alpha);
            const SDL_FRect rect = world_rect(
                viewport,
                {static_cast<float>(x) * grid.cell_size(), static_cast<float>(y) * grid.cell_size()},
                {grid.cell_size(), grid.cell_size()});
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}

void draw_emitters(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller)
{
    const float cell_size = simulation.grid().cell_size();
    const float icon_size = cell_size * 0.4f;
    const float line_length = cell_size * 0.9f;
    const auto selected_index = controller.selected_fixture_index();

    for (std::size_t i = 0; i < simulation.emitters().size(); ++i)
    {
        const auto& emitter = simulation.emitters()[i];
        const SDL_FRect icon = world_rect(
            viewport,
            {emitter.position.x - icon_size * 0.5f, emitter.position.y - icon_size * 0.5f},
            {icon_size, icon_size});

        if (emitter.kind == physics_sim::WaterEmitterKind::Directional)
        {
            SDL_SetRenderDrawColor(renderer, 255, 214, 87, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 126, 87, 255);
        }
        SDL_RenderFillRectF(renderer, &icon);

        const physics_sim::Vec2 direction = physics_sim::normalize(emitter.direction);
        const physics_sim::Vec2 line_end_world = emitter.position + (direction * line_length);
        const physics_sim::Vec2 line_start = viewport.world_to_window(emitter.position);
        const physics_sim::Vec2 line_end = viewport.world_to_window(line_end_world);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLineF(renderer, line_start.x, line_start.y, line_end.x, line_end.y);

        const SDL_FRect highlight = world_rect(
            viewport,
            {emitter.position.x - icon_size * 0.85f, emitter.position.y - icon_size * 0.85f},
            {icon_size * 1.7f, icon_size * 1.7f});
        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, 255, 240, 128, 255);
            SDL_RenderDrawRectF(renderer, &highlight);
        }
    }
}

void draw_gates(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller)
{
    const float cell_size = simulation.grid().cell_size();
    const auto selected_index = controller.selected_gate_index();

    for (std::size_t i = 0; i < simulation.gates().size(); ++i)
    {
        const auto& gate = simulation.gates()[i];
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(gate.x) * cell_size, static_cast<float>(gate.y) * cell_size},
            {cell_size, cell_size});

        if (gate.open)
        {
            SDL_SetRenderDrawColor(renderer, 170, 196, 114, 160);
            SDL_RenderDrawRectF(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 90, 160, 104, 220);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, 255, 240, 128, 255);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_drains(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation)
{
    const float cell_size = simulation.grid().cell_size();

    for (const auto& drain : simulation.drains())
    {
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(drain.x) * cell_size, static_cast<float>(drain.y) * cell_size},
            {static_cast<float>(drain.width) * cell_size, static_cast<float>(drain.height) * cell_size});

        if (drain.enabled)
        {
            SDL_SetRenderDrawColor(renderer, 220, 96, 96, 72);
            SDL_RenderFillRectF(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 255, 142, 118, 220);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 110, 90, 90, 72);
        }

        SDL_RenderDrawRectF(renderer, &rect);
        SDL_RenderDrawLineF(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
        SDL_RenderDrawLineF(renderer, rect.x + rect.w, rect.y, rect.x, rect.y + rect.h);
    }
}

void draw_pumps(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation)
{
    const float cell_size = simulation.grid().cell_size();

    for (const auto& pump : simulation.pumps())
    {
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(pump.x) * cell_size, static_cast<float>(pump.y) * cell_size},
            {static_cast<float>(pump.width) * cell_size, static_cast<float>(pump.height) * cell_size});

        if (pump.enabled)
        {
            SDL_SetRenderDrawColor(renderer, 82, 188, 255, 72);
            SDL_RenderFillRectF(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 96, 220, 255, 220);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 90, 100, 112, 72);
        }

        SDL_RenderDrawRectF(renderer, &rect);

        const physics_sim::Vec2 center_world{
            static_cast<float>(pump.x) * cell_size + static_cast<float>(pump.width) * cell_size * 0.5f,
            static_cast<float>(pump.y) * cell_size + static_cast<float>(pump.height) * cell_size * 0.5f};
        const physics_sim::Vec2 direction = physics_sim::normalize(pump.direction);
        const physics_sim::Vec2 arrow_end_world = center_world + direction * (cell_size * 1.2f);
        const physics_sim::Vec2 arrow_start = viewport.world_to_window(center_world);
        const physics_sim::Vec2 arrow_end = viewport.world_to_window(arrow_end_world);
        SDL_RenderDrawLineF(renderer, arrow_start.x, arrow_start.y, arrow_end.x, arrow_end.y);
    }
}

void draw_valves(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller)
{
    const float cell_size = simulation.grid().cell_size();
    const auto selected_index = controller.selected_valve_index();

    for (std::size_t i = 0; i < simulation.valves().size(); ++i)
    {
        const auto& valve = simulation.valves()[i];
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(valve.x) * cell_size, static_cast<float>(valve.y) * cell_size},
            {cell_size, cell_size});

        if (valve.open)
        {
            SDL_SetRenderDrawColor(renderer, 184, 148, 255, 160);
            SDL_RenderDrawRectF(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 128, 96, 224, 220);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, 255, 240, 128, 255);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_sensors(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller)
{
    const float cell_size = simulation.grid().cell_size();
    const auto selected_index = controller.selected_sensor_index();

    for (std::size_t i = 0; i < simulation.sensors().size(); ++i)
    {
        const auto& sensor = simulation.sensors()[i];
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(sensor.x) * cell_size, static_cast<float>(sensor.y) * cell_size},
            {static_cast<float>(sensor.width) * cell_size, static_cast<float>(sensor.height) * cell_size});

        if (sensor.objective)
        {
            SDL_SetRenderDrawColor(renderer, sensor.active ? 255 : 255, sensor.active ? 214 : 128, sensor.active ? 87 : 128, 80);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, sensor.active ? 90 : 90, sensor.active ? 220 : 120, sensor.active ? 255 : 180, sensor.active ? 140 : 80);
        }
        SDL_RenderFillRectF(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, sensor.enabled ? 110 : 90, sensor.enabled ? 220 : 140, sensor.enabled ? 255 : 120, 220);
        SDL_RenderDrawRectF(renderer, &rect);

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, 255, 240, 128, 255);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_crosshair(SDL_Renderer* renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 122, 226, 255, 255);
    SDL_RenderDrawLine(renderer, x - 12, y, x + 12, y);
    SDL_RenderDrawLine(renderer, x, y - 12, x, y + 12);
    SDL_Rect dot{ x - 2, y - 2, 4, 4 };
    SDL_RenderFillRect(renderer, &dot);
}

void draw_help_overlay(SDL_Renderer* renderer, int window_width, int window_height)
{
    const std::array<std::string, 22> lines{
        "HELP",
        "LMB paint / place / select",
        "RMB clear selection",
        "1-9 tools, Tab cycle, V visual mode",
        "H toggle help",
        "Esc opens pause menu / backs out",
        "Enter or click confirm menus",
        "Ctrl+Z undo, Ctrl+Y redo",
        "Arrows/WASD set direction",
        "Ctrl+Arrows/WASD move selected",
        "Q/E rotate selected or tool",
        "[/ ] speed, -/= emission",
        "T toggle selected device",
        "Gate tool places a door",
        "Sensor tool places trigger",
        "Drain tool removes water",
        "Pump tool pushes flow",
        "Valve tool routes flow",
        "Space pause, S step",
        "R clear fluid, F10 retry current scene",
        "PgUp/PgDn browse scenes",
        "Delete remove selected or clear scene",
    };

    constexpr int scale = 2;
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
    const int x = std::max(16, window_width - width - 16);
    const int y = std::max(16, window_height - height - 16);

    SDL_Rect background{x, y, width, height};
    SDL_SetRenderDrawColor(renderer, 6, 10, 18, 210);
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, 255, 194, 102, 255);
    SDL_RenderDrawRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, 236, 244, 255, 255);
    int line_y = y + padding;
    for (const auto& line : lines)
    {
        physics_sim::draw_text(renderer, x + padding, line_y, scale, line);
        line_y += line_height;
    }
}

[[nodiscard]] std::vector<std::string> build_session_shell_body_lines(physics_sim::SessionShellScreen screen)
{
    switch (screen)
    {
    case physics_sim::SessionShellScreen::MainMenu:
        return {
            "MAIN MENU",
            "UP/DOWN SELECT",
            "ENTER CONFIRM",
            "ESC CLOSES MENUS",
        };
    case physics_sim::SessionShellScreen::SceneBrowser:
        return {
            "SCENE BROWSER",
            "UP/DOWN SELECT",
            "ENTER LOADS SCENE",
            "ESC GOES BACK",
        };
    case physics_sim::SessionShellScreen::Settings:
        return {
            "SETTINGS",
            "UP/DOWN SELECT",
            "ENTER APPLIES",
            "ESC GOES BACK",
        };
    case physics_sim::SessionShellScreen::About:
        return {
            "ABOUT",
            "DETERMINISTIC 2D WATER SANDBOX",
            "C++20 / SDL2",
            "ESC OR ENTER RETURNS",
        };
    case physics_sim::SessionShellScreen::PauseMenu:
        return {
            "PAUSED",
            "UP/DOWN SELECT",
            "ENTER CONFIRM",
            "SPACE RESUMES",
        };
    case physics_sim::SessionShellScreen::Playing:
        return {
            "PLAYING",
        };
    }

    return {"MENU"};
}

[[nodiscard]] std::vector<std::string> build_session_shell_option_lines(
    const physics_sim::SessionShellState& shell,
    const std::array<fs::path, 6>& galleryScenePaths)
{
    std::vector<std::string> lines;

    switch (shell.screen)
    {
    case physics_sim::SessionShellScreen::Playing:
        break;
    case physics_sim::SessionShellScreen::MainMenu:
        lines.reserve(5);
        for (std::size_t index = 0; index < physics_sim::session_shell_option_count(shell.screen, galleryScenePaths.size()); ++index)
        {
            lines.emplace_back(physics_sim::session_shell_main_menu_label(index));
        }
        break;
    case physics_sim::SessionShellScreen::SceneBrowser:
        lines.reserve(galleryScenePaths.size() + 1);
        for (const auto& scene_path : galleryScenePaths)
        {
            lines.emplace_back(scene_path.stem().string());
        }
        lines.emplace_back("BACK");
        break;
    case physics_sim::SessionShellScreen::Settings:
        lines.reserve(3);
        for (std::size_t index = 0; index < 3; ++index)
        {
            lines.emplace_back(physics_sim::session_shell_settings_label(index));
        }
        break;
    case physics_sim::SessionShellScreen::About:
        lines.emplace_back("BACK");
        break;
    case physics_sim::SessionShellScreen::PauseMenu:
        lines.reserve(8);
        for (std::size_t index = 0; index < 8; ++index)
        {
            lines.emplace_back(physics_sim::session_shell_pause_menu_label(index));
        }
        break;
    }

    return lines;
}

void draw_session_shell(
    SDL_Renderer* renderer,
    int window_width,
    int window_height,
    const physics_sim::SessionShellState& shell,
    const std::array<fs::path, 6>& galleryScenePaths)
{
    const auto body_lines = build_session_shell_body_lines(shell.screen);
    const auto option_lines = build_session_shell_option_lines(shell, galleryScenePaths);
    std::vector<std::string> all_lines = body_lines;
    if (!option_lines.empty())
    {
        all_lines.push_back("");
        all_lines.insert(all_lines.end(), option_lines.begin(), option_lines.end());
    }

    constexpr int scale = 2;
    constexpr int padding = 12;
    constexpr int line_height = 8 * scale;
    constexpr int char_advance = 6 * scale;

    std::size_t max_length = 0;
    for (const auto& line : all_lines)
    {
        max_length = std::max(max_length, line.size());
    }

    const int width = padding * 2 + static_cast<int>(max_length) * char_advance;
    const int height = padding * 2 + static_cast<int>(all_lines.size()) * line_height;
    const int x = std::max(24, (window_width - width) / 2);
    const int y = std::max(24, (window_height - height) / 2);

    SDL_Rect panel{x, y, width, height};
    SDL_SetRenderDrawColor(renderer, 8, 12, 20, 230);
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, 255, 194, 102, 255);
    SDL_RenderDrawRect(renderer, &panel);

    int line_y = y + padding;
    for (std::size_t index = 0; index < body_lines.size(); ++index)
    {
        SDL_SetRenderDrawColor(renderer, 236, 244, 255, 255);
        physics_sim::draw_text(renderer, x + padding, line_y, scale, body_lines[index]);
        line_y += line_height;
    }

    if (!option_lines.empty())
    {
        line_y += line_height;
        for (std::size_t index = 0; index < option_lines.size(); ++index)
        {
            const bool selected = index == shell.selection;
            SDL_SetRenderDrawColor(renderer, selected ? 255 : 182, selected ? 240 : 210, selected ? 128 : 230, 255);
            physics_sim::draw_text(renderer, x + padding, line_y, scale, option_lines[index]);
            line_y += line_height;
        }
    }
}

[[nodiscard]] std::optional<std::size_t> session_shell_option_index_at_point(
    int mouse_x,
    int mouse_y,
    int window_width,
    int window_height,
    const physics_sim::SessionShellState& shell,
    const std::array<fs::path, 6>& galleryScenePaths)
{
    const auto body_lines = build_session_shell_body_lines(shell.screen);
    const auto option_lines = build_session_shell_option_lines(shell, galleryScenePaths);

    if (option_lines.empty())
    {
        return std::nullopt;
    }

    std::vector<std::string> all_lines = body_lines;
    all_lines.push_back("");
    all_lines.insert(all_lines.end(), option_lines.begin(), option_lines.end());

    constexpr int scale = 2;
    constexpr int padding = 12;
    constexpr int line_height = 8 * scale;
    constexpr int char_advance = 6 * scale;

    std::size_t max_length = 0;
    for (const auto& line : all_lines)
    {
        max_length = std::max(max_length, line.size());
    }

    const int width = padding * 2 + static_cast<int>(max_length) * char_advance;
    const int height = padding * 2 + static_cast<int>(all_lines.size()) * line_height;
    const int x = std::max(24, (window_width - width) / 2);
    const int y = std::max(24, (window_height - height) / 2);
    const int option_start_y = y + padding + static_cast<int>(body_lines.size()) * line_height + line_height;
    const int option_end_y = option_start_y + static_cast<int>(option_lines.size()) * line_height;

    if (mouse_x < x || mouse_x >= x + width || mouse_y < option_start_y || mouse_y >= option_end_y)
    {
        return std::nullopt;
    }

    const std::size_t index = static_cast<std::size_t>((mouse_y - option_start_y) / line_height);
    if (index >= option_lines.size())
    {
        return std::nullopt;
    }

    return index;
}

void draw_tool_preview(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::Vec2& mouse_world)
{
    const float cell_size = simulation.grid().cell_size();
    const auto tool = controller.tool();
    if (tool == physics_sim::SceneTool::PaintWall || tool == physics_sim::SceneTool::EraseWall)
    {
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size, cell_size});
        SDL_SetRenderDrawColor(renderer, tool == physics_sim::SceneTool::PaintWall ? 96 : 255, 196, 255, 120);
        SDL_RenderDrawRectF(renderer, &rect);
    }
    else if (tool == physics_sim::SceneTool::DirectionalEmitter || tool == physics_sim::SceneTool::OmniEmitter)
    {
        const bool valid_placement = controller.can_place_fixture(mouse_world);
        SDL_SetRenderDrawColor(renderer, valid_placement ? 255 : 255, valid_placement ? 255 : 96, valid_placement ? 255 : 96, 160);

        if (tool == physics_sim::SceneTool::DirectionalEmitter)
        {
            const float icon_size = cell_size * 0.45f;
            const SDL_FRect rect = world_rect(
                viewport,
                {mouse_world.x - icon_size * 0.5f, mouse_world.y - icon_size * 0.5f},
                {icon_size, icon_size});
            SDL_RenderDrawRectF(renderer, &rect);

            const physics_sim::Vec2 direction = controller.emitter_direction();
            const physics_sim::Vec2 arrow_end = mouse_world + direction * (cell_size * 0.55f);
            const physics_sim::Vec2 window_start = viewport.world_to_window(mouse_world);
            const physics_sim::Vec2 window_end = viewport.world_to_window(arrow_end);
            SDL_RenderDrawLineF(renderer, window_start.x, window_start.y, window_end.x, window_end.y);
        }
        else
        {
            const float circle_size = cell_size * 0.7f;
            const SDL_FRect rect = world_rect(
                viewport,
                {mouse_world.x - circle_size * 0.5f, mouse_world.y - circle_size * 0.5f},
                {circle_size, circle_size});
            draw_filled_circle(renderer, rect);
        }
    }
    else if (tool == physics_sim::SceneTool::Gate)
    {
        const bool valid_placement = controller.can_place_gate(mouse_world);
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size, cell_size});
        SDL_SetRenderDrawColor(renderer, valid_placement ? 88 : 255, valid_placement ? 200 : 96, valid_placement ? 120 : 96, 160);
        SDL_RenderDrawRectF(renderer, &rect);
        if (valid_placement)
        {
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
    else if (tool == physics_sim::SceneTool::Sensor)
    {
        const bool valid_placement = controller.can_place_sensor(mouse_world);
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size * 3.0f, cell_size * 3.0f});
        SDL_SetRenderDrawColor(renderer, valid_placement ? 110 : 255, valid_placement ? 180 : 96, valid_placement ? 255 : 96, 160);
        SDL_RenderDrawRectF(renderer, &rect);
        if (valid_placement)
        {
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
    else if (tool == physics_sim::SceneTool::Drain)
    {
        const bool valid_placement = controller.can_place_drain(mouse_world);
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size * 3.0f, cell_size * 3.0f});
        SDL_SetRenderDrawColor(renderer, valid_placement ? 255 : 255, valid_placement ? 96 : 96, valid_placement ? 96 : 96, 160);
        SDL_RenderDrawRectF(renderer, &rect);
        if (valid_placement)
        {
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
    else if (tool == physics_sim::SceneTool::Pump)
    {
        const bool valid_placement = controller.can_place_pump(mouse_world);
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size * 3.0f, cell_size * 3.0f});
        SDL_SetRenderDrawColor(renderer, valid_placement ? 96 : 255, valid_placement ? 220 : 96, valid_placement ? 255 : 96, 160);
        SDL_RenderDrawRectF(renderer, &rect);
        if (valid_placement)
        {
            SDL_RenderFillRectF(renderer, &rect);
        }

        const physics_sim::Vec2 center_world{
            cell_top_left.x + cell_size * 1.5f,
            cell_top_left.y + cell_size * 1.5f};
        const physics_sim::Vec2 direction = physics_sim::normalize(controller.emitter_direction());
        const physics_sim::Vec2 arrow_end = center_world + direction * (cell_size * 1.4f);
        const physics_sim::Vec2 window_start = viewport.world_to_window(center_world);
        const physics_sim::Vec2 window_end = viewport.world_to_window(arrow_end);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
        SDL_RenderDrawLineF(renderer, window_start.x, window_start.y, window_end.x, window_end.y);
    }
    else if (tool == physics_sim::SceneTool::Valve)
    {
        const bool valid_placement = controller.can_place_valve(mouse_world);
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size, cell_size});
        SDL_SetRenderDrawColor(renderer, valid_placement ? 184 : 255, valid_placement ? 148 : 96, valid_placement ? 255 : 96, 160);
        SDL_RenderDrawRectF(renderer, &rect);
        if (valid_placement)
        {
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
    else
    {
        const float icon_size = cell_size * 0.4f;
        const SDL_FRect rect = world_rect(
            viewport,
            {mouse_world.x - icon_size * 0.5f, mouse_world.y - icon_size * 0.5f},
            {icon_size, icon_size});
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 160);
        SDL_RenderDrawRectF(renderer, &rect);
    }
}

void sync_viewport_to_simulation(physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const float world_width = static_cast<float>(simulation.grid().width()) * simulation.grid().cell_size();
    const float world_height = static_cast<float>(simulation.grid().height()) * simulation.grid().cell_size();
    viewport.set_world_size({world_width, world_height});
}

void paint_wall_line(
    physics_sim::SceneController& controller,
    physics_sim::Vec2 start,
    physics_sim::Vec2 end)
{
    controller.begin_stroke(start);
    controller.drag_stroke(end);
    controller.end_stroke(end);
}

void load_demo_scene(physics_sim::SceneController& controller)
{
    const auto previous_tool = controller.tool();

    controller.reset_scene();

    controller.set_tool(physics_sim::SceneTool::PaintWall);
    paint_wall_line(controller, {320.0f, 576.0f}, {960.0f, 576.0f});
    paint_wall_line(controller, {320.0f, 384.0f}, {320.0f, 576.0f});
    paint_wall_line(controller, {960.0f, 384.0f}, {960.0f, 576.0f});

    controller.set_emitter_direction({0.0f, 1.0f});
    controller.set_emitter_speed(8.0f);
    controller.set_emission_rate(120.0f);
    controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
    controller.place_fixture({640.0f, 160.0f});

    controller.set_tool(previous_tool);
    controller.sync_history();
}

bool load_scene_from_file(
    const fs::path& path,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    physics_sim::SimulationState& state,
    physics_sim::FixedStepDriver& driver,
    physics_sim::SceneController& controller,
    physics_sim::SceneMetadata* metadata_out = nullptr,
    const AppLogger* logger = nullptr)
{
    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene load attempt: " << path.string();
        logger->log(stream.str());
    }

    if (!physics_sim::load_scene(path, simulation, metadata_out))
    {
        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "scene load failed: " << path.string() << " reason=";
            std::error_code exists_ec;
            stream << (std::filesystem::exists(path, exists_ec) ? "invalid scene data" : "file missing");
            logger->log(stream.str());
        }

        return false;
    }

    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene load ok: " << path.string();
        logger->log(stream.str());
    }

    sync_viewport_to_simulation(viewport, simulation);
    state.reset();
    driver.reset();
    controller.clear_selection();
    controller.sync_history();
    return true;
}

bool save_frame(SDL_Renderer* renderer, const fs::path& path, int width, int height, const AppLogger* logger = nullptr)
{
    const auto write_failure_report = [&](const std::string& reason)
    {
        std::ofstream report(path.string() + ".error.txt", std::ios::trunc);
        if (report.is_open())
        {
            report << reason << "\n";
            report << SDL_GetError() << "\n";
        }
    };

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surface == nullptr)
    {
        write_failure_report("SDL_CreateRGBSurfaceWithFormat failed");
        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "frame capture failed: " << path.string() << " reason=SDL_CreateRGBSurfaceWithFormat failed";
            logger->log(stream.str());
        }
        return false;
    }

    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    SDL_RenderFlush(renderer);
    const int read_result = SDL_RenderReadPixels(renderer, nullptr, surface->format->format, surface->pixels, surface->pitch);
    if (read_result != 0)
    {
        write_failure_report("SDL_RenderReadPixels failed");
        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "frame capture failed: " << path.string() << " reason=SDL_RenderReadPixels failed";
            logger->log(stream.str());
        }
        SDL_FreeSurface(surface);
        return false;
    }

    const std::string path_utf8 = path.string();
    const int save_result = SDL_SaveBMP(surface, path_utf8.c_str());
    SDL_FreeSurface(surface);
    if (save_result != 0)
    {
        write_failure_report("SDL_SaveBMP failed");
        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "frame capture failed: " << path.string() << " reason=SDL_SaveBMP failed";
            logger->log(stream.str());
        }
    }
    else if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "frame capture ok: " << path.string();
        logger->log(stream.str());
    }
    return save_result == 0;
}

bool restore_demo_scene(
    const fs::path& path,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    physics_sim::SimulationState& state,
    physics_sim::FixedStepDriver& driver,
    physics_sim::SceneController& controller,
    physics_sim::SceneMetadata* metadata_out = nullptr,
    const AppLogger* logger = nullptr)
{
    if (load_scene_from_file(path, simulation, viewport, state, driver, controller, metadata_out, logger))
    {
        return true;
    }

    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene fallback: loading demo scene after failure for " << path.string();
        logger->log(stream.str());
    }

    load_demo_scene(controller);
    sync_viewport_to_simulation(viewport, simulation);
    state.reset();
    driver.reset();
    controller.clear_selection();
    controller.sync_history();
    if (metadata_out != nullptr)
    {
        *metadata_out = {};
    }

    if (physics_sim::save_scene(path, simulation, physics_sim::SceneMetadata{}))
    {
        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "scene save ok: " << path.string();
            logger->log(stream.str());
        }
    }
    else if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene save failed: " << path.string();
        logger->log(stream.str());
    }
    return false;
}

std::string build_window_title(
    const physics_sim::SimulationState& state,
    const physics_sim::FixedStepDriver& driver,
    const physics_sim::FixedStepDriver::AdvanceResult& last_update,
    double average_fps,
    const physics_sim::WaterSimulation2D& simulation,
    const char* visual_mode_label,
    const fs::path& current_scene_path,
    const physics_sim::SceneMetadata& scene_metadata,
    const physics_sim::SceneController& controller,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::SessionShellState* session_shell_state = nullptr)
{
    std::ostringstream title;
    title.setf(std::ios::fixed);
    title.precision(2);
    title << "Physics Sim"
          << " | " << (driver.paused() ? "paused" : "running")
          << " | mode=" << (visual_mode_label != nullptr ? visual_mode_label : "unknown")
          << " | tool=" << tool_name(controller.tool())
          << " | scene=" << scene_label(current_scene_path, scene_metadata)
          << " | ticks=" << state.tick_count
          << " | sim=" << state.simulated_time.count() << "s"
          << " | fps=" << average_fps
          << " | step=" << driver.fixed_step().count() << "s"
          << " | particles=" << simulation.particles().size()
          << " | emitters=" << simulation.emitters().size()
          << " | emitted=" << simulation.metrics().total_emitted
          << " | removed=" << simulation.metrics().total_removed
          << " | outflow=" << simulation.metrics().total_outflow
          << " | objective=" << (simulation.metrics().objective_sensors > 0 ? (simulation.metrics().objective_completed ? "done" : "running") : "off")
          << " | div=" << simulation.metrics().average_divergence_after_projection
          << "/" << simulation.metrics().max_divergence_after_projection
          << " | zoom=" << viewport.scale();

    if (session_shell_state != nullptr && session_shell_state->screen != physics_sim::SessionShellScreen::Playing)
    {
        title << " | shell=" << physics_sim::session_shell_screen_title(session_shell_state->screen);
    }

    if (const auto* selected = controller.selected_fixture())
    {
        title << " | sel=" << (selected->kind == physics_sim::WaterEmitterKind::Directional ? "hose" : "omni")
              << "@" << selected->position.x << "," << selected->position.y;
    }
    else if (const auto* selected_gate = controller.selected_gate())
    {
        title << " | sel=gate@" << selected_gate->x << "," << selected_gate->y
              << " " << (selected_gate->open ? "open" : "closed");
    }
    else if (const auto* selected_sensor = controller.selected_sensor())
    {
        title << " | sel=sensor@" << selected_sensor->x << "," << selected_sensor->y
              << " " << selected_sensor->width << "x" << selected_sensor->height
              << " " << (selected_sensor->active ? "active" : "idle");
    }
    else if (const auto* selected_valve = controller.selected_valve())
    {
        title << " | sel=valve@" << selected_valve->x << "," << selected_valve->y
              << " " << (selected_valve->open ? "open" : "closed");
    }

    if (last_update.steps > 0)
    {
        title << " | last=" << last_update.steps;
    }

    if (last_update.clamped)
    {
        title << " | clamped";
    }

    return title.str();
}

void apply_action(
    physics_sim::Action action,
    physics_sim::FixedStepDriver& driver,
    physics_sim::SimulationState& state,
    physics_sim::SceneController& controller,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    const fs::path& current_scene_path,
    physics_sim::SceneMetadata* metadata_out = nullptr,
    std::string* feedback_out = nullptr,
    const AppLogger* logger = nullptr)
{
    switch (action)
    {
    case physics_sim::Action::Quit:
        break;
    case physics_sim::Action::TogglePause:
        driver.toggle_pause();
        break;
    case physics_sim::Action::StepOnce:
        driver.request_step();
        break;
    case physics_sim::Action::Reset:
        if (logger != nullptr)
        {
            logger->log("action: reset");
        }
        restore_demo_scene(current_scene_path, simulation, viewport, state, driver, controller, metadata_out, logger);
        if (feedback_out != nullptr)
        {
            *feedback_out = "RETRY SCENE";
        }
        break;
    case physics_sim::Action::ResetFluid:
        if (logger != nullptr)
        {
            logger->log("action: reset-fluid");
        }
        simulation.clear_fluid();
        state.reset();
        driver.reset();
        controller.clear_selection();
        controller.sync_history();
        if (feedback_out != nullptr)
        {
            *feedback_out = "RESET FLUID";
        }
        break;
    }
}

void handle_fixture_edit_key(
    physics_sim::SceneController& controller,
    SDL_Keycode keycode,
    SDL_Keymod modifiers,
    float move_step)
{
    const bool has_selection = controller.has_selected_fixture();
    const bool move_selected = has_selection && (modifiers & KMOD_CTRL) != 0;

    if (move_selected)
    {
        switch (keycode)
        {
        case SDLK_UP:
        case SDLK_w:
            static_cast<void>(controller.move_selected_fixture({0.0f, -move_step}));
            return;
        case SDLK_DOWN:
        case SDLK_s:
            static_cast<void>(controller.move_selected_fixture({0.0f, move_step}));
            return;
        case SDLK_LEFT:
        case SDLK_a:
            static_cast<void>(controller.move_selected_fixture({-move_step, 0.0f}));
            return;
        case SDLK_RIGHT:
        case SDLK_d:
            static_cast<void>(controller.move_selected_fixture({move_step, 0.0f}));
            return;
        default:
            break;
        }
    }

    switch (keycode)
    {
    case SDLK_UP:
    case SDLK_w:
        if (has_selection)
        {
            static_cast<void>(controller.set_selected_fixture_direction({0.0f, -1.0f}));
        }
        else
        {
            controller.set_emitter_direction({0.0f, -1.0f});
        }
        break;
    case SDLK_DOWN:
    case SDLK_s:
        if (has_selection)
        {
            static_cast<void>(controller.set_selected_fixture_direction({0.0f, 1.0f}));
        }
        else
        {
            controller.set_emitter_direction({0.0f, 1.0f});
        }
        break;
    case SDLK_LEFT:
    case SDLK_a:
        if (has_selection)
        {
            static_cast<void>(controller.set_selected_fixture_direction({-1.0f, 0.0f}));
        }
        else
        {
            controller.set_emitter_direction({-1.0f, 0.0f});
        }
        break;
    case SDLK_RIGHT:
    case SDLK_d:
        if (has_selection)
        {
            static_cast<void>(controller.set_selected_fixture_direction({1.0f, 0.0f}));
        }
        else
        {
            controller.set_emitter_direction({1.0f, 0.0f});
        }
        break;
    case SDLK_q:
        if (has_selection)
        {
            static_cast<void>(controller.rotate_selected_fixture(-0.2617994f));
        }
        else
        {
            controller.rotate_emitter_direction(-0.2617994f);
        }
        break;
    case SDLK_e:
        if (has_selection)
        {
            static_cast<void>(controller.rotate_selected_fixture(0.2617994f));
        }
        else
        {
            controller.rotate_emitter_direction(0.2617994f);
        }
        break;
    case SDLK_LEFTBRACKET:
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_speed(-1.0f));
        }
        else
        {
            controller.set_emitter_speed(std::max(0.0f, controller.emitter_speed() - 1.0f));
        }
        break;
    case SDLK_RIGHTBRACKET:
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_speed(1.0f));
        }
        else
        {
            controller.set_emitter_speed(controller.emitter_speed() + 1.0f);
        }
        break;
    case SDLK_MINUS:
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_emission_rate(-4.0f));
        }
        else
        {
            controller.set_emission_rate(std::max(0.0f, controller.emission_rate() - 4.0f));
        }
        break;
    case SDLK_EQUALS:
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_emission_rate(4.0f));
        }
        else
        {
            controller.set_emission_rate(controller.emission_rate() + 4.0f);
        }
        break;
    case SDLK_t:
        if (controller.has_selected_gate())
        {
            static_cast<void>(controller.toggle_selected_gate_open());
        }
        else if (controller.has_selected_valve())
        {
            static_cast<void>(controller.toggle_selected_valve_open());
        }
        else if (controller.has_selected_sensor())
        {
            static_cast<void>(controller.toggle_selected_sensor_enabled());
        }
        else if (has_selection)
        {
            static_cast<void>(controller.toggle_selected_fixture_enabled());
        }
        break;
    default:
        break;
    }
}
} // namespace

int main(int argc, char* argv[])
{
    const RuntimeOptions options = parse_command_line(argc, argv);
    SDL_SetMainReady();
    const fs::path demoScenePath{"scenes/demo_scene.pscene"};
    const fs::path autosaveScenePath{"scenes/autosave.pscene"};
    const fs::path logFilePath = options.logFilePath.value_or(fs::path{"physics-sim.log"});
    const fs::path startupScenePath = options.startupScenePath.value_or(demoScenePath);
    const std::array<fs::path, 6> galleryScenePaths{
        demoScenePath,
        fs::path{"scenes/free_fall.pscene"},
        fs::path{"scenes/hose_wall_impact.pscene"},
        fs::path{"scenes/omni_spray.pscene"},
        fs::path{"scenes/objective_fill.pscene"},
        fs::path{"scenes/future_device.pscene"},
    };
    const auto same_scene_path = [](const fs::path& lhs, const fs::path& rhs) -> bool
    {
        std::error_code ec;
        if (fs::equivalent(lhs, rhs, ec))
        {
            return true;
        }

        return lhs.lexically_normal() == rhs.lexically_normal();
    };
    const auto gallery_index_for_path = [&](const fs::path& path) -> std::optional<std::size_t>
    {
        for (std::size_t index = 0; index < galleryScenePaths.size(); ++index)
        {
            if (same_scene_path(path, galleryScenePaths[index]))
            {
                return index;
            }
        }

        return std::nullopt;
    };
    AppLogger logger{logFilePath};

    {
        std::ostringstream stream;
        stream << "startup: log_file=" << logFilePath.string()
               << " scene_path=" << startupScenePath.string()
               << " autosave_path=" << autosaveScenePath.string();
        if (options.replayFilePath.has_value())
        {
            stream << " replay_file=" << options.replayFilePath->string();
        }
        if (options.initialVisualMode.has_value())
        {
            stream << " initial_visual_mode=" << visual_mode_name(*options.initialVisualMode);
        }
        logger.log(stream.str());
    }

    std::optional<physics_sim::ReplayScript> replayScript;
    std::size_t nextReplayEventIndex = 0;
    std::uint64_t replayTick = 0;
    if (options.replayFilePath.has_value())
    {
        replayScript = physics_sim::load_replay_script(*options.replayFilePath);
        if (!replayScript.has_value())
        {
            logger.log("fatal replay script load failed: " + options.replayFilePath->string());
            return 1;
        }

        std::ostringstream stream;
        stream << "replay script loaded: " << options.replayFilePath->string()
               << " events=" << replayScript->events.size();
        logger.log(stream.str());
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        show_error("SDL_Init failed", SDL_GetError(), &logger);
        return 1;
    }
    logger.log("SDL_Init ok");

    const fs::path settingsPath = options.settingsFilePath.value_or(default_settings_path().value_or(fs::path{"physics-sim-settings.txt"}));
    physics_sim::UserSettings userSettings = physics_sim::load_user_settings_or_default(settingsPath);
    if (options.initialWindowSize.has_value())
    {
        userSettings.window_size = *options.initialWindowSize;
    }
    if (options.initialHelpOverlayVisible.has_value())
    {
        userSettings.help_overlay_visible = *options.initialHelpOverlayVisible;
    }
    if (options.initialVisualMode.has_value())
    {
        userSettings.visual_mode = *options.initialVisualMode;
    }

    {
        std::ostringstream stream;
        stream << "settings path=" << settingsPath.string()
               << " window=" << userSettings.window_size.width << 'x' << userSettings.window_size.height
               << " help_overlay=" << (userSettings.help_overlay_visible ? "on" : "off")
               << " visual_mode=" << visual_mode_name(userSettings.visual_mode);
        logger.log(stream.str());
    }

    auto persist_user_settings = [&]()
    {
        if (physics_sim::save_user_settings(settingsPath, userSettings))
        {
            std::ostringstream stream;
            stream << "settings save ok: " << settingsPath.string();
            logger.log(stream.str());
            return true;
        }

        std::ostringstream stream;
        stream << "settings save failed: " << settingsPath.string();
        logger.log(stream.str());
        return false;
    };

    SDL_Window* window = SDL_CreateWindow(
        "Physics Sim",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        userSettings.window_size.width,
        userSettings.window_size.height,
        SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        show_error("SDL_CreateWindow failed", SDL_GetError(), &logger);
        SDL_Quit();
        return 1;
    }

    std::string rendererMode = "accelerated+vsync";
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer != nullptr)
        {
            rendererMode = "accelerated";
        }
    }

    if (renderer == nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (renderer != nullptr)
        {
            rendererMode = "software";
        }
    }

    if (renderer == nullptr)
    {
        show_error("SDL_CreateRenderer failed", SDL_GetError(), &logger);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    {
        std::ostringstream stream;
        stream << "renderer created: " << rendererMode;
        logger.log(stream.str());
    }

    bool running = true;
    int windowWidth = userSettings.window_size.width;
    int windowHeight = userSettings.window_size.height;
    physics_sim::Vec2 mouseScreen{windowWidth * 0.5f, windowHeight * 0.5f};
    physics_sim::FixedStepDriver stepDriver;
    physics_sim::SimulationState simulationState;
    physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
    physics_sim::SceneController controller{simulation};
    physics_sim::SceneViewport viewport;
    physics_sim::SceneMetadata sceneMetadata;
    fs::path currentScenePath = startupScenePath;
    std::optional<std::size_t> currentGalleryIndex = gallery_index_for_path(currentScenePath);
    viewport.set_world_size({1280.0f, 720.0f});
    viewport.set_window_size(windowWidth, windowHeight);
    static_cast<void>(restore_demo_scene(startupScenePath, simulation, viewport, simulationState, stepDriver, controller, &sceneMetadata, &logger));
    currentGalleryIndex = gallery_index_for_path(currentScenePath);

    bool panning = false;
    bool painting = false;
    bool frameDumped = false;
    VisualMode visualMode = userSettings.visual_mode;
    bool showHelp = userSettings.help_overlay_visible;
    const bool captureByTickCount = options.dumpFrameAfterTicks.has_value();
    const bool cleanCaptureFrame = captureByTickCount;
    const bool sessionShellBypassed = options.skipSessionShell
        || options.replayFilePath.has_value()
        || options.dumpFramePath.has_value()
        || captureByTickCount;
    physics_sim::SessionShellState sessionShellState;
    if (sessionShellBypassed)
    {
        sessionShellState.screen = physics_sim::SessionShellScreen::Playing;
        stepDriver.set_paused(false);
    }
    else
    {
        stepDriver.set_paused(true);
    }
    std::optional<std::string> statusMessage;
    Clock::time_point statusMessageExpiresAt{};

    const auto set_status_message = [&](std::string message)
    {
        statusMessage = std::move(message);
        statusMessageExpiresAt = Clock::now() + std::chrono::seconds{2};
    };

    const auto current_status_message = [&]() -> const char*
    {
        if (!statusMessage.has_value())
        {
            return nullptr;
        }

        if (Clock::now() > statusMessageExpiresAt)
        {
            statusMessage.reset();
            return nullptr;
        }

        return statusMessage->c_str();
    };

    const auto load_gallery_scene = [&](std::size_t index) -> bool
    {
        if (index >= galleryScenePaths.size())
        {
            return false;
        }

        const fs::path& path = galleryScenePaths[index];
        if (!load_scene_from_file(path, simulation, viewport, simulationState, stepDriver, controller, &sceneMetadata, &logger))
        {
            set_status_message("LOAD FAILED");
            return false;
        }

        currentScenePath = path;
        currentGalleryIndex = index;
        set_status_message(std::string{"LOADED "} + scene_label(path, sceneMetadata));
        return true;
    };

    const auto save_autosave_scene = [&]() -> bool
    {
        auto snapshot = physics_sim::capture_scene(simulation, sceneMetadata);
        if (physics_sim::save_scene(autosaveScenePath, snapshot))
        {
            std::ostringstream stream;
            stream << "scene save ok: " << autosaveScenePath.string();
            logger.log(stream.str());
            set_status_message("SAVED AUTOSAVE");
            return true;
        }

        std::ostringstream stream;
        stream << "scene save failed: " << autosaveScenePath.string();
        logger.log(stream.str());
        set_status_message("SAVE FAILED");
        return false;
    };

    const auto load_autosave_or_demo = [&]() -> bool
    {
        if (!load_scene_from_file(autosaveScenePath, simulation, viewport, simulationState, stepDriver, controller, &sceneMetadata, &logger))
        {
            static_cast<void>(restore_demo_scene(demoScenePath, simulation, viewport, simulationState, stepDriver, controller, &sceneMetadata, &logger));
            currentScenePath = demoScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            set_status_message("LOADED DEMO");
            return false;
        }

        currentScenePath = autosaveScenePath;
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
        set_status_message("LOADED AUTOSAVE");
        return true;
    };

    const auto clear_current_scene = [&]()
    {
        controller.reset_scene();
        simulationState.reset();
        stepDriver.reset();
        sceneMetadata = {};
        set_status_message("CLEARED SCENE");
    };

    auto process_replay_actions = [&](std::uint64_t current_replay_tick) -> bool
    {
        if (!replayScript.has_value())
        {
            return true;
        }

        while (nextReplayEventIndex < replayScript->events.size()
            && replayScript->events[nextReplayEventIndex].tick <= current_replay_tick)
        {
            const physics_sim::ReplayEvent event = replayScript->events[nextReplayEventIndex++];
            std::ostringstream stream;
            stream << "replay action: tick=" << current_replay_tick
                   << " scheduled=" << event.tick
                   << " command=" << event.command;
            for (const auto& argument : event.arguments)
            {
                stream << ' ' << argument;
            }
            logger.log(stream.str());

            if (event.command == "action")
            {
                const auto action = physics_sim::action_from_replay_token(event.arguments.front());
                if (!action.has_value())
                {
                    logger.log("fatal replay action failed to parse: " + event.arguments.front());
                    running = false;
                    return false;
                }

                if (*action == physics_sim::Action::Quit)
                {
                    running = false;
                    return false;
                }

                std::string actionFeedback;
                apply_action(*action, stepDriver, simulationState, controller, simulation, viewport, currentScenePath, &sceneMetadata, &actionFeedback, &logger);
                if (!actionFeedback.empty())
                {
                    set_status_message(std::move(actionFeedback));
                }
                continue;
            }

            if (event.command == "tool")
            {
                const auto tool = parse_replay_tool(event.arguments.front());
                if (!tool.has_value())
                {
                    logger.log("fatal replay tool failed to parse: " + event.arguments.front());
                    running = false;
                    return false;
                }

                controller.set_tool(*tool);
                continue;
            }

            if (event.command == "direction")
            {
                const auto x = parse_float_token(event.arguments[0]);
                const auto y = parse_float_token(event.arguments[1]);
                if (!x.has_value() || !y.has_value())
                {
                    logger.log("fatal replay direction failed to parse");
                    running = false;
                    return false;
                }

                if (controller.has_selected_fixture())
                {
                    static_cast<void>(controller.set_selected_fixture_direction({*x, *y}));
                }
                else
                {
                    controller.set_emitter_direction({*x, *y});
                }
                continue;
            }

            if (event.command == "speed")
            {
                const auto speed = parse_float_token(event.arguments[0]);
                if (!speed.has_value())
                {
                    logger.log("fatal replay speed failed to parse");
                    running = false;
                    return false;
                }

                if (controller.has_selected_fixture())
                {
                    static_cast<void>(controller.set_selected_fixture_speed(*speed));
                }
                else
                {
                    controller.set_emitter_speed(*speed);
                }
                continue;
            }

            if (event.command == "rate")
            {
                const auto rate = parse_float_token(event.arguments[0]);
                if (!rate.has_value())
                {
                    logger.log("fatal replay rate failed to parse");
                    running = false;
                    return false;
                }

                if (controller.has_selected_fixture())
                {
                    static_cast<void>(controller.set_selected_fixture_emission_rate(*rate));
                }
                else
                {
                    controller.set_emission_rate(*rate);
                }
                continue;
            }

            if (event.command == "place")
            {
                const auto x = parse_float_token(event.arguments[0]);
                const auto y = parse_float_token(event.arguments[1]);
                if (!x.has_value() || !y.has_value())
                {
                    logger.log("fatal replay place failed to parse");
                    running = false;
                    return false;
                }

                if (!controller.place_fixture({*x, *y}))
                {
                    logger.log("fatal replay place invalid placement");
                    running = false;
                    return false;
                }
                continue;
            }

            logger.log("fatal replay command not recognized: " + event.command);
            running = false;
            return false;
        }

        return true;
    };

    auto advance_simulation_step = [&](physics_sim::FixedStepDriver::duration step) -> bool
    {
        simulationState.advance(step);
        simulation.step(step.count());
        ++replayTick;
        return process_replay_actions(replayTick);
    };

    const auto update_shell_pause_state = [&]()
    {
        stepDriver.set_paused(sessionShellState.screen != physics_sim::SessionShellScreen::Playing);
    };

    const auto open_pause_menu = [&]()
    {
        sessionShellState.screen = physics_sim::SessionShellScreen::PauseMenu;
        sessionShellState.return_screen = physics_sim::SessionShellScreen::Playing;
        sessionShellState.selection = 0;
        update_shell_pause_state();
        panning = false;
        painting = false;
    };

    const auto execute_session_shell_command = [&](physics_sim::SessionShellCommand command, physics_sim::SessionShellState previous_shell_state) -> bool
    {
        switch (command.kind)
        {
        case physics_sim::SessionShellCommandKind::None:
            return false;
        case physics_sim::SessionShellCommandKind::Quit:
            running = false;
            return true;
        case physics_sim::SessionShellCommandKind::StartPlaying:
        case physics_sim::SessionShellCommandKind::Resume:
            panning = false;
            painting = false;
            break;
        case physics_sim::SessionShellCommandKind::RetryCurrentScene:
        {
            std::string actionFeedback;
            apply_action(
                physics_sim::Action::Reset,
                stepDriver,
                simulationState,
                controller,
                simulation,
                viewport,
                currentScenePath,
                &sceneMetadata,
                &actionFeedback,
                &logger);
            if (!actionFeedback.empty())
            {
                set_status_message(std::move(actionFeedback));
            }
            break;
        }
        case physics_sim::SessionShellCommandKind::ResetFluid:
        {
            std::string actionFeedback;
            apply_action(
                physics_sim::Action::ResetFluid,
                stepDriver,
                simulationState,
                controller,
                simulation,
                viewport,
                currentScenePath,
                &sceneMetadata,
                &actionFeedback,
                &logger);
            if (!actionFeedback.empty())
            {
                set_status_message(std::move(actionFeedback));
            }
            break;
        }
        case physics_sim::SessionShellCommandKind::ClearScene:
            clear_current_scene();
            break;
        case physics_sim::SessionShellCommandKind::SaveScene:
            static_cast<void>(save_autosave_scene());
            break;
        case physics_sim::SessionShellCommandKind::LoadScene:
            static_cast<void>(load_autosave_or_demo());
            break;
        case physics_sim::SessionShellCommandKind::ReturnToMainMenu:
        case physics_sim::SessionShellCommandKind::OpenSceneBrowser:
        case physics_sim::SessionShellCommandKind::OpenSettings:
        case physics_sim::SessionShellCommandKind::OpenAbout:
            panning = false;
            painting = false;
            break;
        case physics_sim::SessionShellCommandKind::LoadSceneAtIndex:
            if (!command.scene_index.has_value() || !load_gallery_scene(*command.scene_index))
            {
                sessionShellState = previous_shell_state;
                set_status_message("LOAD FAILED");
                update_shell_pause_state();
                return true;
            }
            break;
        case physics_sim::SessionShellCommandKind::ToggleHelpOverlay:
            showHelp = !showHelp;
            userSettings.help_overlay_visible = showHelp;
            static_cast<void>(persist_user_settings());
            break;
        case physics_sim::SessionShellCommandKind::CycleVisualMode:
            visualMode = next_visual_mode(visualMode);
            userSettings.visual_mode = visualMode;
            static_cast<void>(persist_user_settings());
            break;
        case physics_sim::SessionShellCommandKind::Back:
            break;
        }

        update_shell_pause_state();
        return true;
    };

    const auto handle_session_shell_keyboard = [&](SDL_Keycode keycode) -> bool
    {
        if (sessionShellBypassed)
        {
            return false;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::Playing)
        {
            if (keycode == SDLK_ESCAPE)
            {
                open_pause_menu();
                return true;
            }

            return false;
        }

        if (keycode == SDLK_ESCAPE)
        {
            physics_sim::session_shell_back(sessionShellState);
            update_shell_pause_state();
            panning = false;
            painting = false;
            return true;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::PauseMenu && keycode == SDLK_SPACE)
        {
            sessionShellState.screen = physics_sim::SessionShellScreen::Playing;
            sessionShellState.selection = 0;
            update_shell_pause_state();
            panning = false;
            painting = false;
            return true;
        }

        const std::size_t optionCount = physics_sim::session_shell_option_count(sessionShellState.screen, galleryScenePaths.size());
        if (optionCount == 0)
        {
            return true;
        }

        if (keycode == SDLK_UP || keycode == SDLK_w)
        {
            physics_sim::session_shell_wrap_selection(sessionShellState, optionCount, -1);
            return true;
        }

        if (keycode == SDLK_DOWN || keycode == SDLK_s)
        {
            physics_sim::session_shell_wrap_selection(sessionShellState, optionCount, 1);
            return true;
        }

        if (keycode != SDLK_RETURN && keycode != SDLK_KP_ENTER)
        {
            return true;
        }

        const auto previousShellState = sessionShellState;
        const auto command = physics_sim::session_shell_activate(sessionShellState, galleryScenePaths.size());
        static_cast<void>(execute_session_shell_command(command, previousShellState));
        return true;
    };

    const auto handle_session_shell_mouse_move = [&](int mouse_x, int mouse_y) -> bool
    {
        if (sessionShellBypassed)
        {
            return false;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::Playing)
        {
            return false;
        }

        const auto hovered_index = session_shell_option_index_at_point(mouse_x, mouse_y, windowWidth, windowHeight, sessionShellState, galleryScenePaths);
        if (hovered_index.has_value())
        {
            sessionShellState.selection = *hovered_index;
        }

        return true;
    };

    const auto handle_session_shell_mouse_click = [&](int mouse_x, int mouse_y, Uint8 button) -> bool
    {
        if (sessionShellBypassed)
        {
            return false;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::Playing)
        {
            return false;
        }

        if (button != SDL_BUTTON_LEFT)
        {
            if (button == SDL_BUTTON_RIGHT)
            {
                physics_sim::session_shell_back(sessionShellState);
                update_shell_pause_state();
                panning = false;
                painting = false;
                return true;
            }

            return true;
        }

        const auto hovered_index = session_shell_option_index_at_point(mouse_x, mouse_y, windowWidth, windowHeight, sessionShellState, galleryScenePaths);
        if (!hovered_index.has_value())
        {
            return true;
        }

        sessionShellState.selection = *hovered_index;
        const auto previousShellState = sessionShellState;
        const auto command = physics_sim::session_shell_activate(sessionShellState, galleryScenePaths.size());
        static_cast<void>(execute_session_shell_command(command, previousShellState));
        return true;
    };

    if (!process_replay_actions(replayTick))
    {
        running = false;
    }

    const auto appStart = Clock::now();
    auto lastFrameTime = appStart;
    std::uint64_t frameCount = 0;
    physics_sim::FixedStepDriver::AdvanceResult lastUpdate;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
            {
                if (event.key.repeat != 0)
                {
                    break;
                }

                if (handle_session_shell_keyboard(event.key.keysym.sym))
                {
                    break;
                }

                if (const auto action = physics_sim::action_from_keycode(event.key.keysym.sym))
                {
                    if (*action == physics_sim::Action::Quit)
                    {
                        running = false;
                    }
                    else
                    {
                        std::string actionFeedback;
                        apply_action(*action, stepDriver, simulationState, controller, simulation, viewport, currentScenePath, &sceneMetadata, &actionFeedback, &logger);
                        if (!actionFeedback.empty())
                        {
                            set_status_message(std::move(actionFeedback));
                        }
                    }
                    break;
                }

                const SDL_Keymod modifiers = static_cast<SDL_Keymod>(event.key.keysym.mod);
                if (event.key.keysym.sym == SDLK_TAB)
                {
                    const int direction = (modifiers & KMOD_SHIFT) != 0 ? -1 : 1;
                    controller.set_tool(physics_sim::next_scene_tool(controller.tool(), direction));
                    set_status_message(std::string{"TOOL "} + tool_name(controller.tool()));
                    break;
                }
                if (event.key.keysym.sym == SDLK_z && (modifiers & KMOD_CTRL) != 0)
                {
                    if ((modifiers & KMOD_SHIFT) != 0)
                    {
                        static_cast<void>(controller.redo_scene_edit());
                    }
                    else
                    {
                        static_cast<void>(controller.undo_scene_edit());
                    }
                    break;
                }

                if (event.key.keysym.sym == SDLK_y && (modifiers & KMOD_CTRL) != 0)
                {
                    static_cast<void>(controller.redo_scene_edit());
                    break;
                }

                switch (event.key.keysym.sym)
                {
                case SDLK_F5:
                    static_cast<void>(save_autosave_scene());
                    break;
                case SDLK_F9:
                    static_cast<void>(load_autosave_or_demo());
                    break;
                case SDLK_PAGEUP:
                {
                    if (!galleryScenePaths.empty())
                    {
                        const std::size_t nextIndex = currentGalleryIndex.has_value()
                            ? ((*currentGalleryIndex + galleryScenePaths.size() - 1) % galleryScenePaths.size())
                            : (galleryScenePaths.size() - 1);
                        static_cast<void>(load_gallery_scene(nextIndex));
                    }
                    break;
                }
                case SDLK_PAGEDOWN:
                {
                    if (!galleryScenePaths.empty())
                    {
                        const std::size_t nextIndex = currentGalleryIndex.has_value()
                            ? ((*currentGalleryIndex + 1) % galleryScenePaths.size())
                            : 0;
                        static_cast<void>(load_gallery_scene(nextIndex));
                    }
                    break;
                }
                case SDLK_1:
                    controller.set_tool(physics_sim::SceneTool::PaintWall);
                    set_status_message("TOOL WALL");
                    break;
                case SDLK_2:
                    controller.set_tool(physics_sim::SceneTool::EraseWall);
                    set_status_message("TOOL ERASE");
                    break;
                case SDLK_3:
                    controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
                    set_status_message("TOOL HOSE");
                    break;
                case SDLK_4:
                    controller.set_tool(physics_sim::SceneTool::OmniEmitter);
                    set_status_message("TOOL OMNI");
                    break;
                case SDLK_5:
                    controller.set_tool(physics_sim::SceneTool::Gate);
                    set_status_message("TOOL GATE");
                    break;
                case SDLK_6:
                    controller.set_tool(physics_sim::SceneTool::Sensor);
                    set_status_message("TOOL SENSOR");
                    break;
                case SDLK_7:
                    controller.set_tool(physics_sim::SceneTool::Drain);
                    set_status_message("TOOL DRAIN");
                    break;
                case SDLK_8:
                    controller.set_tool(physics_sim::SceneTool::Pump);
                    set_status_message("TOOL PUMP");
                    break;
                case SDLK_9:
                    controller.set_tool(physics_sim::SceneTool::Valve);
                    set_status_message("TOOL VALVE");
                    break;
                case SDLK_DELETE:
                    if (!controller.delete_selected_fixture())
                    {
                        if (!controller.delete_selected_gate())
                        {
                            if (!controller.delete_selected_valve())
                            {
                            if (!controller.delete_selected_sensor())
                            {
                                clear_current_scene();
                            }
                            else
                            {
                                set_status_message("DELETED SENSOR");
                            }
                            }
                            else
                            {
                                set_status_message("DELETED VALVE");
                            }
                        }
                        else
                        {
                            set_status_message("DELETED GATE");
                        }
                    }
                    else
                    {
                        set_status_message("DELETED FIXTURE");
                    }
                    break;
                case SDLK_v:
                    visualMode = next_visual_mode(visualMode);
                    userSettings.visual_mode = visualMode;
                    static_cast<void>(persist_user_settings());
                    break;
                case SDLK_h:
                    showHelp = !showHelp;
                    userSettings.help_overlay_visible = showHelp;
                    static_cast<void>(persist_user_settings());
                    break;
                default:
                    handle_fixture_edit_key(
                        controller,
                        event.key.keysym.sym,
                        modifiers,
                        simulation.grid().cell_size());
                    break;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                mouseScreen.x = static_cast<float>(event.button.x);
                mouseScreen.y = static_cast<float>(event.button.y);

                if (handle_session_shell_mouse_click(event.button.x, event.button.y, event.button.button))
                {
                    break;
                }

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    const physics_sim::Vec2 world = viewport.window_to_world(mouseScreen);
                    if (controller.tool() == physics_sim::SceneTool::PaintWall || controller.tool() == physics_sim::SceneTool::EraseWall)
                    {
                        controller.begin_stroke(world);
                        painting = true;
                    }
                    else if (controller.tool() == physics_sim::SceneTool::DirectionalEmitter || controller.tool() == physics_sim::SceneTool::OmniEmitter)
                    {
                        if (!controller.select_fixture_at(world, simulation.grid().cell_size() * 0.65f))
                        {
                            if (!controller.place_fixture(world))
                            {
                                set_status_message("INVALID PLACEMENT");
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Gate)
                    {
                        if (!controller.select_gate_at(world, simulation.grid().cell_size() * 0.65f))
                        {
                            if (!controller.place_gate(world))
                            {
                                set_status_message("INVALID PLACEMENT");
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Sensor)
                    {
                        if (!controller.select_sensor_at(world))
                        {
                            if (!controller.place_sensor(world))
                            {
                                set_status_message("INVALID PLACEMENT");
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Drain)
                    {
                        if (!controller.place_drain(world))
                        {
                            set_status_message("INVALID PLACEMENT");
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Pump)
                    {
                        if (!controller.place_pump(world))
                        {
                            set_status_message("INVALID PLACEMENT");
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Valve)
                    {
                        if (!controller.select_valve_at(world))
                        {
                            if (!controller.place_valve(world))
                            {
                                set_status_message("INVALID PLACEMENT");
                            }
                        }
                    }
                    else
                    {
                        controller.place_fixture(world);
                    }
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    controller.clear_selection();
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                mouseScreen.x = static_cast<float>(event.button.x);
                mouseScreen.y = static_cast<float>(event.button.y);

                if (sessionShellState.screen != physics_sim::SessionShellScreen::Playing)
                {
                    break;
                }

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (painting)
                    {
                        controller.end_stroke(viewport.window_to_world(mouseScreen));
                        painting = false;
                    }
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = false;
                }
                break;
            case SDL_MOUSEMOTION:
            {
                const physics_sim::Vec2 previousScreen = mouseScreen;
                mouseScreen.x = static_cast<float>(event.motion.x);
                mouseScreen.y = static_cast<float>(event.motion.y);

                if (handle_session_shell_mouse_move(event.motion.x, event.motion.y))
                {
                    break;
                }

                if (panning)
                {
                    viewport.pan_pixels({static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)});
                }

                if (painting)
                {
                    controller.drag_stroke(viewport.window_to_world(mouseScreen));
                }

                (void)previousScreen;
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                if (sessionShellState.screen != physics_sim::SessionShellScreen::Playing)
                {
                    break;
                }

                float wheel = static_cast<float>(event.wheel.y);
                if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                {
                    wheel = -wheel;
                }

                if (wheel != 0.0f)
                {
                    const float factor = wheel > 0.0f ? 1.12f : 1.0f / 1.12f;
                    viewport.zoom_at(std::pow(factor, std::abs(wheel)), mouseScreen);
                }
                break;
            }
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                    userSettings.window_size.width = windowWidth;
                    userSettings.window_size.height = windowHeight;
                    viewport.set_window_size(windowWidth, windowHeight);
                    static_cast<void>(persist_user_settings());
                }
                break;
            default:
                break;
            }
        }

        if (captureByTickCount && !frameDumped && simulationState.tick_count < *options.dumpFrameAfterTicks)
        {
            while (running && simulationState.tick_count < *options.dumpFrameAfterTicks)
            {
                stepDriver.request_step();
                lastUpdate = stepDriver.advance(physics_sim::FixedStepDriver::duration{0}, [&](physics_sim::FixedStepDriver::duration step)
                {
                    static_cast<void>(advance_simulation_step(step));
                });
                if (!running)
                {
                    break;
                }
            }
        }

        const auto now = Clock::now();
        const auto frameDelta = std::chrono::duration_cast<physics_sim::FixedStepDriver::duration>(now - lastFrameTime);
        lastFrameTime = now;
        const auto realElapsed = std::chrono::duration<double>(now - appStart);

        if (options.autoExitAfter && (!captureByTickCount || frameDumped) && std::chrono::duration_cast<std::chrono::milliseconds>(realElapsed) >= *options.autoExitAfter)
        {
            running = false;
        }

        ++frameCount;

        if (!(captureByTickCount && !frameDumped && simulationState.tick_count >= *options.dumpFrameAfterTicks))
        {
            lastUpdate = stepDriver.advance(frameDelta, [&](physics_sim::FixedStepDriver::duration step)
            {
                static_cast<void>(advance_simulation_step(step));
            });
        }

        SDL_SetRenderDrawColor(renderer, 14, 18, 28, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer, viewport, simulation);
        if (visualMode != VisualMode::Particles)
        {
            draw_fluid_density(renderer, viewport, simulation);
        }
        draw_walls(renderer, viewport, simulation);
        if (visualMode != VisualMode::Density)
        {
            draw_particles(renderer, viewport, simulation);
        }
        draw_drains(renderer, viewport, simulation);
        draw_pumps(renderer, viewport, simulation);
        draw_gates(renderer, viewport, simulation, controller);
        draw_valves(renderer, viewport, simulation, controller);
        draw_sensors(renderer, viewport, simulation, controller);
        draw_emitters(renderer, viewport, simulation, controller);
        const bool sessionShellVisible = sessionShellState.screen != physics_sim::SessionShellScreen::Playing;
        if (!sessionShellVisible)
        {
            draw_tool_preview(renderer, viewport, simulation, controller, viewport.window_to_world(mouseScreen));
            draw_crosshair(renderer, static_cast<int>(mouseScreen.x), static_cast<int>(mouseScreen.y));
        }
        const double averageFps = realElapsed.count() > 0.0 ? static_cast<double>(frameCount) / realElapsed.count() : 0.0;
        if (!cleanCaptureFrame && !sessionShellVisible)
        {
            physics_sim::DebugOverlayMetrics overlayMetrics;
            overlayMetrics.fps = averageFps;
            overlayMetrics.driver = &stepDriver;
            overlayMetrics.state = &simulationState;
            overlayMetrics.simulation = &simulation;
            overlayMetrics.controller = &controller;
            overlayMetrics.visual_mode = visual_mode_name(visualMode);
            overlayMetrics.status_message = current_status_message();
            draw_debug_overlay(renderer, overlayMetrics, viewport.scale());
        }

        if (showHelp && !sessionShellVisible)
        {
            draw_help_overlay(renderer, windowWidth, windowHeight);
        }

        if (sessionShellVisible)
        {
            draw_session_shell(renderer, windowWidth, windowHeight, sessionShellState, galleryScenePaths);
        }

        SDL_SetWindowTitle(
            window,
            build_window_title(
                simulationState,
                stepDriver,
                lastUpdate,
                averageFps,
                simulation,
                visual_mode_name(visualMode),
                currentScenePath,
                sceneMetadata,
                controller,
                viewport,
                &sessionShellState).c_str());

        SDL_RenderPresent(renderer);

        if (!frameDumped && options.dumpFramePath.has_value())
        {
            const bool ready_to_dump = captureByTickCount
                ? simulationState.tick_count >= *options.dumpFrameAfterTicks
                : !options.dumpFrameAfter.has_value() || realElapsed >= *options.dumpFrameAfter;
            if (ready_to_dump)
            {
                static_cast<void>(save_frame(renderer, *options.dumpFramePath, windowWidth, windowHeight, &logger));
                frameDumped = true;
                if (captureByTickCount)
                {
                    running = false;
                }
            }
        }
    }

    static_cast<void>(persist_user_settings());
    logger.log("shutdown: normal exit");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#if defined(_WIN32)
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main(__argc, __argv);
}
#endif
