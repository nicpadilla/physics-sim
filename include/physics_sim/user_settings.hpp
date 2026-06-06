#pragma once

#include <physics_sim/input_bindings.hpp>
#include <physics_sim/solver_profile.hpp>
#include <physics_sim/visual_mode.hpp>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace physics_sim
{
struct WindowSize
{
    int width = 1280;
    int height = 720;
};

struct UserSettings
{
    WindowSize window_size{};
    bool help_overlay_visible = false;
    VisualMode visual_mode = VisualMode::Surface;
    FluidSolverProfile solver_profile = FluidSolverProfile::Balanced;
    bool reduced_motion = false;
    bool fullscreen = false;
    bool high_contrast = false;
    bool audio_muted = false;
    int audio_master_volume = 100;
    int audio_effects_volume = 100;
    int audio_music_volume = 25;
    InputBindings input_bindings{};
};

inline constexpr int UserSettingsVersion = 3;

[[nodiscard]] inline std::optional<VisualMode> parse_visual_mode_token(std::string_view value)
{
    if (value == "surface")
    {
        return VisualMode::Surface;
    }

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

[[nodiscard]] inline std::optional<bool> parse_bool_token(std::string_view value)
{
    if (value == "1" || value == "true" || value == "on")
    {
        return true;
    }

    if (value == "0" || value == "false" || value == "off")
    {
        return false;
    }

    return std::nullopt;
}

[[nodiscard]] inline std::optional<int> parse_int_token(std::string_view value)
{
    int parsed = 0;
    const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (ec != std::errc{} || ptr != value.data() + value.size())
    {
        return std::nullopt;
    }

    return parsed;
}

[[nodiscard]] inline std::optional<int> parse_volume_token(std::string_view value)
{
    const auto parsed = parse_int_token(value);
    if (!parsed.has_value() || *parsed < 0 || *parsed > 100)
    {
        return std::nullopt;
    }

    return parsed;
}

[[nodiscard]] inline std::optional<UserSettings> parse_user_settings_text(std::string_view text)
{
    std::string text_copy{text};
    std::istringstream stream{text_copy};
    UserSettings settings;

    bool saw_header = false;
    bool saw_window = false;
    bool saw_help_overlay = false;
    bool saw_visual_mode = false;
    bool saw_solver_profile = false;
    bool saw_fullscreen = false;
    bool saw_high_contrast = false;
    bool saw_audio_muted = false;
    bool saw_audio_master_volume = false;
    bool saw_audio_effects_volume = false;
    bool saw_audio_music_volume = false;
    int file_version = 0;

    auto trim = [](std::string& line)
    {
        const auto is_not_space = [](unsigned char ch) { return !std::isspace(ch); };
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), is_not_space));
        line.erase(std::find_if(line.rbegin(), line.rend(), is_not_space).base(), line.end());
    };

    std::string line;
    while (std::getline(stream, line))
    {
        trim(line);
        if (line.empty() || line.front() == '#')
        {
            continue;
        }

        std::istringstream line_stream(line);
        std::string keyword;
        line_stream >> keyword;
        if (!line_stream)
        {
            return std::nullopt;
        }

        if (keyword == "physics-sim-settings")
        {
            int version = 0;
            if (!(line_stream >> version) || (version < 1 || version > UserSettingsVersion))
            {
                return std::nullopt;
            }

            file_version = version;
            saw_header = true;
            continue;
        }

        if (keyword == "window")
        {
            if (!(line_stream >> settings.window_size.width >> settings.window_size.height))
            {
                return std::nullopt;
            }

            if (settings.window_size.width <= 0 || settings.window_size.height <= 0)
            {
                return std::nullopt;
            }

            saw_window = true;
            continue;
        }

        if (keyword == "help_overlay")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_bool_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.help_overlay_visible = *parsed;
            saw_help_overlay = true;
            continue;
        }

        if (keyword == "visual_mode")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_visual_mode_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.visual_mode = *parsed;
            saw_visual_mode = true;
            continue;
        }

        if (keyword == "solver_profile")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_solver_profile_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.solver_profile = *parsed;
            saw_solver_profile = true;
            continue;
        }

        if (keyword == "reduced_motion")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_bool_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.reduced_motion = *parsed;
            continue;
        }

        if (keyword == "fullscreen")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_bool_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.fullscreen = *parsed;
            saw_fullscreen = true;
            continue;
        }

        if (keyword == "high_contrast")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_bool_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.high_contrast = *parsed;
            saw_high_contrast = true;
            continue;
        }

        if (keyword == "audio_muted")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_bool_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.audio_muted = *parsed;
            saw_audio_muted = true;
            continue;
        }

        if (keyword == "audio_master")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_volume_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.audio_master_volume = *parsed;
            saw_audio_master_volume = true;
            continue;
        }

        if (keyword == "audio_effects")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_volume_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.audio_effects_volume = *parsed;
            saw_audio_effects_volume = true;
            continue;
        }

        if (keyword == "audio_music")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto parsed = parse_volume_token(token);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }

            settings.audio_music_volume = *parsed;
            saw_audio_music_volume = true;
            continue;
        }

        if (keyword == "bind")
        {
            std::string binding_name;
            std::string key_token;
            if (!(line_stream >> binding_name >> key_token))
            {
                return std::nullopt;
            }

            const auto parsed_keycode = parse_keycode_token(key_token);
            if (!parsed_keycode.has_value())
            {
                return std::nullopt;
            }

            if (!set_binding(settings.input_bindings, binding_name, *parsed_keycode))
            {
                return std::nullopt;
            }

            continue;
        }

        return std::nullopt;
    }

    if (!saw_header || !saw_window || !saw_help_overlay || !saw_visual_mode)
    {
        return std::nullopt;
    }

    if (file_version >= 3 && !saw_solver_profile)
    {
        return std::nullopt;
    }

    if (!validate_input_bindings(settings.input_bindings))
    {
        return std::nullopt;
    }

    return settings;
}

[[nodiscard]] inline std::optional<UserSettings> load_user_settings(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse_user_settings_text(buffer.str());
}

[[nodiscard]] inline UserSettings load_user_settings_or_default(const std::filesystem::path& path)
{
    const auto loaded = load_user_settings(path);
    return loaded.has_value() ? *loaded : UserSettings{};
}

[[nodiscard]] inline bool save_user_settings(const std::filesystem::path& path, const UserSettings& settings)
{
    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }

    file << "physics-sim-settings " << UserSettingsVersion << "\n";
    file << "window " << settings.window_size.width << ' ' << settings.window_size.height << "\n";
    file << "help_overlay " << (settings.help_overlay_visible ? 1 : 0) << "\n";
    file << "visual_mode " << visual_mode_name(settings.visual_mode) << "\n";
    file << "solver_profile " << solver_profile_name(settings.solver_profile) << "\n";
    file << "reduced_motion " << (settings.reduced_motion ? 1 : 0) << "\n";
    file << "fullscreen " << (settings.fullscreen ? 1 : 0) << "\n";
    file << "high_contrast " << (settings.high_contrast ? 1 : 0) << "\n";
    file << "audio_muted " << (settings.audio_muted ? 1 : 0) << "\n";
    file << "audio_master " << settings.audio_master_volume << "\n";
    file << "audio_effects " << settings.audio_effects_volume << "\n";
    file << "audio_music " << settings.audio_music_volume << "\n";
    file << "bind pause_resume " << keycode_token(settings.input_bindings.pause_resume) << "\n";
    file << "bind step_once " << keycode_token(settings.input_bindings.step_once) << "\n";
    file << "bind reset " << keycode_token(settings.input_bindings.reset) << "\n";
    file << "bind reset_fluid " << keycode_token(settings.input_bindings.reset_fluid) << "\n";
    file << "bind toggle_help " << keycode_token(settings.input_bindings.toggle_help) << "\n";
    file << "bind cycle_visual_mode " << keycode_token(settings.input_bindings.cycle_visual_mode) << "\n";
    file << "bind save_scene " << keycode_token(settings.input_bindings.save_scene) << "\n";
    file << "bind load_scene " << keycode_token(settings.input_bindings.load_scene) << "\n";
    file << "bind delete_selection " << keycode_token(settings.input_bindings.delete_selection) << "\n";
    file << "bind undo " << keycode_token(settings.input_bindings.undo) << "\n";
    file << "bind redo " << keycode_token(settings.input_bindings.redo) << "\n";
    file << "bind tool_prev " << keycode_token(settings.input_bindings.tool_prev) << "\n";
    file << "bind tool_next " << keycode_token(settings.input_bindings.tool_next) << "\n";
    file << "bind rotate_counterclockwise " << keycode_token(settings.input_bindings.rotate_counterclockwise) << "\n";
    file << "bind rotate_clockwise " << keycode_token(settings.input_bindings.rotate_clockwise) << "\n";
    file << "bind speed_down " << keycode_token(settings.input_bindings.speed_down) << "\n";
    file << "bind speed_up " << keycode_token(settings.input_bindings.speed_up) << "\n";
    file << "bind emission_down " << keycode_token(settings.input_bindings.emission_down) << "\n";
    file << "bind emission_up " << keycode_token(settings.input_bindings.emission_up) << "\n";
    file << "bind toggle_fullscreen " << keycode_token(settings.input_bindings.toggle_fullscreen) << "\n";
    for (std::size_t index = 0; index < settings.input_bindings.tool_hotkeys.size(); ++index)
    {
        file << "bind tool_hotkey_" << (index + 1) << ' ' << keycode_token(settings.input_bindings.tool_hotkeys[index]) << "\n";
    }

    file.flush();
    return static_cast<bool>(file);
}
} // namespace physics_sim
