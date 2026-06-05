#pragma once

#include <physics_sim/input_bindings.hpp>
#include <physics_sim/user_settings.hpp>
#include <physics_sim/visual_mode.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace physics_sim
{
enum class SettingsMenuEntryKind
{
    CycleWindowSize,
    ToggleFullscreen,
    ToggleHelpOverlay,
    CycleVisualMode,
    ToggleHighContrast,
    ToggleReducedMotion,
    ToggleAudioMute,
    AdjustMasterVolume,
    AdjustEffectsVolume,
    AdjustMusicVolume,
    RemapBinding,
    Back,
};

struct SettingsMenuEntry
{
    SettingsMenuEntryKind kind = SettingsMenuEntryKind::Back;
    std::string label{};
    std::string binding_name{};
    int adjustment = 0;
};

[[nodiscard]] inline constexpr std::array<WindowSize, 3> settings_window_size_presets() noexcept
{
    return {
        WindowSize{1280, 720},
        WindowSize{1600, 900},
        WindowSize{1920, 1080},
    };
}

[[nodiscard]] inline std::string window_size_label(const WindowSize& size)
{
    return std::to_string(size.width) + "x" + std::to_string(size.height);
}

[[nodiscard]] inline std::string bool_label(std::string_view prefix, bool value)
{
    return std::string{prefix} + (value ? "On" : "Off");
}

[[nodiscard]] inline std::string volume_label(std::string_view prefix, int value, int adjustment)
{
    return std::string{prefix} + std::to_string(value) + (adjustment < 0 ? " (-)" : " (+)");
}

[[nodiscard]] inline WindowSize next_window_size(WindowSize current, int direction) noexcept
{
    if (direction == 0)
    {
        return current;
    }

    const auto presets = settings_window_size_presets();
    std::size_t current_index = 0;
    for (std::size_t index = 0; index < presets.size(); ++index)
    {
        if (presets[index].width == current.width && presets[index].height == current.height)
        {
            current_index = index;
            break;
        }
    }

    const int step = direction > 0 ? 1 : -1;
    const int size = static_cast<int>(presets.size());
    const int next_index = (static_cast<int>(current_index) + step + size) % size;
    return presets[static_cast<std::size_t>(next_index)];
}

[[nodiscard]] inline std::vector<SettingsMenuEntry> build_settings_menu_entries(const UserSettings& settings)
{
    std::vector<SettingsMenuEntry> entries;
    entries.reserve(32);

    entries.push_back({
        SettingsMenuEntryKind::CycleWindowSize,
        "Display: Window Size " + window_size_label(settings.window_size),
        {},
        1,
    });
    entries.push_back({
        SettingsMenuEntryKind::ToggleFullscreen,
        bool_label("Display: Fullscreen ", settings.fullscreen),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::ToggleHelpOverlay,
        bool_label("Display: Help Overlay ", settings.help_overlay_visible),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::CycleVisualMode,
        std::string{"Display: Visual Mode "} + visual_mode_name(settings.visual_mode),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::ToggleHighContrast,
        bool_label("Accessibility: High Contrast ", settings.high_contrast),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::ToggleReducedMotion,
        bool_label("Accessibility: Reduced Motion ", settings.reduced_motion),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::ToggleAudioMute,
        bool_label("Audio: Muted ", settings.audio_muted),
        {},
        0,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustMasterVolume,
        volume_label("Audio: Master Volume ", settings.audio_master_volume, -10),
        {},
        -10,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustMasterVolume,
        volume_label("Audio: Master Volume ", settings.audio_master_volume, 10),
        {},
        10,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustEffectsVolume,
        volume_label("Audio: Effects Volume ", settings.audio_effects_volume, -10),
        {},
        -10,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustEffectsVolume,
        volume_label("Audio: Effects Volume ", settings.audio_effects_volume, 10),
        {},
        10,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustMusicVolume,
        volume_label("Audio: Music Volume ", settings.audio_music_volume, -10),
        {},
        -10,
    });
    entries.push_back({
        SettingsMenuEntryKind::AdjustMusicVolume,
        volume_label("Audio: Music Volume ", settings.audio_music_volume, 10),
        {},
        10,
    });

    const auto add_remap_entry = [&](std::string_view binding_name)
    {
        const auto current = binding_value(settings.input_bindings, binding_name);
        entries.push_back({
            SettingsMenuEntryKind::RemapBinding,
            std::string{"Controls: "} + binding_display_name(binding_name) + " -> " + (current.has_value() ? keycode_label(*current) : std::string{"Unbound"}),
            std::string{binding_name},
            0,
        });
    };

    add_remap_entry("pause_resume");
    add_remap_entry("step_once");
    add_remap_entry("reset");
    add_remap_entry("reset_fluid");
    add_remap_entry("toggle_help");
    add_remap_entry("cycle_visual_mode");
    add_remap_entry("save_scene");
    add_remap_entry("load_scene");
    add_remap_entry("delete_selection");
    add_remap_entry("undo");
    add_remap_entry("redo");
    add_remap_entry("tool_prev");
    add_remap_entry("tool_next");
    add_remap_entry("rotate_counterclockwise");
    add_remap_entry("rotate_clockwise");
    add_remap_entry("speed_down");
    add_remap_entry("speed_up");
    add_remap_entry("emission_down");
    add_remap_entry("emission_up");
    add_remap_entry("toggle_fullscreen");

    entries.push_back({
        SettingsMenuEntryKind::Back,
        "Back",
        {},
        0,
    });

    return entries;
}
} // namespace physics_sim
