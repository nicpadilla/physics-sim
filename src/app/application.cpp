#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <physics_sim/application.hpp>

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
#include <limits>
#include <optional>
#include <span>
#include <system_error>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <physics_sim/action.hpp>
#include <physics_sim/audio_feedback.hpp>
#include <physics_sim/challenge_objective.hpp>
#include <physics_sim/debug_overlay.hpp>
#include <physics_sim/feedback.hpp>
#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/gallery_manifest.hpp>
#include <physics_sim/input_bindings.hpp>
#include <physics_sim/math.hpp>
#include <physics_sim/mode_switch.hpp>
#include <physics_sim/player_guidance.hpp>
#include <physics_sim/player_feedback.hpp>
#include <physics_sim/replay_script.hpp>
#include <physics_sim/save_browser.hpp>
#include <physics_sim/settings_menu.hpp>
#include <physics_sim/session_shell.hpp>
#include <physics_sim/tutorial_progress.hpp>
#include <physics_sim/scene_document.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/scene_viewport.hpp>
#include <physics_sim/ui_palette.hpp>
#include <physics_sim/user_settings.hpp>
#include <physics_sim/simulation_state.hpp>
#include <physics_sim/surface_reconstruction.hpp>
#include <physics_sim/visual_mode.hpp>
#include <physics_sim/water_simulation.hpp>
#include <physics_sim/water_visual_effects.hpp>

namespace
{
using Clock = std::chrono::steady_clock;
namespace fs = std::filesystem;
using physics_sim::VisualMode;
using physics_sim::next_visual_mode;
using physics_sim::next_solver_profile;
using physics_sim::parse_solver_profile_token;
using physics_sim::solver_profile_name;
using physics_sim::visual_mode_name;

[[nodiscard]] fs::path executable_base_path()
{
    char* base_path = SDL_GetBasePath();
    if (base_path == nullptr)
    {
        return {};
    }
    fs::path result{base_path};
    SDL_free(base_path);
    return result;
}

[[nodiscard]] fs::path resolve_runtime_resource(const fs::path& path, const fs::path& executable_base)
{
    if (path.is_absolute() || fs::exists(path) || executable_base.empty())
    {
        return path;
    }
    const fs::path packaged_path = executable_base / path;
    return fs::exists(packaged_path) ? packaged_path : path;
}

struct RuntimeOptions
{
    std::optional<std::chrono::milliseconds> autoExitAfter;
    std::optional<fs::path> dumpFramePath;
    std::optional<std::chrono::milliseconds> dumpFrameAfter;
    std::optional<std::uint64_t> dumpFrameAfterTicks;
    std::optional<physics_sim::WindowSize> initialWindowSize;
    std::optional<bool> initialHelpOverlayVisible;
    std::optional<bool> initialReducedMotion;
    std::optional<fs::path> logFilePath;
    std::optional<fs::path> replayFilePath;
    std::optional<fs::path> settingsFilePath;
    std::optional<fs::path> startupScenePath;
    std::optional<VisualMode> initialVisualMode;
    std::optional<physics_sim::FluidSolverProfile> initialSolverProfile;
    bool disableAudio = false;
    bool debugOverlay = false;
    bool tutorialMode = false;
    bool skipSessionShell = false;
    bool advancedTools = false;
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

[[nodiscard]] std::optional<fs::path> default_pref_root()
{
    char* pref_path = SDL_GetPrefPath("Nic", "physics-sim");
    if (pref_path == nullptr)
    {
        return std::nullopt;
    }

    fs::path path = fs::path{pref_path};
    SDL_free(pref_path);
    return path;
}

[[nodiscard]] std::optional<fs::path> default_settings_path()
{
    const auto root = default_pref_root();
    if (!root.has_value())
    {
        return std::nullopt;
    }

    return *root / "physics-sim-settings.txt";
}

[[nodiscard]] std::optional<fs::path> default_tutorial_state_path()
{
    const auto root = default_pref_root();
    if (!root.has_value())
    {
        return std::nullopt;
    }

    return *root / "physics-sim-tutorial-state.txt";
}

[[nodiscard]] std::optional<fs::path> default_save_directory()
{
    const auto root = default_pref_root();
    if (!root.has_value())
    {
        return std::nullopt;
    }

    return *root / "saves";
}

[[nodiscard]] std::optional<bool> load_tutorial_seen_state(const fs::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::istringstream stream{buffer.str()};

    std::string header;
    int version = 0;
    if (!(stream >> header >> version) || header != "physics-sim-tutorial-state" || version != 1)
    {
        return std::nullopt;
    }

    std::string keyword;
    int seen_value = 0;
    if (!(stream >> keyword >> seen_value) || keyword != "seen")
    {
        return std::nullopt;
    }

    return seen_value != 0;
}

[[nodiscard]] bool save_tutorial_seen_state(const fs::path& path, bool seen)
{
    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        static_cast<void>(std::filesystem::create_directories(parent, ec));
    }

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }

    file << "physics-sim-tutorial-state 1\n";
    file << "seen " << (seen ? 1 : 0) << "\n";
    file.flush();
    return static_cast<bool>(file);
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

    if (value == "surface")
    {
        return VisualMode::Surface;
    }

    return std::nullopt;
}

std::optional<physics_sim::SceneTool> parse_replay_tool(std::string_view value)
{
    if (value == "pointer-water" || value == "pour" || value == "water")
    {
        return physics_sim::SceneTool::PointerWater;
    }

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
        else if (arg == "--solver-profile" && i + 1 < argc)
        {
            options.initialSolverProfile = parse_solver_profile_token(argv[++i]);
        }
        else if (arg.starts_with("--solver-profile="))
        {
            options.initialSolverProfile = parse_solver_profile_token(arg.substr(std::string_view("--solver-profile=").size()));
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
        else if (arg == "--advanced-tools")
        {
            options.advancedTools = true;
        }
        else if (arg == "--reduced-motion")
        {
            options.initialReducedMotion = true;
        }
        else if (arg.starts_with("--reduced-motion="))
        {
            options.initialReducedMotion = physics_sim::parse_bool_token(
                arg.substr(std::string_view("--reduced-motion=").size()));
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
        else if (arg == "--tutorial-mode")
        {
            options.tutorialMode = true;
        }
        else if (arg == "--disable-audio")
        {
            options.disableAudio = true;
        }
        else if (arg == "--debug-overlay")
        {
            options.debugOverlay = true;
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
    case physics_sim::SceneTool::PointerWater:
        return "pour";
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

physics_sim::SceneTool next_sandbox_tool(physics_sim::SceneTool current, int direction, bool advanced) noexcept
{
    constexpr std::array compact_tools{
        physics_sim::SceneTool::PointerWater,
        physics_sim::SceneTool::PaintWall,
        physics_sim::SceneTool::EraseWall};
    constexpr std::array all_tools{
        physics_sim::SceneTool::PointerWater, physics_sim::SceneTool::PaintWall, physics_sim::SceneTool::EraseWall,
        physics_sim::SceneTool::DirectionalEmitter, physics_sim::SceneTool::OmniEmitter, physics_sim::SceneTool::Gate,
        physics_sim::SceneTool::Sensor, physics_sim::SceneTool::Drain, physics_sim::SceneTool::Pump, physics_sim::SceneTool::Valve};
    const std::span<const physics_sim::SceneTool> tools = advanced
        ? std::span<const physics_sim::SceneTool>{all_tools}
        : std::span<const physics_sim::SceneTool>{compact_tools};
    auto iterator = std::find(tools.begin(), tools.end(), current);
    const int current_index = iterator == tools.end() ? 0 : static_cast<int>(std::distance(tools.begin(), iterator));
    const int step = direction < 0 ? -1 : 1;
    return tools[static_cast<std::size_t>((current_index + step + static_cast<int>(tools.size())) % static_cast<int>(tools.size()))];
}

void draw_tool_palette(
    SDL_Renderer* renderer,
    const physics_sim::SceneController& controller,
    bool advanced,
    const physics_sim::UiPalette& palette)
{
    constexpr int x = 12;
    constexpr int y = 12;
    constexpr int width = 188;
    constexpr int row_height = 24;
    constexpr int header_height = 48;
    const int tool_count = advanced ? 10 : 3;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect background{x, y, width, header_height + (tool_count + 1) * row_height + 8};
    SDL_SetRenderDrawColor(renderer, 6, 10, 18, 220);
    SDL_RenderFillRect(renderer, &background);
    SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, 255);
    SDL_RenderDrawRect(renderer, &background);
    physics_sim::draw_text(renderer, x + 8, y + 7, 2, advanced ? "TOOLS - A COMPACT" : "TOOLS - A ADVANCED");
    for (int index = 0; index < tool_count; ++index)
    {
        const auto tool = static_cast<physics_sim::SceneTool>(index);
        SDL_Rect row{x + 5, y + header_height + index * row_height, width - 10, row_height - 2};
        if (controller.tool() == tool)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, 105);
            SDL_RenderFillRect(renderer, &row);
        }
        SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, 255);
        physics_sim::draw_text(renderer, row.x + 5, row.y + 4, 2, std::to_string(index) + " " + tool_name(tool));
    }
    SDL_SetRenderDrawColor(renderer, palette.muted_text.r, palette.muted_text.g, palette.muted_text.b, 255);
    physics_sim::draw_text(renderer, x + 8, y + header_height + tool_count * row_height + 4, 1, "TAB CYCLE  T TOGGLE  DEL REMOVE");
}

[[nodiscard]] std::optional<physics_sim::SceneTool> tool_palette_hit(int mouse_x, int mouse_y, bool advanced) noexcept
{
    constexpr int x = 12;
    constexpr int y = 12;
    constexpr int width = 188;
    constexpr int row_height = 24;
    constexpr int header_height = 48;
    const int tool_count = advanced ? 10 : 3;
    if (mouse_x < x + 5 || mouse_x >= x + width - 5)
    {
        return std::nullopt;
    }
    const int index = (mouse_y - (y + header_height)) / row_height;
    if (mouse_y < y + header_height || index < 0 || index >= tool_count)
    {
        return std::nullopt;
    }
    return static_cast<physics_sim::SceneTool>(index);
}

[[nodiscard]] bool tool_palette_toggle_hit(int mouse_x, int mouse_y) noexcept
{
    return mouse_x >= 12 && mouse_x < 200 && mouse_y >= 0 && mouse_y < 60;
}

void draw_challenge_overlay(
    SDL_Renderer* renderer,
    int window_width,
    const std::optional<physics_sim::SceneChallenge>& challenge,
    const physics_sim::ChallengeProgress& progress,
    const physics_sim::UiPalette& palette)
{
    if (!challenge.has_value()) return;
    constexpr int width = 350;
    constexpr int height = 118;
    const int x = std::max(12, window_width - width - 12);
    const int y = 12;
    SDL_Rect panel{x, y, width, height};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 6, 10, 18, 225);
    SDL_RenderFillRect(renderer, &panel);
    const auto& border = progress.status == physics_sim::ChallengeStatus::Complete ? palette.success
        : progress.status == physics_sim::ChallengeStatus::Failed ? palette.error : palette.objective;
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, 255);
    SDL_RenderDrawRect(renderer, &panel);
    physics_sim::draw_text(renderer, x + 10, y + 8, 2, "CHALLENGE: " + challenge->title);
    const std::uint64_t held = std::min(progress.held_ticks, challenge->hold_ticks);
    physics_sim::draw_text(renderer, x + 10, y + 34, 2,
        "TARGETS " + std::to_string(challenge->required_objective_sensors) + "  HOLD "
        + std::to_string(held) + "/" + std::to_string(challenge->hold_ticks));
    std::string status = "RUNNING - F10 RESTART";
    if (progress.status == physics_sim::ChallengeStatus::Complete) status = "COMPLETE - F10 REPLAY";
    if (progress.status == physics_sim::ChallengeStatus::Failed) status = std::string{"FAILED: "} + progress.failure_reason;
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, 255);
    physics_sim::draw_text(renderer, x + 10, y + 60, 2, status);
    std::string budgets = "BUDGET ";
    budgets += challenge->maximum_emitted_mass ? "EMIT " + std::to_string(static_cast<int>(*challenge->maximum_emitted_mass)) : "EMIT OPEN";
    budgets += challenge->maximum_outflow_mass ? "  LOSS " + std::to_string(static_cast<int>(*challenge->maximum_outflow_mass)) : "  LOSS OPEN";
    SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, 255);
    physics_sim::draw_text(renderer, x + 10, y + 87, 1, budgets);
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

void draw_grid(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::UiPalette& palette)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, palette.grid.r, palette.grid.g, palette.grid.b, palette.grid.a);

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

void draw_walls(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::UiPalette& palette)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, palette.wall.r, palette.wall.g, palette.wall.b, palette.wall.a);
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

void draw_particles(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::UiPalette& palette)
{
    const float cell_size = simulation.grid().cell_size();
    const float particle_size = std::max(8.0f, cell_size * 0.6f);

    SDL_SetRenderDrawColor(renderer, palette.water.r, palette.water.g, palette.water.b, palette.water.a);
    for (const auto& particle : simulation.particles())
    {
        const SDL_FRect rect = world_rect(
            viewport,
            {particle.position.x - particle_size * 0.5f, particle.position.y - particle_size * 0.5f},
            {particle_size, particle_size});
        draw_filled_circle(renderer, rect);
    }
}

void draw_fluid_density(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::UiPalette& palette)
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
            SDL_SetRenderDrawColor(renderer, palette.water_density.r, palette.water_density.g, palette.water_density.b, alpha);
            const SDL_FRect rect = world_rect(
                viewport,
                {static_cast<float>(x) * grid.cell_size(), static_cast<float>(y) * grid.cell_size()},
                {grid.cell_size(), grid.cell_size()});
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}

void draw_fluid_surface(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::UiPalette& palette,
    float interpolation_alpha,
    bool reduced_motion)
{
    const auto& grid = simulation.grid();
    if (grid.width() == 0 || grid.height() == 0)
    {
        return;
    }

    std::vector<std::uint8_t> solids(grid.cell_count(), 0);
    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            const std::size_t index = y * grid.width() + x;
            solids[index] = grid.solid(x, y) ? 1 : 0;
        }
    }
    physics_sim::ParticleSurfaceField field = physics_sim::build_particle_surface_field(
        simulation.particles(), solids, grid.width(), grid.height(), grid.cell_size(), 4);

    struct SurfaceHistory
    {
        std::size_t width = 0;
        std::size_t height = 0;
        std::vector<float> previous{};
        std::vector<float> current{};
    };
    static SurfaceHistory history;
    if (history.width != field.width || history.height != field.height || history.current.size() != field.vertex_values.size())
    {
        history = {field.width, field.height, field.vertex_values, field.vertex_values};
    }
    else if (history.current != field.vertex_values)
    {
        history.previous = std::move(history.current);
        history.current = field.vertex_values;
    }
    interpolation_alpha = std::clamp(interpolation_alpha, 0.0f, 1.0f);
    for (std::size_t index = 0; index < field.vertex_values.size(); ++index)
    {
        field.vertex_values[index] = history.previous[index] + (history.current[index] - history.previous[index]) * interpolation_alpha;
    }

    const float surface_threshold = physics_sim::particle_surface_threshold(field);
    const auto surface = physics_sim::reconstruct_particle_surface(field, surface_threshold);
    std::vector<SDL_Vertex> vertices;
    vertices.reserve(surface.size() * 3);
    float surface_min_y = std::numeric_limits<float>::max();
    float surface_max_y = std::numeric_limits<float>::lowest();
    for (const auto& triangle : surface)
    {
        for (const auto& point : {triangle.a, triangle.b, triangle.c})
        {
            surface_min_y = std::min(surface_min_y, point.y);
            surface_max_y = std::max(surface_max_y, point.y);
        }
    }
    for (const auto& triangle : surface)
    {
        for (const auto& point : {triangle.a, triangle.b, triangle.c})
        {
            const float depth = surface_max_y > surface_min_y
                ? std::clamp((point.y - surface_min_y) / (surface_max_y - surface_min_y), 0.0f, 1.0f)
                : 0.0f;
            const float lighten = (1.0f - depth) * 0.28f;
            const float darken = depth * 0.12f;
            const auto shade = [&](std::uint8_t channel)
            {
                const float value = static_cast<float>(channel)
                    + (255.0f - static_cast<float>(channel)) * lighten
                    - static_cast<float>(channel) * darken;
                return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 255.0f));
            };
            const SDL_Color color{shade(palette.water.r), shade(palette.water.g), shade(palette.water.b), 224};
            const physics_sim::Vec2 window = viewport.world_to_window({point.x, point.y});
            vertices.push_back({{window.x, window.y}, color, {0.0f, 0.0f}});
        }
    }
    if (!vertices.empty())
    {
        SDL_RenderGeometry(renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), nullptr, 0);
    }

    SDL_SetRenderDrawColor(
        renderer,
        static_cast<std::uint8_t>(std::min(255, static_cast<int>(palette.water.r) + 70)),
        static_cast<std::uint8_t>(std::min(255, static_cast<int>(palette.water.g) + 28)),
        static_cast<std::uint8_t>(std::min(255, static_cast<int>(palette.water.b) + 12)),
        190);
    bool have_previous = false;
    physics_sim::Vec2 previous{};
    for (std::size_t x = 0; x <= field.width; ++x)
    {
        std::size_t top_y = field.height + 1;
        for (std::size_t y = 0; y <= field.height; ++y)
        {
            if (field.vertex_values[y * (field.width + 1) + x] >= surface_threshold)
            {
                top_y = y;
                break;
            }
        }
        if (top_y > field.height)
        {
            have_previous = false;
            continue;
        }
        const physics_sim::Vec2 current = viewport.world_to_window({
            static_cast<float>(x) * field.sample_spacing,
            static_cast<float>(top_y) * field.sample_spacing});
        if (have_previous && std::abs(current.y - previous.y) <= grid.cell_size() * viewport.scale())
        {
            SDL_RenderDrawLineF(renderer, previous.x, previous.y, current.x, current.y);
        }
        previous = current;
        have_previous = true;
    }

    physics_sim::WaterVisualEffectsSettings effect_settings;
    effect_settings.reduced_motion = reduced_motion;
    const auto effects = physics_sim::build_water_visual_effects(
        simulation.particles(), solids, grid.width(), grid.height(), grid.cell_size(), simulation.simulation_tick(), effect_settings);

    SDL_BlendMode previous_blend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &previous_blend);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const auto& point : effects.foam)
    {
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(90.0f + point.intensity * 125.0f, 0.0f, 255.0f));
        SDL_SetRenderDrawColor(renderer, 224, 244, 255, alpha);
        const SDL_FRect bounds = world_rect(
            viewport,
            {point.position.x - point.radius, point.position.y - point.radius},
            {point.radius * 2.0f, point.radius * 2.0f});
        draw_filled_circle(renderer, bounds);
    }
    for (const auto& streak : effects.spray)
    {
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(75.0f + streak.intensity * 145.0f, 0.0f, 255.0f));
        SDL_SetRenderDrawColor(renderer, 205, 238, 255, alpha);
        const auto start = viewport.world_to_window(streak.start);
        const auto end = viewport.world_to_window(streak.end);
        SDL_RenderDrawLineF(renderer, start.x, start.y, end.x, end.y);
    }
    for (const auto& impact : effects.impacts)
    {
        const std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(45.0f + impact.intensity * 90.0f, 0.0f, 180.0f));
        SDL_SetRenderDrawColor(renderer, 214, 242, 255, alpha);
        const SDL_FRect bounds = world_rect(
            viewport,
            {impact.position.x - impact.radius, impact.position.y - impact.radius},
            {impact.radius * 2.0f, impact.radius * 2.0f});
        draw_filled_circle(renderer, bounds);
    }
    SDL_SetRenderDrawBlendMode(renderer, previous_blend);
}

void draw_emitters(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
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
            SDL_SetRenderDrawColor(renderer, palette.emitter_directional.r, palette.emitter_directional.g, palette.emitter_directional.b, palette.emitter_directional.a);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, palette.emitter_omni.r, palette.emitter_omni.g, palette.emitter_omni.b, palette.emitter_omni.a);
        }
        SDL_RenderFillRectF(renderer, &icon);

        const physics_sim::Vec2 direction = physics_sim::normalize(emitter.direction);
        const physics_sim::Vec2 line_end_world = emitter.position + (direction * line_length);
        const physics_sim::Vec2 line_start = viewport.world_to_window(emitter.position);
        const physics_sim::Vec2 line_end = viewport.world_to_window(line_end_world);

        SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, palette.text.a);
        SDL_RenderDrawLineF(renderer, line_start.x, line_start.y, line_end.x, line_end.y);

        const SDL_FRect highlight = world_rect(
            viewport,
            {emitter.position.x - icon_size * 0.85f, emitter.position.y - icon_size * 0.85f},
            {icon_size * 1.7f, icon_size * 1.7f});
        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, palette.selection.a);
            SDL_RenderDrawRectF(renderer, &highlight);
        }
    }
}

void draw_gates(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
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
            SDL_SetRenderDrawColor(renderer, palette.gate_open.r, palette.gate_open.g, palette.gate_open.b, palette.gate_open.a);
            SDL_RenderDrawRectF(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, palette.gate_closed.r, palette.gate_closed.g, palette.gate_closed.b, palette.gate_closed.a);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, palette.selection.a);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_drains(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
{
    const float cell_size = simulation.grid().cell_size();

    for (std::size_t index = 0; index < simulation.drains().size(); ++index)
    {
        const auto& drain = simulation.drains()[index];
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(drain.x) * cell_size, static_cast<float>(drain.y) * cell_size},
            {static_cast<float>(drain.width) * cell_size, static_cast<float>(drain.height) * cell_size});

        if (drain.enabled)
        {
            SDL_SetRenderDrawColor(renderer, palette.drain_active.r, palette.drain_active.g, palette.drain_active.b, palette.drain_active.a);
            SDL_RenderFillRectF(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, palette.error.r, palette.error.g, palette.error.b, palette.error.a);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, palette.drain_inactive.r, palette.drain_inactive.g, palette.drain_inactive.b, palette.drain_inactive.a);
        }

        SDL_RenderDrawRectF(renderer, &rect);
        SDL_RenderDrawLineF(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
        SDL_RenderDrawLineF(renderer, rect.x + rect.w, rect.y, rect.x, rect.y + rect.h);
        if (controller.selected_drain_index() == index)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, 255);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_pumps(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
{
    const float cell_size = simulation.grid().cell_size();

    for (std::size_t index = 0; index < simulation.pumps().size(); ++index)
    {
        const auto& pump = simulation.pumps()[index];
        const SDL_FRect rect = world_rect(
            viewport,
            {static_cast<float>(pump.x) * cell_size, static_cast<float>(pump.y) * cell_size},
            {static_cast<float>(pump.width) * cell_size, static_cast<float>(pump.height) * cell_size});

        if (pump.enabled)
        {
            SDL_SetRenderDrawColor(renderer, palette.pump_active.r, palette.pump_active.g, palette.pump_active.b, palette.pump_active.a);
            SDL_RenderFillRectF(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, palette.text.a);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, palette.pump_inactive.r, palette.pump_inactive.g, palette.pump_inactive.b, palette.pump_inactive.a);
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
        if (controller.selected_pump_index() == index)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, 255);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_valves(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
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
            SDL_SetRenderDrawColor(renderer, palette.valve_open.r, palette.valve_open.g, palette.valve_open.b, palette.valve_open.a);
            SDL_RenderDrawRectF(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, palette.valve_closed.r, palette.valve_closed.g, palette.valve_closed.b, palette.valve_closed.a);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, palette.selection.a);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_sensors(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::UiPalette& palette)
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
            const auto& color = sensor.active ? palette.sensor_objective_active : palette.sensor_objective_inactive;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        }
        else
        {
            const auto& color = sensor.active ? palette.sensor_normal_active : palette.sensor_normal_inactive;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        }
        SDL_RenderFillRectF(renderer, &rect);

        const auto& border = sensor.enabled ? palette.text : palette.muted_text;
        SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
        SDL_RenderDrawRectF(renderer, &rect);

        if (selected_index.has_value() && *selected_index == i)
        {
            SDL_SetRenderDrawColor(renderer, palette.selection.r, palette.selection.g, palette.selection.b, palette.selection.a);
            SDL_RenderDrawRectF(renderer, &rect);
        }
    }
}

void draw_crosshair(SDL_Renderer* renderer, int x, int y, const physics_sim::UiPalette& palette)
{
    SDL_SetRenderDrawColor(renderer, palette.crosshair.r, palette.crosshair.g, palette.crosshair.b, palette.crosshair.a);
    SDL_RenderDrawLine(renderer, x - 12, y, x + 12, y);
    SDL_RenderDrawLine(renderer, x, y - 12, x, y + 12);
    SDL_Rect dot{ x - 2, y - 2, 4, 4 };
    SDL_RenderFillRect(renderer, &dot);
}

void draw_help_overlay(
    SDL_Renderer* renderer,
    int window_width,
    int window_height,
    const physics_sim::InputBindings& bindings,
    const physics_sim::UiPalette& palette)
{
    const auto lines = physics_sim::build_help_overlay_lines(bindings);

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
    SDL_SetRenderDrawColor(renderer, palette.panel_background.r, palette.panel_background.g, palette.panel_background.b, palette.panel_background.a);
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, palette.panel_border.r, palette.panel_border.g, palette.panel_border.b, palette.panel_border.a);
    SDL_RenderDrawRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, palette.text.a);
    int line_y = y + padding;
    for (const auto& line : lines)
    {
        physics_sim::draw_text(renderer, x + padding, line_y, scale, line);
        line_y += line_height;
    }
}

void draw_tutorial_overlay(
    SDL_Renderer* renderer,
    int window_width,
    int window_height,
    const physics_sim::TutorialProgress& progress,
    const physics_sim::InputBindings& bindings,
    const physics_sim::UiPalette& palette)
{
    const auto lines = physics_sim::build_tutorial_overlay_lines(progress, bindings);

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
    SDL_SetRenderDrawColor(renderer, palette.panel_background.r, palette.panel_background.g, palette.panel_background.b, palette.panel_background.a);
    SDL_RenderFillRect(renderer, &background);

    SDL_SetRenderDrawColor(renderer, palette.panel_border.r, palette.panel_border.g, palette.panel_border.b, palette.panel_border.a);
    SDL_RenderDrawRect(renderer, &background);

    int line_y = y + padding;
    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        const bool is_current = i == 1;
        const bool is_complete_line = physics_sim::tutorial_is_complete(progress) && i + 1 == lines.size();
        const auto& color = is_complete_line ? palette.success : (is_current ? palette.selection : palette.text);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        physics_sim::draw_text(renderer, x + padding, line_y, scale, lines[i]);
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
            "LEFT/RIGHT OR WHEEL FILTER",
            "ENTER LOADS SCENE",
            "ESC GOES BACK",
        };
    case physics_sim::SessionShellScreen::SaveBrowser:
        return {
            "SAVE BROWSER",
            "UP/DOWN SELECT",
            "ENTER LOADS SAVE",
            "ESC GOES BACK",
        };
    case physics_sim::SessionShellScreen::Settings:
        return {
            "SETTINGS",
            "UP/DOWN SELECT",
            "ENTER TO TOGGLE OR REBIND",
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
    const std::vector<physics_sim::GalleryEntry>& galleryEntries,
    const std::vector<physics_sim::SaveBrowserEntry>& saveBrowserEntries,
    const physics_sim::UserSettings& userSettings)
{
    std::vector<std::string> lines;

    switch (shell.screen)
    {
    case physics_sim::SessionShellScreen::Playing:
        break;
    case physics_sim::SessionShellScreen::MainMenu:
        lines.reserve(6);
        for (std::size_t index = 0; index < physics_sim::session_shell_option_count(shell.screen, galleryEntries.size()); ++index)
        {
            lines.emplace_back(physics_sim::session_shell_main_menu_label(index));
        }
        break;
    case physics_sim::SessionShellScreen::SceneBrowser:
        lines.reserve(galleryEntries.size() + 1);
        for (const auto& entry : galleryEntries)
        {
            lines.emplace_back("[" + std::string{physics_sim::gallery_category_name(entry.category)} + "] " + entry.title);
        }
        lines.emplace_back("BACK");
        break;
    case physics_sim::SessionShellScreen::SaveBrowser:
        lines.reserve(saveBrowserEntries.size());
        for (const auto& entry : saveBrowserEntries)
        {
            lines.emplace_back(entry.label);
        }
        break;
    case physics_sim::SessionShellScreen::Settings:
    {
        const auto settings_entries = physics_sim::build_settings_menu_entries(userSettings);
        lines.reserve(settings_entries.size());
        for (const auto& entry : settings_entries)
        {
            lines.emplace_back(entry.label);
        }
        break;
    }
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
    const std::vector<physics_sim::GalleryEntry>& galleryEntries,
    const std::vector<physics_sim::SaveBrowserEntry>& saveBrowserEntries,
    const physics_sim::UserSettings& userSettings,
    const physics_sim::UiPalette& palette)
{
    const auto body_lines = build_session_shell_body_lines(shell.screen);
    const auto option_lines = build_session_shell_option_lines(shell, galleryEntries, saveBrowserEntries, userSettings);
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
    SDL_SetRenderDrawColor(renderer, palette.panel_background.r, palette.panel_background.g, palette.panel_background.b, palette.panel_background.a);
    SDL_RenderFillRect(renderer, &panel);

    SDL_SetRenderDrawColor(renderer, palette.panel_border.r, palette.panel_border.g, palette.panel_border.b, palette.panel_border.a);
    SDL_RenderDrawRect(renderer, &panel);

    int line_y = y + padding;
    for (std::size_t index = 0; index < body_lines.size(); ++index)
    {
        SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, palette.text.a);
        physics_sim::draw_text(renderer, x + padding, line_y, scale, body_lines[index]);
        line_y += line_height;
    }

    if (!option_lines.empty())
    {
        line_y += line_height;
        for (std::size_t index = 0; index < option_lines.size(); ++index)
        {
            const bool selected = index == shell.selection;
            const auto& color = selected ? palette.selection : palette.muted_text;
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
    const std::vector<physics_sim::GalleryEntry>& galleryEntries,
    const std::vector<physics_sim::SaveBrowserEntry>& saveBrowserEntries,
    const physics_sim::UserSettings& userSettings)
{
    const auto body_lines = build_session_shell_body_lines(shell.screen);
    const auto option_lines = build_session_shell_option_lines(shell, galleryEntries, saveBrowserEntries, userSettings);

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
    const physics_sim::Vec2& mouse_world,
    const physics_sim::UiPalette& palette)
{
    const float cell_size = simulation.grid().cell_size();
    const auto tool = controller.tool();
    if (tool == physics_sim::SceneTool::PointerWater)
    {
        SDL_SetRenderDrawColor(renderer, palette.tool_preview_valid.r, palette.tool_preview_valid.g, palette.tool_preview_valid.b, palette.tool_preview_valid.a);
        const float circle_size = cell_size * 0.55f;
        const SDL_FRect rect = world_rect(
            viewport,
            {mouse_world.x - circle_size * 0.5f, mouse_world.y - circle_size * 0.5f},
            {circle_size, circle_size});
        draw_filled_circle(renderer, rect);

        const physics_sim::Vec2 direction = controller.emitter_direction();
        const physics_sim::Vec2 arrow_end = mouse_world + direction * (cell_size * 0.7f);
        const physics_sim::Vec2 window_start = viewport.world_to_window(mouse_world);
        const physics_sim::Vec2 window_end = viewport.world_to_window(arrow_end);
        SDL_RenderDrawLineF(renderer, window_start.x, window_start.y, window_end.x, window_end.y);
    }
    else if (tool == physics_sim::SceneTool::PaintWall || tool == physics_sim::SceneTool::EraseWall)
    {
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size, cell_size});
        const auto& color = tool == physics_sim::SceneTool::PaintWall ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRectF(renderer, &rect);
    }
    else if (tool == physics_sim::SceneTool::DirectionalEmitter || tool == physics_sim::SceneTool::OmniEmitter)
    {
        const bool valid_placement = controller.can_place_fixture(mouse_world);
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

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
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
        SDL_SetRenderDrawColor(renderer, palette.text.r, palette.text.g, palette.text.b, palette.text.a);
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
        const auto& color = valid_placement ? palette.tool_preview_valid : palette.tool_preview_invalid;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
        SDL_SetRenderDrawColor(renderer, palette.tool_preview_invalid.r, palette.tool_preview_invalid.g, palette.tool_preview_invalid.b, palette.tool_preview_invalid.a);
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

void paint_default_basin(physics_sim::SceneController& controller)
{
    controller.set_tool(physics_sim::SceneTool::PaintWall);
    paint_wall_line(controller, {320.0f, 576.0f}, {960.0f, 576.0f});
    paint_wall_line(controller, {320.0f, 384.0f}, {320.0f, 576.0f});
    paint_wall_line(controller, {960.0f, 384.0f}, {960.0f, 576.0f});
}

void load_starter_scene(physics_sim::SceneController& controller)
{
    const auto previous_tool = controller.tool();

    controller.reset_scene();
    paint_default_basin(controller);

    controller.set_tool(previous_tool);
    controller.sync_history();
}

void load_demo_scene(physics_sim::SceneController& controller)
{
    const auto previous_tool = controller.tool();

    controller.reset_scene();
    paint_default_basin(controller);

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
    const AppLogger* logger = nullptr,
    physics_sim::PlayerFeedback* feedback_out = nullptr,
    physics_sim::FluidSolverProfile fallback_solver_profile = physics_sim::FluidSolverProfile::Balanced,
    std::optional<physics_sim::FluidSolverProfile> forced_solver_profile = std::nullopt)
{
    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene load attempt: " << path.string();
        logger->log(stream.str());
    }

    if (!physics_sim::load_scene(path, simulation, metadata_out, fallback_solver_profile, forced_solver_profile))
    {
        const auto feedback = physics_sim::describe_scene_load_failure(path);
        if (feedback_out != nullptr)
        {
            *feedback_out = feedback;
        }

        if (logger != nullptr)
        {
            std::ostringstream stream;
            stream << "scene load failed: " << path.string()
                   << " status=" << feedback.status_message
                   << " detail=" << feedback.detail
                   << " recovery=" << physics_sim::player_recovery_action_name(feedback.recovery);
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
    const AppLogger* logger = nullptr,
    physics_sim::FluidSolverProfile fallback_solver_profile = physics_sim::FluidSolverProfile::Balanced,
    std::optional<physics_sim::FluidSolverProfile> forced_solver_profile = std::nullopt)
{
    if (load_scene_from_file(path, simulation, viewport, state, driver, controller, metadata_out, logger, nullptr, fallback_solver_profile, forced_solver_profile))
    {
        return true;
    }

    if (logger != nullptr)
    {
        std::ostringstream stream;
        stream << "scene fallback: loading demo scene after failure for " << path.string();
        logger->log(stream.str());
    }

    const bool no_spout_fallback = path.filename() == fs::path{"starter_basin.pscene"}
        || path.filename() == fs::path{"tutorial_intro.pscene"};
    if (no_spout_fallback)
    {
        load_starter_scene(controller);
    }
    else
    {
        load_demo_scene(controller);
    }
    const auto profile = forced_solver_profile.value_or(fallback_solver_profile);
    simulation.set_solver_settings(physics_sim::WaterSimulation2D::solver_settings_for_profile(profile));
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
    const physics_sim::SessionShellState* session_shell_state = nullptr,
    const physics_sim::TutorialProgress* tutorial_progress = nullptr)
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

    if (tutorial_progress != nullptr && !physics_sim::tutorial_is_complete(*tutorial_progress))
    {
        title << " | tutorial=" << physics_sim::tutorial_step_title(physics_sim::tutorial_current_step(*tutorial_progress));
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
    const AppLogger* logger = nullptr,
    physics_sim::TutorialProgress* tutorialProgress = nullptr,
    physics_sim::FluidSolverProfile fallback_solver_profile = physics_sim::FluidSolverProfile::Balanced,
    std::optional<physics_sim::FluidSolverProfile> forced_solver_profile = std::nullopt)
{
    switch (action)
    {
    case physics_sim::Action::Quit:
        break;
    case physics_sim::Action::TogglePause:
    {
        const bool was_paused = driver.paused();
        driver.toggle_pause();
        if (tutorialProgress != nullptr)
        {
            if (!was_paused && driver.paused())
            {
                tutorialProgress->pause_opened = true;
            }
            else if (was_paused && !driver.paused())
            {
                tutorialProgress->pause_resumed = true;
            }
        }
        break;
    }
    case physics_sim::Action::StepOnce:
        driver.request_step();
        break;
    case physics_sim::Action::Reset:
        if (logger != nullptr)
        {
            logger->log("action: reset");
        }
        restore_demo_scene(
            current_scene_path,
            simulation,
            viewport,
            state,
            driver,
            controller,
            metadata_out,
            logger,
            fallback_solver_profile,
            forced_solver_profile);
        if (tutorialProgress != nullptr)
        {
            physics_sim::tutorial_mark_reset_or_retry(*tutorialProgress);
        }
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
        if (tutorialProgress != nullptr)
        {
            physics_sim::tutorial_mark_reset_or_retry(*tutorialProgress);
        }
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
    float move_step,
    const physics_sim::InputBindings& bindings,
    physics_sim::TutorialProgress* tutorialProgress = nullptr)
{
    const bool has_selection = controller.has_selected_fixture();
    const bool move_selected = has_selection && (modifiers & KMOD_CTRL) != 0;
    const auto mark_device_used = [&]()
    {
        if (tutorialProgress != nullptr)
        {
            physics_sim::tutorial_mark_device_used(*tutorialProgress);
        }
    };

    if (move_selected)
    {
        switch (keycode)
        {
        case SDLK_UP:
        case SDLK_w:
            static_cast<void>(controller.move_selected_fixture({0.0f, -move_step}));
            mark_device_used();
            return;
        case SDLK_DOWN:
        case SDLK_s:
            static_cast<void>(controller.move_selected_fixture({0.0f, move_step}));
            mark_device_used();
            return;
        case SDLK_LEFT:
        case SDLK_a:
            static_cast<void>(controller.move_selected_fixture({-move_step, 0.0f}));
            mark_device_used();
            return;
        case SDLK_RIGHT:
        case SDLK_d:
            static_cast<void>(controller.move_selected_fixture({move_step, 0.0f}));
            mark_device_used();
            return;
        default:
            break;
        }
    }

    const auto rotate_counterclockwise = keycode == bindings.rotate_counterclockwise;
    const auto rotate_clockwise = keycode == bindings.rotate_clockwise;
    const auto speed_down = keycode == bindings.speed_down;
    const auto speed_up = keycode == bindings.speed_up;
    const auto emission_down = keycode == bindings.emission_down;
    const auto emission_up = keycode == bindings.emission_up;

    if (rotate_counterclockwise)
    {
        if (controller.has_selected_pump())
        {
            static_cast<void>(controller.rotate_selected_pump(-0.2617994f));
        }
        else if (has_selection)
        {
            static_cast<void>(controller.rotate_selected_fixture(-0.2617994f));
        }
        else
        {
            controller.rotate_emitter_direction(-0.2617994f);
        }
        mark_device_used();
        return;
    }

    if (rotate_clockwise)
    {
        if (controller.has_selected_pump())
        {
            static_cast<void>(controller.rotate_selected_pump(0.2617994f));
        }
        else if (has_selection)
        {
            static_cast<void>(controller.rotate_selected_fixture(0.2617994f));
        }
        else
        {
            controller.rotate_emitter_direction(0.2617994f);
        }
        mark_device_used();
        return;
    }

    if (speed_down)
    {
        if (controller.has_selected_pump())
        {
            static_cast<void>(controller.adjust_selected_pump_strength(-1.0f));
        }
        else if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_speed(-1.0f));
        }
        else
        {
            controller.set_emitter_speed(std::max(0.0f, controller.emitter_speed() - 1.0f));
        }
        mark_device_used();
        return;
    }

    if (speed_up)
    {
        if (controller.has_selected_pump())
        {
            static_cast<void>(controller.adjust_selected_pump_strength(1.0f));
        }
        else if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_speed(1.0f));
        }
        else
        {
            controller.set_emitter_speed(controller.emitter_speed() + 1.0f);
        }
        mark_device_used();
        return;
    }

    if (emission_down)
    {
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_emission_rate(-4.0f));
        }
        else
        {
            controller.set_emission_rate(std::max(0.0f, controller.emission_rate() - 4.0f));
        }
        mark_device_used();
        return;
    }

    if (emission_up)
    {
        if (has_selection)
        {
            static_cast<void>(controller.adjust_selected_fixture_emission_rate(4.0f));
        }
        else
        {
            controller.set_emission_rate(controller.emission_rate() + 4.0f);
        }
        mark_device_used();
        return;
    }

    if (keycode == SDLK_t)
    {
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
        else if (controller.has_selected_drain())
        {
            static_cast<void>(controller.toggle_selected_drain_enabled());
        }
        else if (controller.has_selected_pump())
        {
            static_cast<void>(controller.toggle_selected_pump_enabled());
        }
        else if (has_selection)
        {
            static_cast<void>(controller.toggle_selected_fixture_enabled());
        }
        mark_device_used();
        return;
    }
}
} // namespace

int physics_sim::app::run_application(int argc, char* argv[])
{
    const RuntimeOptions options = parse_command_line(argc, argv);
    SDL_SetMainReady();
    const fs::path executableBase = executable_base_path();
    const auto resource_path = [&](const fs::path& path) { return resolve_runtime_resource(path, executableBase); };
    const fs::path starterScenePath = resource_path("scenes/starter_basin.pscene");
    const fs::path demoScenePath = resource_path("scenes/demo_scene.pscene");
    const fs::path tutorialScenePath = resource_path("scenes/tutorial_intro.pscene");
    const fs::path saveDirectory = default_save_directory().value_or(fs::path{"physics-sim-saves"});
    const fs::path autosaveScenePath = saveDirectory / "autosave.pscene";
    const fs::path logFilePath = options.logFilePath.value_or(fs::path{"physics-sim.log"});
    const fs::path startupScenePath = options.startupScenePath.has_value() ? resource_path(*options.startupScenePath) : starterScenePath;
    physics_sim::GalleryManifest galleryManifest;
    std::string galleryManifestError;
    const fs::path galleryManifestPath = resource_path("gallery/gallery.manifest");
    if (!physics_sim::load_gallery_manifest(galleryManifestPath, galleryManifest, &galleryManifestError))
    {
        galleryManifest.entries = {
            {"tutorial-intro", "Pour and Build", "Learn pouring and wall editing.", physics_sim::GalleryCategory::Learn, tutorialScenePath, {}, {"tutorial"}, 10},
            {"starter-basin", "Starter Basin", "A clean basin for freeform play.", physics_sim::GalleryCategory::Sandbox, starterScenePath, {}, {"pour", "walls"}, 20},
        };
    }
    else
    {
        for (auto& entry : galleryManifest.entries)
        {
            entry.scene_path = resource_path(entry.scene_path);
            entry.thumbnail_path = resource_path(entry.thumbnail_path);
        }
    }
    const std::vector<physics_sim::GalleryEntry> allGalleryEntries = galleryManifest.entries;
    std::vector<physics_sim::GalleryEntry> galleryEntries = allGalleryEntries;
    int galleryCategoryFilter = 0;
    const auto rebuild_gallery_filter = [&]()
    {
        galleryEntries.clear();
        for (const auto& entry : allGalleryEntries)
        {
            const bool included = galleryCategoryFilter == 0
                || (galleryCategoryFilter == 1 && entry.category == physics_sim::GalleryCategory::Learn)
                || (galleryCategoryFilter == 2 && entry.category == physics_sim::GalleryCategory::Sandbox)
                || (galleryCategoryFilter == 3 && entry.category == physics_sim::GalleryCategory::Challenges);
            if (included) galleryEntries.push_back(entry);
        }
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
        for (std::size_t index = 0; index < galleryEntries.size(); ++index)
        {
            if (same_scene_path(path, galleryEntries[index].scene_path))
            {
                return index;
            }
        }

        return std::nullopt;
    };
    AppLogger logger{logFilePath};

    if (!galleryManifestError.empty())
    {
        logger.log(galleryManifestError + " Using the safe built-in gallery fallback.");
    }

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
        if (options.initialSolverProfile.has_value())
        {
            stream << " initial_solver_profile=" << solver_profile_name(*options.initialSolverProfile);
        }
        logger.log(stream.str());
    }

    std::optional<physics_sim::ReplayScript> replayScript;
    std::size_t nextReplayEventIndex = 0;
    std::uint64_t replayTick = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        const auto feedback = physics_sim::describe_renderer_failure(SDL_GetError());
        show_error(feedback.status_message.c_str(), feedback.detail, &logger);
        return 1;
    }
    logger.log("SDL_Init ok");

    if (options.replayFilePath.has_value())
    {
        replayScript = physics_sim::load_replay_script(*options.replayFilePath);
        if (!replayScript.has_value())
        {
            const auto feedback = physics_sim::describe_replay_failure("replay script missing or malformed");
            logger.log("replay script load failed: " + options.replayFilePath->string());
            logger.log(feedback.detail);
            show_error(feedback.status_message.c_str(), feedback.detail, &logger);
            SDL_Quit();
            return 1;
        }

        std::ostringstream stream;
        stream << "replay script loaded: " << options.replayFilePath->string()
               << " events=" << replayScript->events.size();
        logger.log(stream.str());
    }

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
    if (options.initialSolverProfile.has_value())
    {
        userSettings.solver_profile = *options.initialSolverProfile;
    }
    if (options.initialReducedMotion.has_value())
    {
        userSettings.reduced_motion = *options.initialReducedMotion;
    }

    {
        std::ostringstream stream;
        stream << "settings path=" << settingsPath.string()
               << " window=" << userSettings.window_size.width << 'x' << userSettings.window_size.height
               << " help_overlay=" << (userSettings.help_overlay_visible ? "on" : "off")
               << " visual_mode=" << visual_mode_name(userSettings.visual_mode)
               << " solver_profile=" << solver_profile_name(userSettings.solver_profile)
               << " reduced_motion=" << (userSettings.reduced_motion ? "on" : "off");
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
        const auto feedback = physics_sim::describe_renderer_failure(SDL_GetError());
        show_error(feedback.status_message.c_str(), feedback.detail, &logger);
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
        const auto feedback = physics_sim::describe_renderer_failure(SDL_GetError());
        show_error(feedback.status_message.c_str(), feedback.detail, &logger);
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

    std::optional<physics_sim::PlayerFeedback> startupAudioFeedback;
    physics_sim::AudioPlayer audioPlayer;
    {
        physics_sim::AudioSettings audioSettings{
            userSettings.audio_muted,
            userSettings.audio_master_volume,
            userSettings.audio_effects_volume,
            userSettings.audio_music_volume,
        };
        std::string audioError;
        const bool audioStarted = audioPlayer.initialize(audioSettings, options.disableAudio, &audioError);
        if (options.disableAudio)
        {
            logger.log("audio disabled by command-line flag");
        }
        else if (audioStarted)
        {
            logger.log("audio initialized");
        }
        else
        {
            startupAudioFeedback = physics_sim::describe_audio_failure(audioError);
            logger.log(startupAudioFeedback->detail);
        }
    }

    const auto sync_audio_settings = [&]()
    {
        audioPlayer.apply_settings({
            userSettings.audio_muted,
            userSettings.audio_master_volume,
            userSettings.audio_effects_volume,
            userSettings.audio_music_volume,
        });
    };

    const auto play_audio = [&](physics_sim::AudioCue cue)
    {
        static_cast<void>(audioPlayer.play(cue));
    };

    sync_audio_settings();

    bool running = true;
    int applicationExitCode = 0;
    int windowWidth = userSettings.window_size.width;
    int windowHeight = userSettings.window_size.height;
    physics_sim::Vec2 mouseScreen{windowWidth * 0.5f, windowHeight * 0.5f};
    physics_sim::FixedStepDriver stepDriver;
    physics_sim::SimulationState simulationState;
    physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
    simulation.set_solver_settings(physics_sim::WaterSimulation2D::solver_settings_for_profile(userSettings.solver_profile));
    physics_sim::SceneController controller{simulation};
    physics_sim::SceneViewport viewport;
    physics_sim::SceneMetadata sceneMetadata;
    physics_sim::ChallengeEvaluator challengeEvaluator;
    const auto sync_solver_profile_from_simulation = [&]()
    {
        userSettings.solver_profile = simulation.solver_settings().profile;
    };
    viewport.set_world_size({1280.0f, 720.0f});
    viewport.set_window_size(windowWidth, windowHeight);
    const bool captureByTickCount = options.dumpFrameAfterTicks.has_value();
    const bool sessionShellBypassed = options.skipSessionShell
        || options.replayFilePath.has_value()
        || options.dumpFramePath.has_value()
        || captureByTickCount;
    const fs::path tutorialStatePath = default_tutorial_state_path().value_or(fs::path{"physics-sim-tutorial-state.txt"});
    const bool tutorialSeen = load_tutorial_seen_state(tutorialStatePath).value_or(false);
    bool tutorialStartup = options.tutorialMode || (!sessionShellBypassed && !tutorialSeen);
    const fs::path initialScenePath = tutorialStartup ? tutorialScenePath : startupScenePath;
    fs::path currentScenePath = initialScenePath;
    std::optional<std::size_t> currentGalleryIndex = gallery_index_for_path(currentScenePath);
    std::vector<physics_sim::SaveBrowserEntry> saveBrowserEntries;
    const auto refresh_save_browser_entries = [&]()
    {
        saveBrowserEntries = physics_sim::build_save_browser_entries(saveDirectory, autosaveScenePath);
    };

    if (tutorialStartup)
    {
        if (!fs::exists(tutorialScenePath))
        {
            const auto feedback = physics_sim::describe_package_content_failure("tutorial scene missing");
            logger.log(feedback.detail);
            show_error(feedback.status_message.c_str(), feedback.detail, &logger);
            static_cast<void>(restore_demo_scene(
                starterScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                userSettings.solver_profile,
                options.initialSolverProfile));
            currentScenePath = starterScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            logger.log("tutorial scene fallback: loaded starter basin after tutorial content missing");
        }
        else if (!load_scene_from_file(
                     tutorialScenePath,
                     simulation,
                     viewport,
                     simulationState,
                     stepDriver,
                     controller,
                     &sceneMetadata,
                     &logger,
                     nullptr,
                     userSettings.solver_profile,
                     options.initialSolverProfile))
        {
            static_cast<void>(restore_demo_scene(
                starterScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                userSettings.solver_profile,
                options.initialSolverProfile));
            currentScenePath = starterScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            logger.log("tutorial scene fallback: loaded starter basin after tutorial load failure");
        }
    }
    else
    {
        static_cast<void>(restore_demo_scene(
            startupScenePath,
            simulation,
            viewport,
            simulationState,
            stepDriver,
            controller,
            &sceneMetadata,
            &logger,
            userSettings.solver_profile,
            options.initialSolverProfile));
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
    }

    if (replayScript.has_value())
    {
        const auto sceneDigest = physics_sim::stable_replay_file_digest(currentScenePath);
        const bool identityMatches = sceneDigest.has_value()
            && physics_sim::replay_identity_matches(
                *replayScript,
                *sceneDigest,
                stepDriver.fixed_step().count(),
                simulation.solver_settings().profile);
        if (!identityMatches)
        {
            std::ostringstream detail;
            detail << "replay identity mismatch: scene="
                   << (sceneDigest.has_value() ? *sceneDigest : std::string{"unavailable"})
                   << " expected_scene=" << replayScript->scene_digest
                   << " timestep=" << stepDriver.fixed_step().count()
                   << " expected_timestep=" << replayScript->fixed_timestep
                   << " profile=" << solver_profile_name(simulation.solver_settings().profile)
                   << " expected_profile=" << solver_profile_name(replayScript->solver_profile);
            logger.log(detail.str());
            if (!options.skipSessionShell && !options.autoExitAfter.has_value() && !options.dumpFramePath.has_value())
            {
                show_error("REPLAY IDENTITY MISMATCH", detail.str(), &logger);
            }
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }

    sync_solver_profile_from_simulation();
    refresh_save_browser_entries();

    bool tutorialActive = tutorialStartup;
    physics_sim::TutorialProgress tutorialProgress{};
    bool panning = false;
    bool painting = false;
    std::optional<physics_sim::SceneTool> paintingTool{};
    std::size_t paintingSolidCount = 0;
    bool pointerWaterActive = false;
    std::array<physics_sim::WaterEmitter, 1> pointerWaterEmitters{};
    bool frameDumped = false;
    VisualMode visualMode = userSettings.visual_mode;
    bool showHelp = userSettings.help_overlay_visible;
    bool advancedToolsVisible = options.advancedTools;
    const bool cleanCaptureFrame = captureByTickCount;
    physics_sim::SessionShellState sessionShellState;
    if (tutorialActive || sessionShellBypassed)
    {
        sessionShellState.screen = physics_sim::SessionShellScreen::Playing;
        stepDriver.set_paused(false);
    }
    else
    {
        sessionShellState.screen = physics_sim::SessionShellScreen::MainMenu;
        stepDriver.set_paused(true);
    }

    const auto sync_pointer_water_emitter = [&]()
    {
        auto& emitter = pointerWaterEmitters[0];
        emitter.kind = physics_sim::WaterEmitterKind::Directional;
        emitter.position = viewport.window_to_world(mouseScreen);
        emitter.direction = controller.emitter_direction();
        emitter.speed = controller.emitter_speed();
        emitter.emission_rate = controller.emission_rate();
        emitter.enabled = pointerWaterActive;
    };

    const auto stop_pointer_water = [&]()
    {
        pointerWaterActive = false;
        pointerWaterEmitters[0] = {};
    };

    const auto start_pointer_water = [&]()
    {
        pointerWaterEmitters[0] = {};
        pointerWaterActive = true;
        sync_pointer_water_emitter();
    };

    const auto set_current_tool = [&](physics_sim::SceneTool tool)
    {
        if (controller.tool() != tool)
        {
            stop_pointer_water();
        }
        controller.set_tool(tool);
    };

    const auto update_shell_pause_state = [&]()
    {
        if (sessionShellState.screen != physics_sim::SessionShellScreen::Playing)
        {
            stop_pointer_water();
        }
        stepDriver.set_paused(sessionShellState.screen != physics_sim::SessionShellScreen::Playing);
    };

    enum class StatusMessageKind
    {
        Info,
        Success,
        Warning,
        Error,
    };

    const auto status_message_rgba = [](const physics_sim::UiPalette& palette, StatusMessageKind kind) noexcept -> std::array<std::uint8_t, 4>
    {
        switch (kind)
        {
        case StatusMessageKind::Info:
            return {palette.text.r, palette.text.g, palette.text.b, palette.text.a};
        case StatusMessageKind::Success:
            return {palette.success.r, palette.success.g, palette.success.b, palette.success.a};
        case StatusMessageKind::Warning:
            return {palette.warning.r, palette.warning.g, palette.warning.b, palette.warning.a};
        case StatusMessageKind::Error:
            return {palette.error.r, palette.error.g, palette.error.b, palette.error.a};
        }

        return {palette.text.r, palette.text.g, palette.text.b, palette.text.a};
    };

    std::optional<std::string> statusMessage;
    Clock::time_point statusMessageStartedAt{};
    Clock::time_point statusMessageExpiresAt{};
    StatusMessageKind statusMessageKind = StatusMessageKind::Info;

    const auto set_status_message = [&](std::string message, StatusMessageKind kind = StatusMessageKind::Info)
    {
        statusMessage = std::move(message);
        statusMessageKind = kind;
        statusMessageStartedAt = Clock::now();
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

    const auto current_status_message_alpha = [&]() -> float
    {
        if (!statusMessage.has_value())
        {
            return 1.0f;
        }

        const Clock::time_point now = Clock::now();
        if (now > statusMessageExpiresAt)
        {
            return 1.0f;
        }

        const std::chrono::duration<double> elapsed = now - statusMessageStartedAt;
        const std::chrono::duration<double> lifetime = statusMessageExpiresAt - statusMessageStartedAt;
        return physics_sim::feedback_message_alpha(elapsed, lifetime, userSettings.reduced_motion);
    };

    std::size_t previousActiveSensors = simulation.metrics().active_sensors;
    bool previousObjectiveCompleted = simulation.metrics().objective_completed;
    std::uint64_t previousAudioEmitted = simulation.metrics().total_emitted;
    std::uint64_t previousAudioOutflow = simulation.metrics().total_outflow;
    const auto sync_scene_feedback_metrics = [&]()
    {
        const auto metrics = simulation.metrics();
        previousActiveSensors = metrics.active_sensors;
        previousObjectiveCompleted = metrics.objective_completed;
        previousAudioEmitted = metrics.total_emitted;
        previousAudioOutflow = metrics.total_outflow;
        challengeEvaluator.reset();
        static_cast<void>(challengeEvaluator.update(sceneMetadata.challenge, metrics, simulation.simulation_tick()));
    };

    const auto update_scene_feedback_audio = [&]()
    {
        const auto metrics = simulation.metrics();
        const std::uint64_t emittedDelta = metrics.total_emitted >= previousAudioEmitted ? metrics.total_emitted - previousAudioEmitted : 0;
        const std::uint64_t outflowDelta = metrics.total_outflow >= previousAudioOutflow ? metrics.total_outflow - previousAudioOutflow : 0;
        const bool pumpEnabled = std::any_of(simulation.pumps().begin(), simulation.pumps().end(), [](const physics_sim::WaterPump& pump) { return pump.enabled; });
        const bool drainEnabled = std::any_of(simulation.drains().begin(), simulation.drains().end(), [](const physics_sim::WaterDrain& drain) { return drain.enabled; });
        const bool fixtureOpen = std::any_of(simulation.gates().begin(), simulation.gates().end(), [](const physics_sim::WaterGate& gate) { return gate.open; })
            || std::any_of(simulation.valves().begin(), simulation.valves().end(), [](const physics_sim::WaterValve& valve) { return valve.open; });
        physics_sim::AudioLoopSignals loopSignals;
        loopSignals.pour = std::clamp(static_cast<float>(emittedDelta) / 12.0f, 0.0f, 1.0f);
        loopSignals.flow = metrics.active_particles > 0
            ? std::clamp(static_cast<float>(std::log1p(std::max(0.0, metrics.kinetic_energy)) / std::log(1000001.0)), 0.0f, 1.0f)
            : 0.0f;
        loopSignals.impact = std::clamp(static_cast<float>(metrics.max_divergence_after_projection) * 0.02f, 0.0f, 1.0f);
        loopSignals.pump = pumpEnabled ? 0.55f : 0.0f;
        loopSignals.drain = drainEnabled ? std::clamp(static_cast<float>(outflowDelta) / 8.0f, 0.0f, 1.0f) : 0.0f;
        loopSignals.fixture = fixtureOpen && metrics.active_particles > 0 ? 0.25f : 0.0f;
        loopSignals.objective = metrics.active_sensors > 0 ? 0.32f : 0.0f;
        static_cast<void>(audioPlayer.update_continuous(loopSignals));
        if (metrics.active_sensors > 0 && previousActiveSensors == 0)
        {
            play_audio(physics_sim::AudioCue::DeviceTrigger);
        }
        const auto previousChallengeStatus = challengeEvaluator.progress().status;
        const auto& challengeProgress = challengeEvaluator.update(sceneMetadata.challenge, metrics, simulation.simulation_tick());
        if (sceneMetadata.challenge.has_value() && challengeProgress.status == physics_sim::ChallengeStatus::Complete
            && previousChallengeStatus != physics_sim::ChallengeStatus::Complete)
        {
            play_audio(physics_sim::AudioCue::ObjectiveComplete);
            set_status_message("CHALLENGE COMPLETE", StatusMessageKind::Success);
            logger.log("challenge complete: tick=" + std::to_string(simulation.simulation_tick())
                + " state_digest=" + simulation.state_digest());
        }
        else if (sceneMetadata.challenge.has_value() && challengeProgress.status == physics_sim::ChallengeStatus::Failed
            && previousChallengeStatus != physics_sim::ChallengeStatus::Failed)
        {
            play_audio(physics_sim::AudioCue::InvalidAction);
            set_status_message(challengeProgress.failure_reason, StatusMessageKind::Error);
            logger.log("challenge failed: tick=" + std::to_string(simulation.simulation_tick())
                + " reason=" + challengeProgress.failure_reason);
        }
        else if (!sceneMetadata.challenge.has_value() && metrics.objective_completed && !previousObjectiveCompleted)
        {
            play_audio(physics_sim::AudioCue::ObjectiveComplete);
        }

        previousActiveSensors = metrics.active_sensors;
        previousObjectiveCompleted = metrics.objective_completed;
        previousAudioEmitted = metrics.total_emitted;
        previousAudioOutflow = metrics.total_outflow;
    };

    if (startupAudioFeedback.has_value())
    {
        set_status_message(startupAudioFeedback->status_message, StatusMessageKind::Warning);
    }

    sync_scene_feedback_metrics();

    std::optional<std::string> pendingSettingsBindingName;

    const auto settings_entries = [&]() -> std::vector<physics_sim::SettingsMenuEntry>
    {
        return physics_sim::build_settings_menu_entries(userSettings);
    };

    const auto update_window_metrics = [&]()
    {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        viewport.set_window_size(windowWidth, windowHeight);
    };

    const auto persist_settings_or_report = [&]() -> bool
    {
        if (persist_user_settings())
        {
            return true;
        }

        const auto feedback = physics_sim::describe_settings_failure(settingsPath);
        logger.log(feedback.detail);
        play_audio(physics_sim::AudioCue::InvalidAction);
        set_status_message(feedback.status_message, StatusMessageKind::Error);
        return false;
    };

    const auto apply_settings_entry = [&](const physics_sim::SettingsMenuEntry& entry) -> bool
    {
        switch (entry.kind)
        {
        case physics_sim::SettingsMenuEntryKind::CycleWindowSize:
            userSettings.window_size = physics_sim::next_window_size(userSettings.window_size, entry.adjustment);
            if (!userSettings.fullscreen)
            {
                SDL_SetWindowSize(window, userSettings.window_size.width, userSettings.window_size.height);
            }
            update_window_metrics();
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(
                std::string{"WINDOW "} + physics_sim::window_size_label(userSettings.window_size),
                StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::ToggleFullscreen:
            userSettings.fullscreen = !userSettings.fullscreen;
            if (SDL_SetWindowFullscreen(window, userSettings.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
            {
                userSettings.fullscreen = !userSettings.fullscreen;
                logger.log(std::string{"fullscreen change failed: "} + SDL_GetError());
                play_audio(physics_sim::AudioCue::InvalidAction);
                set_status_message("FULLSCREEN FAILED", StatusMessageKind::Error);
                return false;
            }
            if (!userSettings.fullscreen)
            {
                SDL_SetWindowSize(window, userSettings.window_size.width, userSettings.window_size.height);
            }
            update_window_metrics();
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(userSettings.fullscreen ? "FULLSCREEN ON" : "FULLSCREEN OFF", StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::ToggleHelpOverlay:
            showHelp = !showHelp;
            userSettings.help_overlay_visible = showHelp;
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(showHelp ? "HELP ON" : "HELP OFF", StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::CycleVisualMode:
            visualMode = next_visual_mode(visualMode);
            userSettings.visual_mode = visualMode;
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"VISUAL "} + visual_mode_name(visualMode), StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::CycleSolverProfile:
            userSettings.solver_profile = next_solver_profile(userSettings.solver_profile);
            simulation.set_solver_settings(physics_sim::WaterSimulation2D::solver_settings_for_profile(userSettings.solver_profile));
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"SOLVER "} + solver_profile_name(userSettings.solver_profile), StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::ToggleHighContrast:
            userSettings.high_contrast = !userSettings.high_contrast;
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(userSettings.high_contrast ? "HIGH CONTRAST ON" : "HIGH CONTRAST OFF", StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::ToggleReducedMotion:
            userSettings.reduced_motion = !userSettings.reduced_motion;
            if (!persist_settings_or_report())
            {
                return false;
            }
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(userSettings.reduced_motion ? "REDUCED MOTION ON" : "REDUCED MOTION OFF", StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::ToggleAudioMute:
            userSettings.audio_muted = !userSettings.audio_muted;
            if (!persist_settings_or_report())
            {
                return false;
            }
            sync_audio_settings();
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(userSettings.audio_muted ? "AUDIO MUTED" : "AUDIO ON", StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::AdjustMasterVolume:
            userSettings.audio_master_volume = std::clamp(userSettings.audio_master_volume + entry.adjustment, 0, 100);
            if (!persist_settings_or_report())
            {
                return false;
            }
            sync_audio_settings();
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"MASTER VOLUME "} + std::to_string(userSettings.audio_master_volume), StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::AdjustEffectsVolume:
            userSettings.audio_effects_volume = std::clamp(userSettings.audio_effects_volume + entry.adjustment, 0, 100);
            if (!persist_settings_or_report())
            {
                return false;
            }
            sync_audio_settings();
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"EFFECTS VOLUME "} + std::to_string(userSettings.audio_effects_volume), StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::AdjustMusicVolume:
            userSettings.audio_music_volume = std::clamp(userSettings.audio_music_volume + entry.adjustment, 0, 100);
            if (!persist_settings_or_report())
            {
                return false;
            }
            sync_audio_settings();
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"MUSIC VOLUME "} + std::to_string(userSettings.audio_music_volume), StatusMessageKind::Info);
            return true;
        case physics_sim::SettingsMenuEntryKind::RemapBinding:
            pendingSettingsBindingName = entry.binding_name;
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(
                std::string{"PRESS NEW KEY FOR "} + physics_sim::binding_display_name(entry.binding_name),
                StatusMessageKind::Warning);
            return true;
        case physics_sim::SettingsMenuEntryKind::Back:
            return true;
        }

        return true;
    };

    const auto handle_pending_settings_binding = [&](SDL_Keycode keycode) -> bool
    {
        if (!pendingSettingsBindingName.has_value())
        {
            return false;
        }

        if (keycode == SDLK_ESCAPE)
        {
            pendingSettingsBindingName.reset();
            set_status_message("REMAP CANCELED", StatusMessageKind::Warning);
            play_audio(physics_sim::AudioCue::UiCancel);
            return true;
        }

        const std::string binding_name = *pendingSettingsBindingName;
        auto candidate = userSettings.input_bindings;
        if (!physics_sim::set_binding(candidate, binding_name, keycode))
        {
            set_status_message("REMAP FAILED", StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return true;
        }

        if (!physics_sim::validate_input_bindings(candidate))
        {
            set_status_message("BINDING CONFLICT", StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return true;
        }

        userSettings.input_bindings = candidate;
        pendingSettingsBindingName.reset();
        if (!persist_settings_or_report())
        {
            return true;
        }

        play_audio(physics_sim::AudioCue::UiConfirm);
        set_status_message(std::string{"BOUND "} + physics_sim::binding_display_name(binding_name), StatusMessageKind::Success);
        return true;
    };

    const auto save_scene_snapshot = [&](const fs::path& path, std::string_view success_message, bool announce, bool mark_tutorial) -> bool
    {
        const auto snapshot = physics_sim::capture_scene(simulation, sceneMetadata);
        std::error_code exists_ec;
        if (std::filesystem::exists(path, exists_ec))
        {
            const fs::path backup_path = physics_sim::backup_scene_path(path);
            std::error_code backup_ec;
            std::filesystem::copy_file(path, backup_path, std::filesystem::copy_options::overwrite_existing, backup_ec);
            if (backup_ec)
            {
                logger.log(std::string{"scene backup failed: "} + path.string() + " -> " + backup_path.string() + " reason=" + backup_ec.message());
            }
            else
            {
                logger.log(std::string{"scene backup ok: "} + path.string() + " -> " + backup_path.string());
            }
        }

        if (physics_sim::save_scene(path, snapshot))
        {
            logger.log(std::string{"scene save ok: "} + path.string());
            if (announce)
            {
                play_audio(physics_sim::AudioCue::Save);
                set_status_message(std::string{success_message}, StatusMessageKind::Success);
            }
            if (mark_tutorial && tutorialActive)
            {
                physics_sim::tutorial_mark_save_or_load(tutorialProgress);
            }
            refresh_save_browser_entries();
            return true;
        }

        const auto feedback = physics_sim::describe_scene_save_failure(path);
        logger.log(feedback.detail);
        if (announce)
        {
            play_audio(physics_sim::AudioCue::InvalidAction);
            set_status_message(feedback.status_message, StatusMessageKind::Error);
        }
        return false;
    };

    const auto save_autosave_scene = [&]() -> bool
    {
        return save_scene_snapshot(autosaveScenePath, "SAVED AUTOSAVE", false, true);
    };

    const auto save_named_scene = [&]() -> bool
    {
        const fs::path namedScenePath = physics_sim::named_save_path(saveDirectory, currentScenePath, sceneMetadata);
        const std::string sceneLabel = scene_label(currentScenePath, sceneMetadata);
        if (!save_scene_snapshot(namedScenePath, std::string{"SAVED "} + sceneLabel, false, false))
        {
            play_audio(physics_sim::AudioCue::InvalidAction);
            set_status_message("SAVE FAILED", StatusMessageKind::Error);
            return false;
        }

        currentScenePath = namedScenePath;
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
        if (!save_autosave_scene())
        {
            play_audio(physics_sim::AudioCue::InvalidAction);
            set_status_message("AUTOSAVE FAILED", StatusMessageKind::Error);
            return false;
        }

        play_audio(physics_sim::AudioCue::Save);
        set_status_message(std::string{"SAVED "} + sceneLabel, StatusMessageKind::Success);
        return true;
    };

    const auto load_gallery_scene = [&](std::size_t index) -> bool
    {
        stop_pointer_water();
        if (index >= galleryEntries.size())
        {
            return false;
        }

        const fs::path& path = galleryEntries[index].scene_path;
        physics_sim::PlayerFeedback feedback;
        if (!load_scene_from_file(
                path,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                &feedback,
                userSettings.solver_profile,
                options.initialSolverProfile))
        {
            logger.log(feedback.detail);
            set_status_message(feedback.status_message, StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return false;
        }

        sync_solver_profile_from_simulation();
        currentScenePath = path;
        currentGalleryIndex = index;
        sync_scene_feedback_metrics();
        refresh_save_browser_entries();
        play_audio(physics_sim::AudioCue::Load);
        set_status_message(std::string{"LOADED "} + scene_label(path, sceneMetadata), StatusMessageKind::Success);
        if (tutorialActive)
        {
            physics_sim::tutorial_mark_gallery_browsed(tutorialProgress);
        }
        static_cast<void>(save_autosave_scene());
        return true;
    };

    const auto load_autosave_or_demo = [&]() -> bool
    {
        stop_pointer_water();
        physics_sim::PlayerFeedback feedback;
        if (!load_scene_from_file(
                autosaveScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                &feedback,
                userSettings.solver_profile,
                options.initialSolverProfile))
        {
            logger.log(feedback.detail);
            static_cast<void>(restore_demo_scene(
                demoScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                userSettings.solver_profile,
                options.initialSolverProfile));
            sync_solver_profile_from_simulation();
            currentScenePath = demoScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            sync_scene_feedback_metrics();
            refresh_save_browser_entries();
            play_audio(physics_sim::AudioCue::Load);
            set_status_message("LOADED DEMO", StatusMessageKind::Success);
            if (tutorialActive)
            {
                physics_sim::tutorial_mark_save_or_load(tutorialProgress);
            }
            static_cast<void>(save_autosave_scene());
            return false;
        }

        sync_solver_profile_from_simulation();
        currentScenePath = autosaveScenePath;
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
        sync_scene_feedback_metrics();
        refresh_save_browser_entries();
        play_audio(physics_sim::AudioCue::Load);
        set_status_message("LOADED AUTOSAVE", StatusMessageKind::Success);
        if (tutorialActive)
        {
            physics_sim::tutorial_mark_save_or_load(tutorialProgress);
        }
        static_cast<void>(save_autosave_scene());
        return true;
    };

    const auto load_save_browser_entry = [&](std::size_t index) -> bool
    {
        stop_pointer_water();
        refresh_save_browser_entries();
        if (index >= saveBrowserEntries.size())
        {
            return false;
        }

        const auto entry = saveBrowserEntries[index];
        if (entry.kind == physics_sim::SaveBrowserEntryKind::Back)
        {
            sessionShellState.screen = sessionShellState.return_screen;
            sessionShellState.selection = 0;
            update_shell_pause_state();
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiCancel);
            set_status_message("BACK", StatusMessageKind::Info);
            return true;
        }

        if (!entry.loadable)
        {
            const auto feedback = physics_sim::describe_scene_load_failure(entry.path);
            logger.log(feedback.detail);
            set_status_message(feedback.status_message, StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return false;
        }

        physics_sim::PlayerFeedback feedback;
        if (!load_scene_from_file(
                entry.path,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                &feedback,
                userSettings.solver_profile,
                options.initialSolverProfile))
        {
            logger.log(feedback.detail);
            set_status_message(feedback.status_message, StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return false;
        }

        sync_solver_profile_from_simulation();
        currentScenePath = entry.path;
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
        sync_scene_feedback_metrics();
        refresh_save_browser_entries();
        play_audio(physics_sim::AudioCue::Load);
        set_status_message(std::string{"LOADED "} + (entry.label.empty() ? scene_label(entry.path, sceneMetadata) : entry.label), StatusMessageKind::Success);
        if (tutorialActive)
        {
            physics_sim::tutorial_mark_save_or_load(tutorialProgress);
        }
        static_cast<void>(save_autosave_scene());
        return true;
    };

    const auto clear_current_scene = [&]()
    {
        stop_pointer_water();
        controller.reset_scene();
        simulationState.reset();
        stepDriver.reset();
        sceneMetadata = {};
        sync_scene_feedback_metrics();
        static_cast<void>(save_autosave_scene());
        play_audio(physics_sim::AudioCue::Reset);
        set_status_message("CLEARED SCENE", StatusMessageKind::Success);
        if (tutorialActive)
        {
            physics_sim::tutorial_mark_reset_or_retry(tutorialProgress);
        }
    };

    const auto mark_tutorial_seen = [&]()
    {
        if (save_tutorial_seen_state(tutorialStatePath, true))
        {
            logger.log("tutorial state save ok");
        }
        else
        {
            logger.log("tutorial state save failed");
        }
    };

    const auto complete_tutorial_if_needed = [&]()
    {
        if (tutorialActive && physics_sim::tutorial_is_complete(tutorialProgress))
        {
            tutorialActive = false;
            mark_tutorial_seen();
            play_audio(physics_sim::AudioCue::ObjectiveComplete);
            set_status_message("TUTORIAL COMPLETE", StatusMessageKind::Success);
        }
    };

    const auto start_tutorial_session = [&]() -> bool
    {
        stop_pointer_water();
        tutorialActive = true;
        tutorialProgress = {};
        sessionShellState.screen = physics_sim::SessionShellScreen::Playing;
        sessionShellState.selection = 0;
        stepDriver.set_paused(false);
        panning = false;
        painting = false;
        paintingTool.reset();
        paintingSolidCount = 0;

        physics_sim::PlayerFeedback feedback;
        if (!fs::exists(tutorialScenePath))
        {
            const auto package_feedback = physics_sim::describe_package_content_failure("tutorial scene missing");
            logger.log(package_feedback.detail);
            static_cast<void>(restore_demo_scene(
                starterScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                userSettings.solver_profile,
                options.initialSolverProfile));
            sync_solver_profile_from_simulation();
            currentScenePath = starterScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            sync_scene_feedback_metrics();
            refresh_save_browser_entries();
            set_status_message(package_feedback.status_message, StatusMessageKind::Error);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return false;
        }

        if (!load_scene_from_file(
                tutorialScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                &feedback,
                userSettings.solver_profile,
                options.initialSolverProfile))
        {
            logger.log(feedback.detail);
            static_cast<void>(restore_demo_scene(
                starterScenePath,
                simulation,
                viewport,
                simulationState,
                stepDriver,
                controller,
                &sceneMetadata,
                &logger,
                userSettings.solver_profile,
                options.initialSolverProfile));
            sync_solver_profile_from_simulation();
            currentScenePath = starterScenePath;
            currentGalleryIndex = gallery_index_for_path(currentScenePath);
            sync_scene_feedback_metrics();
            refresh_save_browser_entries();
            set_status_message("TUTORIAL FALLBACK", StatusMessageKind::Warning);
            play_audio(physics_sim::AudioCue::InvalidAction);
            return false;
        }

        sync_solver_profile_from_simulation();
        currentScenePath = tutorialScenePath;
        currentGalleryIndex = gallery_index_for_path(currentScenePath);
        sync_scene_feedback_metrics();
        refresh_save_browser_entries();
        static_cast<void>(save_autosave_scene());
        set_status_message("TUTORIAL STARTED", StatusMessageKind::Success);
        return true;
    };

    const auto skip_tutorial_session = [&]()
    {
        if (tutorialActive)
        {
            tutorialActive = false;
            mark_tutorial_seen();
            set_status_message("TUTORIAL SKIPPED", StatusMessageKind::Warning);
        }
    };

    const auto perform_action = [&](physics_sim::Action action, physics_sim::AudioCue cue) -> std::string
    {
        stop_pointer_water();
        std::string actionFeedback;
        apply_action(
            action,
            stepDriver,
            simulationState,
            controller,
            simulation,
            viewport,
            currentScenePath,
            &sceneMetadata,
            &actionFeedback,
            &logger,
            tutorialActive ? &tutorialProgress : nullptr,
            userSettings.solver_profile,
            options.initialSolverProfile);

        if (action == physics_sim::Action::Reset || action == physics_sim::Action::ResetFluid)
        {
            sync_solver_profile_from_simulation();
            sync_scene_feedback_metrics();
            static_cast<void>(save_autosave_scene());
        }

        if (!actionFeedback.empty())
        {
            set_status_message(std::move(actionFeedback));
        }

        play_audio(cue);
        return actionFeedback;
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

                const auto cue = *action == physics_sim::Action::Reset || *action == physics_sim::Action::ResetFluid
                    ? physics_sim::AudioCue::Reset
                    : physics_sim::AudioCue::UiConfirm;
                static_cast<void>(perform_action(*action, cue));
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

                set_current_tool(*tool);
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

                if (controller.has_selected_pump())
                {
                    static_cast<void>(controller.set_selected_pump_direction({*x, *y}));
                }
                else if (controller.has_selected_fixture())
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

                if (controller.has_selected_pump())
                {
                    static_cast<void>(controller.set_selected_pump_strength(*speed));
                }
                else if (controller.has_selected_fixture())
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

                bool placed = false;
                switch (controller.tool())
                {
                case physics_sim::SceneTool::DirectionalEmitter:
                case physics_sim::SceneTool::OmniEmitter: placed = controller.place_fixture({*x, *y}); break;
                case physics_sim::SceneTool::Gate: placed = controller.place_gate({*x, *y}); break;
                case physics_sim::SceneTool::Sensor: placed = controller.place_sensor({*x, *y}); break;
                case physics_sim::SceneTool::Drain: placed = controller.place_drain({*x, *y}); break;
                case physics_sim::SceneTool::Pump: placed = controller.place_pump({*x, *y}); break;
                case physics_sim::SceneTool::Valve: placed = controller.place_valve({*x, *y}); break;
                default: break;
                }
                if (!placed)
                {
                    logger.log("fatal replay place invalid placement");
                    running = false;
                    return false;
                }
                continue;
            }

            if (event.command == "select")
            {
                const auto x = parse_float_token(event.arguments[0]);
                const auto y = parse_float_token(event.arguments[1]);
                if (!x.has_value() || !y.has_value()) { logger.log("fatal replay select failed to parse"); running = false; return false; }
                const physics_sim::Vec2 point{*x, *y};
                const float radius = simulation.grid().cell_size() * 0.65f;
                bool selected = false;
                switch (controller.tool())
                {
                case physics_sim::SceneTool::DirectionalEmitter:
                case physics_sim::SceneTool::OmniEmitter: selected = controller.select_fixture_at(point, radius); break;
                case physics_sim::SceneTool::Gate: selected = controller.select_gate_at(point, radius); break;
                case physics_sim::SceneTool::Sensor: selected = controller.select_sensor_at(point); break;
                case physics_sim::SceneTool::Drain: selected = controller.select_drain_at(point); break;
                case physics_sim::SceneTool::Pump: selected = controller.select_pump_at(point); break;
                case physics_sim::SceneTool::Valve: selected = controller.select_valve_at(point); break;
                default: break;
                }
                if (!selected) { logger.log("fatal replay select missed device"); running = false; return false; }
                continue;
            }

            if (event.command == "toggle-selected")
            {
                const bool toggled = controller.toggle_selected_fixture_enabled()
                    || controller.toggle_selected_gate_open()
                    || controller.toggle_selected_sensor_enabled()
                    || controller.toggle_selected_drain_enabled()
                    || controller.toggle_selected_pump_enabled()
                    || controller.toggle_selected_valve_open();
                if (!toggled) { logger.log("fatal replay toggle had no selection"); running = false; return false; }
                continue;
            }

            if (event.command == "delete-selected")
            {
                const bool deleted = controller.delete_selected_fixture()
                    || controller.delete_selected_gate()
                    || controller.delete_selected_sensor()
                    || controller.delete_selected_drain()
                    || controller.delete_selected_pump()
                    || controller.delete_selected_valve();
                if (!deleted) { logger.log("fatal replay delete had no selection"); running = false; return false; }
                continue;
            }

            if (event.command == "clear-emitters")
            {
                simulation.emitters().clear();
                controller.clear_selection();
                logger.log("replay cleared emitters");
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
        if (pointerWaterActive)
        {
            sync_pointer_water_emitter();
            simulation.step(step.count(), pointerWaterEmitters);
        }
        else
        {
            simulation.step(step.count());
        }
        ++replayTick;
        const bool replay_ok = process_replay_actions(replayTick);
        if (replay_ok)
        {
            update_scene_feedback_audio();
        }
        return replay_ok;
    };

    const auto open_pause_menu = [&]()
    {
        sessionShellState.screen = physics_sim::SessionShellScreen::PauseMenu;
        sessionShellState.return_screen = physics_sim::SessionShellScreen::Playing;
        sessionShellState.selection = 0;
        if (tutorialActive)
        {
            physics_sim::tutorial_mark_pause_opened(tutorialProgress);
        }
        update_shell_pause_state();
        panning = false;
        painting = false;
        play_audio(physics_sim::AudioCue::UiCancel);
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
            play_audio(physics_sim::AudioCue::UiConfirm);
            panning = false;
            painting = false;
            break;
        case physics_sim::SessionShellCommandKind::StartTutorial:
            play_audio(physics_sim::AudioCue::UiConfirm);
            static_cast<void>(start_tutorial_session());
            break;
        case physics_sim::SessionShellCommandKind::SwitchToLab:
            applicationExitCode = physics_sim::switch_to_lab_exit_code;
            running = false;
            return true;
        case physics_sim::SessionShellCommandKind::Resume:
            play_audio(physics_sim::AudioCue::UiConfirm);
            if (tutorialActive)
            {
                physics_sim::tutorial_mark_pause_resumed(tutorialProgress);
            }
            panning = false;
            painting = false;
            break;
        case physics_sim::SessionShellCommandKind::RetryCurrentScene:
            static_cast<void>(perform_action(physics_sim::Action::Reset, physics_sim::AudioCue::Reset));
            break;
        case physics_sim::SessionShellCommandKind::ResetFluid:
            static_cast<void>(perform_action(physics_sim::Action::ResetFluid, physics_sim::AudioCue::Reset));
            break;
        case physics_sim::SessionShellCommandKind::ClearScene:
            clear_current_scene();
            break;
        case physics_sim::SessionShellCommandKind::SaveScene:
            static_cast<void>(save_named_scene());
            break;
        case physics_sim::SessionShellCommandKind::LoadScene:
            static_cast<void>(load_autosave_or_demo());
            break;
        case physics_sim::SessionShellCommandKind::OpenSaveBrowser:
            refresh_save_browser_entries();
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiSelect);
            break;
        case physics_sim::SessionShellCommandKind::ReturnToMainMenu:
            skip_tutorial_session();
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiCancel);
            break;
        case physics_sim::SessionShellCommandKind::OpenSceneBrowser:
        case physics_sim::SessionShellCommandKind::OpenSettings:
        case physics_sim::SessionShellCommandKind::OpenAbout:
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiSelect);
            break;
        case physics_sim::SessionShellCommandKind::LoadSceneAtIndex:
            if (!command.scene_index.has_value() || !load_gallery_scene(*command.scene_index))
            {
                sessionShellState = previous_shell_state;
                set_status_message("LOAD FAILED", StatusMessageKind::Error);
                play_audio(physics_sim::AudioCue::InvalidAction);
                update_shell_pause_state();
                return true;
            }
            break;
        case physics_sim::SessionShellCommandKind::LoadSaveAtIndex:
            if (!command.scene_index.has_value() || !load_save_browser_entry(*command.scene_index))
            {
                sessionShellState = previous_shell_state;
                set_status_message("LOAD FAILED", StatusMessageKind::Error);
                play_audio(physics_sim::AudioCue::InvalidAction);
                update_shell_pause_state();
                return true;
            }
            break;
        case physics_sim::SessionShellCommandKind::ToggleHelpOverlay:
            showHelp = !showHelp;
            userSettings.help_overlay_visible = showHelp;
            static_cast<void>(persist_settings_or_report());
            play_audio(physics_sim::AudioCue::UiConfirm);
            set_status_message(showHelp ? "HELP ON" : "HELP OFF", StatusMessageKind::Info);
            break;
        case physics_sim::SessionShellCommandKind::CycleVisualMode:
            visualMode = next_visual_mode(visualMode);
            userSettings.visual_mode = visualMode;
            static_cast<void>(persist_settings_or_report());
            play_audio(physics_sim::AudioCue::UiSelect);
            set_status_message(std::string{"VISUAL "} + visual_mode_name(visualMode), StatusMessageKind::Info);
            break;
        case physics_sim::SessionShellCommandKind::Back:
            play_audio(physics_sim::AudioCue::UiCancel);
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

        if (handle_pending_settings_binding(keycode))
        {
            return true;
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
            const auto previous_shell_screen = sessionShellState.screen;
            physics_sim::session_shell_back(sessionShellState);
            if (tutorialActive && previous_shell_screen == physics_sim::SessionShellScreen::PauseMenu && sessionShellState.screen == physics_sim::SessionShellScreen::Playing)
            {
                physics_sim::tutorial_mark_pause_resumed(tutorialProgress);
            }
            update_shell_pause_state();
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiCancel);
            complete_tutorial_if_needed();
            return true;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::PauseMenu && keycode == userSettings.input_bindings.pause_resume)
        {
            sessionShellState.screen = physics_sim::SessionShellScreen::Playing;
            sessionShellState.selection = 0;
            if (tutorialActive)
            {
                physics_sim::tutorial_mark_pause_resumed(tutorialProgress);
            }
            update_shell_pause_state();
            panning = false;
            painting = false;
            play_audio(physics_sim::AudioCue::UiConfirm);
            complete_tutorial_if_needed();
            return true;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::SceneBrowser
            && (keycode == SDLK_LEFT || keycode == SDLK_RIGHT || keycode == SDLK_TAB))
        {
            const int direction = keycode == SDLK_LEFT ? -1 : 1;
            galleryCategoryFilter = (galleryCategoryFilter + direction + 4) % 4;
            rebuild_gallery_filter();
            sessionShellState.selection = 0;
            const std::array<std::string_view, 4> filterNames{"ALL", "LEARN", "SANDBOX", "CHALLENGES"};
            set_status_message("GALLERY FILTER " + std::string{filterNames[static_cast<std::size_t>(galleryCategoryFilter)]}, StatusMessageKind::Info);
            play_audio(physics_sim::AudioCue::UiSelect);
            return true;
        }

        const auto option_count = [&]() -> std::size_t
        {
            if (sessionShellState.screen == physics_sim::SessionShellScreen::Settings)
            {
                return settings_entries().size();
            }

            return physics_sim::session_shell_option_count(
                sessionShellState.screen,
                galleryEntries.size(),
                saveBrowserEntries.size());
        };

        const std::size_t optionCount = option_count();
        if (optionCount == 0)
        {
            return true;
        }

        if (keycode == SDLK_UP || keycode == SDLK_w)
        {
            physics_sim::session_shell_wrap_selection(sessionShellState, optionCount, -1);
            play_audio(physics_sim::AudioCue::UiSelect);
            return true;
        }

        if (keycode == SDLK_DOWN || keycode == SDLK_s)
        {
            physics_sim::session_shell_wrap_selection(sessionShellState, optionCount, 1);
            play_audio(physics_sim::AudioCue::UiSelect);
            return true;
        }

        if (keycode != SDLK_RETURN && keycode != SDLK_KP_ENTER)
        {
            return true;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::Settings)
        {
            const auto entries = settings_entries();
            if (sessionShellState.selection < entries.size())
            {
                if (entries[sessionShellState.selection].kind == physics_sim::SettingsMenuEntryKind::Back)
                {
                    physics_sim::session_shell_back(sessionShellState);
                    update_shell_pause_state();
                    panning = false;
                    painting = false;
                    play_audio(physics_sim::AudioCue::UiCancel);
                }
                else
                {
                    static_cast<void>(apply_settings_entry(entries[sessionShellState.selection]));
                }
            }
            return true;
        }

        const auto previousShellState = sessionShellState;
        const auto command = physics_sim::session_shell_activate(sessionShellState, galleryEntries.size(), saveBrowserEntries.size());
        static_cast<void>(execute_session_shell_command(command, previousShellState));
        complete_tutorial_if_needed();
        return true;
    };

    const auto handle_session_shell_mouse_move = [&](int mouse_x, int mouse_y) -> bool
    {
        if (sessionShellBypassed)
        {
            return false;
        }

        if (pendingSettingsBindingName.has_value())
        {
            return true;
        }

        if (sessionShellState.screen == physics_sim::SessionShellScreen::Playing)
        {
            return false;
        }

        const auto hovered_index = session_shell_option_index_at_point(
            mouse_x,
            mouse_y,
            windowWidth,
            windowHeight,
            sessionShellState,
            galleryEntries,
            saveBrowserEntries,
            userSettings);
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

        if (pendingSettingsBindingName.has_value())
        {
            return true;
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
                play_audio(physics_sim::AudioCue::UiCancel);
                return true;
            }

            return true;
        }

        const auto hovered_index = session_shell_option_index_at_point(
            mouse_x,
            mouse_y,
            windowWidth,
            windowHeight,
            sessionShellState,
            galleryEntries,
            saveBrowserEntries,
            userSettings);
        if (!hovered_index.has_value())
        {
            return true;
        }

        sessionShellState.selection = *hovered_index;
        if (sessionShellState.screen == physics_sim::SessionShellScreen::Settings)
        {
            const auto entries = settings_entries();
            if (sessionShellState.selection < entries.size())
            {
                if (entries[sessionShellState.selection].kind == physics_sim::SettingsMenuEntryKind::Back)
                {
                    physics_sim::session_shell_back(sessionShellState);
                    update_shell_pause_state();
                    panning = false;
                    painting = false;
                    play_audio(physics_sim::AudioCue::UiCancel);
                }
                else
                {
                    static_cast<void>(apply_settings_entry(entries[sessionShellState.selection]));
                }
            }
            return true;
        }

        const auto previousShellState = sessionShellState;
        const auto command = physics_sim::session_shell_activate(sessionShellState, galleryEntries.size(), saveBrowserEntries.size());
        static_cast<void>(execute_session_shell_command(command, previousShellState));
        complete_tutorial_if_needed();
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
            case SDL_AUDIODEVICEREMOVED:
                if (event.adevice.iscapture == 0)
                {
                    audioPlayer.handle_device_removed(event.adevice.which);
                    logger.log("audio output device removed; simulation continues silently");
                    set_status_message("AUDIO DEVICE LOST", StatusMessageKind::Warning);
                }
                break;
            case SDL_AUDIODEVICEADDED:
                if (event.adevice.iscapture == 0 && !audioPlayer.available() && !options.disableAudio)
                {
                    std::string recoveryError;
                    if (audioPlayer.recover(&recoveryError))
                    {
                        logger.log("audio output device recovered");
                        set_status_message("AUDIO RESTORED", StatusMessageKind::Success);
                    }
                    else
                    {
                        logger.log("audio recovery failed: " + recoveryError);
                    }
                }
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

                const SDL_Keymod modifiers = static_cast<SDL_Keymod>(event.key.keysym.mod);
                const auto& bindings = userSettings.input_bindings;

                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                    break;
                }

                if (event.key.keysym.sym == bindings.pause_resume)
                {
                    static_cast<void>(perform_action(physics_sim::Action::TogglePause, physics_sim::AudioCue::UiConfirm));
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.step_once)
                {
                    static_cast<void>(perform_action(physics_sim::Action::StepOnce, physics_sim::AudioCue::UiSelect));
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.reset)
                {
                    static_cast<void>(perform_action(physics_sim::Action::Reset, physics_sim::AudioCue::Reset));
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.reset_fluid)
                {
                    static_cast<void>(perform_action(physics_sim::Action::ResetFluid, physics_sim::AudioCue::Reset));
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.tool_prev || (event.key.keysym.sym == SDLK_TAB && (modifiers & KMOD_SHIFT) != 0))
                {
                    set_current_tool(next_sandbox_tool(controller.tool(), -1, advancedToolsVisible));
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message(std::string{"TOOL "} + tool_name(controller.tool()), StatusMessageKind::Info);
                    break;
                }

                if (event.key.keysym.sym == bindings.tool_next || (event.key.keysym.sym == SDLK_TAB && (modifiers & KMOD_SHIFT) == 0))
                {
                    set_current_tool(next_sandbox_tool(controller.tool(), 1, advancedToolsVisible));
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message(std::string{"TOOL "} + tool_name(controller.tool()), StatusMessageKind::Info);
                    break;
                }

                if (event.key.keysym.sym == bindings.undo)
                {
                    static_cast<void>(controller.undo_scene_edit());
                    break;
                }

                if (event.key.keysym.sym == bindings.redo)
                {
                    static_cast<void>(controller.redo_scene_edit());
                    break;
                }

                if (event.key.keysym.sym == bindings.save_scene)
                {
                    static_cast<void>(save_named_scene());
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.load_scene)
                {
                    static_cast<void>(load_autosave_or_demo());
                    complete_tutorial_if_needed();
                    break;
                }

                if (event.key.keysym.sym == bindings.delete_selection)
                {
                    bool deleted = controller.delete_selected_fixture();
                    deleted = controller.delete_selected_gate() || deleted;
                    deleted = controller.delete_selected_valve() || deleted;
                    deleted = controller.delete_selected_sensor() || deleted;
                    deleted = controller.delete_selected_drain() || deleted;
                    deleted = controller.delete_selected_pump() || deleted;
                    if (deleted)
                    {
                        play_audio(physics_sim::AudioCue::UiConfirm);
                        set_status_message("DELETED DEVICE", StatusMessageKind::Warning);
                    }
                    else
                    {
                        clear_current_scene();
                    }
                    break;
                }

                if (event.key.keysym.sym == bindings.toggle_fullscreen)
                {
                    userSettings.fullscreen = !userSettings.fullscreen;
                    if (SDL_SetWindowFullscreen(window, userSettings.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
                    {
                        userSettings.fullscreen = !userSettings.fullscreen;
                        set_status_message("FULLSCREEN FAILED", StatusMessageKind::Error);
                        logger.log(std::string{"fullscreen change failed: "} + SDL_GetError());
                        play_audio(physics_sim::AudioCue::InvalidAction);
                    }
                    else
                    {
                        if (!userSettings.fullscreen)
                        {
                            SDL_SetWindowSize(window, userSettings.window_size.width, userSettings.window_size.height);
                        }
                        update_window_metrics();
                        static_cast<void>(persist_settings_or_report());
                        play_audio(physics_sim::AudioCue::UiConfirm);
                        set_status_message(userSettings.fullscreen ? "FULLSCREEN ON" : "FULLSCREEN OFF", StatusMessageKind::Info);
                    }
                    break;
                }

                if (event.key.keysym.sym == bindings.toggle_help)
                {
                    showHelp = !showHelp;
                    userSettings.help_overlay_visible = showHelp;
                    static_cast<void>(persist_settings_or_report());
                    play_audio(physics_sim::AudioCue::UiConfirm);
                    set_status_message(showHelp ? "HELP ON" : "HELP OFF", StatusMessageKind::Info);
                    break;
                }

                if (event.key.keysym.sym == bindings.cycle_visual_mode)
                {
                    visualMode = next_visual_mode(visualMode);
                    userSettings.visual_mode = visualMode;
                    static_cast<void>(persist_settings_or_report());
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message(std::string{"VISUAL "} + visual_mode_name(visualMode), StatusMessageKind::Info);
                    break;
                }

                if (event.key.keysym.sym == SDLK_l)
                {
                    applicationExitCode = physics_sim::switch_to_lab_exit_code;
                    running = false;
                    break;
                }

                if (event.key.keysym.sym == SDLK_a && (modifiers & KMOD_CTRL) == 0)
                {
                    advancedToolsVisible = !advancedToolsVisible;
                    if (!advancedToolsVisible && static_cast<int>(controller.tool()) > 2)
                    {
                        set_current_tool(physics_sim::SceneTool::PointerWater);
                    }
                    set_status_message(advancedToolsVisible ? "ADVANCED TOOLS OPEN" : "COMPACT TOOLS", StatusMessageKind::Info);
                    play_audio(physics_sim::AudioCue::UiConfirm);
                    break;
                }

                switch (event.key.keysym.sym)
                {
                case SDLK_PAGEUP:
                {
                    if (!galleryEntries.empty())
                    {
                        const std::size_t nextIndex = currentGalleryIndex.has_value()
                            ? ((*currentGalleryIndex + galleryEntries.size() - 1) % galleryEntries.size())
                            : (galleryEntries.size() - 1);
                        static_cast<void>(load_gallery_scene(nextIndex));
                    }
                    complete_tutorial_if_needed();
                    break;
                }
                case SDLK_PAGEDOWN:
                {
                    if (!galleryEntries.empty())
                    {
                        const std::size_t nextIndex = currentGalleryIndex.has_value()
                            ? ((*currentGalleryIndex + 1) % galleryEntries.size())
                            : 0;
                        static_cast<void>(load_gallery_scene(nextIndex));
                    }
                    complete_tutorial_if_needed();
                    break;
                }
                case SDLK_0:
                    set_current_tool(physics_sim::SceneTool::PointerWater);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL POUR", StatusMessageKind::Info);
                    break;
                case SDLK_1:
                    set_current_tool(physics_sim::SceneTool::PaintWall);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL WALL", StatusMessageKind::Info);
                    break;
                case SDLK_2:
                    set_current_tool(physics_sim::SceneTool::EraseWall);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL ERASE", StatusMessageKind::Info);
                    break;
                case SDLK_3:
                    set_current_tool(physics_sim::SceneTool::DirectionalEmitter);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL HOSE", StatusMessageKind::Info);
                    break;
                case SDLK_4:
                    set_current_tool(physics_sim::SceneTool::OmniEmitter);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL OMNI", StatusMessageKind::Info);
                    break;
                case SDLK_5:
                    set_current_tool(physics_sim::SceneTool::Gate);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL GATE", StatusMessageKind::Info);
                    break;
                case SDLK_6:
                    set_current_tool(physics_sim::SceneTool::Sensor);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL SENSOR", StatusMessageKind::Info);
                    break;
                case SDLK_7:
                    set_current_tool(physics_sim::SceneTool::Drain);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL DRAIN", StatusMessageKind::Info);
                    break;
                case SDLK_8:
                    set_current_tool(physics_sim::SceneTool::Pump);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL PUMP", StatusMessageKind::Info);
                    break;
                case SDLK_9:
                    set_current_tool(physics_sim::SceneTool::Valve);
                    play_audio(physics_sim::AudioCue::UiSelect);
                    set_status_message("TOOL VALVE", StatusMessageKind::Info);
                    break;
                default:
                    handle_fixture_edit_key(
                        controller,
                        event.key.keysym.sym,
                        modifiers,
                        simulation.grid().cell_size(),
                        bindings,
                        tutorialActive ? &tutorialProgress : nullptr);
                    complete_tutorial_if_needed();
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
                    if (tool_palette_toggle_hit(event.button.x, event.button.y))
                    {
                        advancedToolsVisible = !advancedToolsVisible;
                        if (!advancedToolsVisible && static_cast<int>(controller.tool()) > 2)
                        {
                            set_current_tool(physics_sim::SceneTool::PointerWater);
                        }
                        play_audio(physics_sim::AudioCue::UiConfirm);
                        set_status_message(advancedToolsVisible ? "ADVANCED TOOLS OPEN" : "COMPACT TOOLS", StatusMessageKind::Info);
                        break;
                    }
                    if (const auto paletteTool = tool_palette_hit(event.button.x, event.button.y, advancedToolsVisible); paletteTool.has_value())
                    {
                        set_current_tool(*paletteTool);
                        play_audio(physics_sim::AudioCue::UiSelect);
                        set_status_message(std::string{"TOOL "} + tool_name(controller.tool()), StatusMessageKind::Info);
                        break;
                    }
                    const physics_sim::Vec2 world = viewport.window_to_world(mouseScreen);
                    if (controller.tool() == physics_sim::SceneTool::PointerWater)
                    {
                        start_pointer_water();
                        if (tutorialActive)
                        {
                            physics_sim::tutorial_mark_water_poured(tutorialProgress);
                        }
                        complete_tutorial_if_needed();
                        break;
                    }
                    else if (controller.tool() == physics_sim::SceneTool::PaintWall || controller.tool() == physics_sim::SceneTool::EraseWall)
                    {
                        paintingTool = controller.tool();
                        paintingSolidCount = 0;
                        for (std::size_t y = 0; y < simulation.grid().height(); ++y)
                        {
                            for (std::size_t x = 0; x < simulation.grid().width(); ++x)
                            {
                                if (simulation.grid().solid(x, y))
                                {
                                    ++paintingSolidCount;
                                }
                            }
                        }
                    }
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
                                set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                                play_audio(physics_sim::AudioCue::InvalidAction);
                            }
                            else if (tutorialActive)
                            {
                                physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                            else
                            {
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Gate)
                    {
                        if (!controller.select_gate_at(world, simulation.grid().cell_size() * 0.65f))
                        {
                            if (!controller.place_gate(world))
                            {
                                set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                                play_audio(physics_sim::AudioCue::InvalidAction);
                            }
                            else if (tutorialActive)
                            {
                                physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                            else
                            {
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Sensor)
                    {
                        if (!controller.select_sensor_at(world))
                        {
                            if (!controller.place_sensor(world))
                            {
                                set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                                play_audio(physics_sim::AudioCue::InvalidAction);
                            }
                            else if (tutorialActive)
                            {
                                physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                            else
                            {
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Drain)
                    {
                        if (!controller.select_drain_at(world) && !controller.place_drain(world))
                        {
                            set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                            play_audio(physics_sim::AudioCue::InvalidAction);
                        }
                        else if (tutorialActive)
                        {
                            physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                            play_audio(physics_sim::AudioCue::UiConfirm);
                            static_cast<void>(save_autosave_scene());
                        }
                        else
                        {
                            play_audio(physics_sim::AudioCue::UiConfirm);
                            static_cast<void>(save_autosave_scene());
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Pump)
                    {
                        if (!controller.select_pump_at(world) && !controller.place_pump(world))
                        {
                            set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                            play_audio(physics_sim::AudioCue::InvalidAction);
                        }
                        else if (tutorialActive)
                        {
                            physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                            play_audio(physics_sim::AudioCue::UiConfirm);
                            static_cast<void>(save_autosave_scene());
                        }
                        else
                        {
                            play_audio(physics_sim::AudioCue::UiConfirm);
                            static_cast<void>(save_autosave_scene());
                        }
                    }
                    else if (controller.tool() == physics_sim::SceneTool::Valve)
                    {
                        if (!controller.select_valve_at(world))
                        {
                            if (!controller.place_valve(world))
                            {
                                set_status_message("INVALID PLACEMENT", StatusMessageKind::Error);
                                play_audio(physics_sim::AudioCue::InvalidAction);
                            }
                            else if (tutorialActive)
                            {
                                physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                            else
                            {
                                play_audio(physics_sim::AudioCue::UiConfirm);
                                static_cast<void>(save_autosave_scene());
                            }
                        }
                    }
                    else
                    {
                        const bool placed = controller.place_fixture(world);
                        if (placed)
                        {
                            if (tutorialActive)
                            {
                                physics_sim::tutorial_mark_fixture_placed(tutorialProgress);
                            }
                            play_audio(physics_sim::AudioCue::UiConfirm);
                            static_cast<void>(save_autosave_scene());
                        }
                        else
                        {
                            play_audio(physics_sim::AudioCue::InvalidAction);
                        }
                    }
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    controller.clear_selection();
                    play_audio(physics_sim::AudioCue::UiCancel);
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = true;
                }
                complete_tutorial_if_needed();
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
                    stop_pointer_water();
                    if (painting)
                    {
                        controller.end_stroke(viewport.window_to_world(mouseScreen));
                        if (tutorialActive && paintingTool.has_value())
                        {
                            std::size_t solidCount = 0;
                            for (std::size_t y = 0; y < simulation.grid().height(); ++y)
                            {
                                for (std::size_t x = 0; x < simulation.grid().width(); ++x)
                                {
                                    if (simulation.grid().solid(x, y))
                                    {
                                        ++solidCount;
                                    }
                                }
                            }

                            if (*paintingTool == physics_sim::SceneTool::PaintWall && solidCount > paintingSolidCount)
                            {
                                physics_sim::tutorial_mark_wall_painted(tutorialProgress);
                            }
                            else if (*paintingTool == physics_sim::SceneTool::EraseWall && solidCount < paintingSolidCount)
                            {
                                physics_sim::tutorial_mark_wall_erased(tutorialProgress);
                            }
                        }
                        painting = false;
                        paintingTool.reset();
                        play_audio(physics_sim::AudioCue::UiConfirm);
                        static_cast<void>(save_autosave_scene());
                    }
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = false;
                }
                complete_tutorial_if_needed();
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
                    if (tutorialActive && (event.motion.xrel != 0 || event.motion.yrel != 0))
                    {
                        physics_sim::tutorial_mark_camera_panned(tutorialProgress);
                    }
                }

                if (painting)
                {
                    controller.drag_stroke(viewport.window_to_world(mouseScreen));
                }

                (void)previousScreen;
                complete_tutorial_if_needed();
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                if (sessionShellState.screen != physics_sim::SessionShellScreen::Playing)
                {
                    if (sessionShellState.screen == physics_sim::SessionShellScreen::SceneBrowser && event.wheel.y != 0)
                    {
                        const int direction = event.wheel.y > 0 ? -1 : 1;
                        galleryCategoryFilter = (galleryCategoryFilter + direction + 4) % 4;
                        rebuild_gallery_filter();
                        sessionShellState.selection = 0;
                        const std::array<std::string_view, 4> filterNames{"ALL", "LEARN", "SANDBOX", "CHALLENGES"};
                        set_status_message("GALLERY FILTER " + std::string{filterNames[static_cast<std::size_t>(galleryCategoryFilter)]}, StatusMessageKind::Info);
                        play_audio(physics_sim::AudioCue::UiSelect);
                    }
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
                    if (tutorialActive)
                    {
                        physics_sim::tutorial_mark_camera_zoomed(tutorialProgress);
                    }
                }
                complete_tutorial_if_needed();
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
                    static_cast<void>(persist_settings_or_report());
                }
                else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                    stop_pointer_water();
                }
                complete_tutorial_if_needed();
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

        const auto palette = physics_sim::ui_palette(userSettings.high_contrast);

        SDL_SetRenderDrawColor(renderer, palette.screen_background.r, palette.screen_background.g, palette.screen_background.b, palette.screen_background.a);
        SDL_RenderClear(renderer);

        draw_grid(renderer, viewport, simulation, palette);
        if (visualMode == VisualMode::Surface)
        {
            const float interpolationAlpha = captureByTickCount
                ? 1.0f
                : static_cast<float>(stepDriver.accumulator().count() / stepDriver.fixed_step().count());
            draw_fluid_surface(renderer, viewport, simulation, palette, interpolationAlpha, userSettings.reduced_motion);
        }
        else if (visualMode != VisualMode::Particles)
        {
            draw_fluid_density(renderer, viewport, simulation, palette);
        }
        draw_walls(renderer, viewport, simulation, palette);
        if (visualMode != VisualMode::Density && visualMode != VisualMode::Surface)
        {
            draw_particles(renderer, viewport, simulation, palette);
        }
        draw_drains(renderer, viewport, simulation, controller, palette);
        draw_pumps(renderer, viewport, simulation, controller, palette);
        draw_gates(renderer, viewport, simulation, controller, palette);
        draw_valves(renderer, viewport, simulation, controller, palette);
        draw_sensors(renderer, viewport, simulation, controller, palette);
        draw_emitters(renderer, viewport, simulation, controller, palette);
        const bool sessionShellVisible = sessionShellState.screen != physics_sim::SessionShellScreen::Playing;
        if (!sessionShellVisible)
        {
            if (!cleanCaptureFrame || options.advancedTools)
            {
                draw_tool_palette(renderer, controller, advancedToolsVisible, palette);
            }
            draw_tool_preview(renderer, viewport, simulation, controller, viewport.window_to_world(mouseScreen), palette);
            draw_crosshair(renderer, static_cast<int>(mouseScreen.x), static_cast<int>(mouseScreen.y), palette);
            draw_challenge_overlay(renderer, windowWidth, sceneMetadata.challenge, challengeEvaluator.progress(), palette);
        }
        const double averageFps = realElapsed.count() > 0.0 ? static_cast<double>(frameCount) / realElapsed.count() : 0.0;
        if (options.debugOverlay && !cleanCaptureFrame && !sessionShellVisible)
        {
            physics_sim::DebugOverlayMetrics overlayMetrics;
            overlayMetrics.fps = averageFps;
            overlayMetrics.driver = &stepDriver;
            overlayMetrics.state = &simulationState;
            overlayMetrics.simulation = &simulation;
            overlayMetrics.controller = &controller;
            overlayMetrics.visual_mode = visual_mode_name(visualMode);
            overlayMetrics.status_message = current_status_message();
            if (overlayMetrics.status_message != nullptr)
            {
                overlayMetrics.status_message_rgba = status_message_rgba(palette, statusMessageKind);
                overlayMetrics.status_message_alpha = current_status_message_alpha();
            }
            draw_debug_overlay(renderer, overlayMetrics, viewport.scale());
        }

        if (tutorialActive && !sessionShellVisible)
        {
            draw_tutorial_overlay(renderer, windowWidth, windowHeight, tutorialProgress, userSettings.input_bindings, palette);
        }

        if (showHelp && !sessionShellVisible && !tutorialActive)
        {
            draw_help_overlay(renderer, windowWidth, windowHeight, userSettings.input_bindings, palette);
        }

        if (sessionShellVisible)
        {
            draw_session_shell(renderer, windowWidth, windowHeight, sessionShellState, galleryEntries, saveBrowserEntries, userSettings, palette);
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
                &sessionShellState,
                tutorialActive ? &tutorialProgress : nullptr).c_str());

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
    return applicationExitCode;
}
