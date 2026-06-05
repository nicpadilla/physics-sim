#define SDL_MAIN_HANDLED

#include <physics_sim/settings_menu.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)
} // namespace

int main()
{
    physics_sim::UserSettings settings;
    settings.window_size = {1600, 900};
    settings.help_overlay_visible = true;
    settings.visual_mode = physics_sim::VisualMode::Density;
    settings.reduced_motion = true;
    settings.fullscreen = true;
    settings.high_contrast = true;
    settings.audio_muted = true;
    settings.audio_master_volume = 80;
    settings.audio_effects_volume = 60;
    settings.audio_music_volume = 20;
    settings.input_bindings.pause_resume = SDLK_p;
    settings.input_bindings.tool_prev = SDLK_LEFTBRACKET;
    settings.input_bindings.tool_next = SDLK_RIGHTBRACKET;
    settings.input_bindings.speed_down = SDLK_MINUS;
    settings.input_bindings.speed_up = SDLK_EQUALS;

    const auto entries = physics_sim::build_settings_menu_entries(settings);
    REQUIRE(entries.size() == 34, "settings menu entry count was incorrect");
    REQUIRE(entries.front().label == "Display: Window Size 1600x900", "settings menu window size label was incorrect");
    REQUIRE(entries[1].label == "Display: Fullscreen On", "settings menu fullscreen label was incorrect");
    REQUIRE(entries[6].label == "Audio: Muted On", "settings menu audio mute label was incorrect");
    REQUIRE(entries[7].label == "Audio: Master Volume 80 (-)", "settings menu master volume label was incorrect");
    REQUIRE(entries[8].label == "Audio: Master Volume 80 (+)", "settings menu master volume increase label was incorrect");
    REQUIRE(entries[13].label == "Controls: Pause / Resume -> P", "settings menu remap label did not use the active binding");
    REQUIRE(entries[24].label == "Controls: Previous Tool -> [", "settings menu tool-prev label was incorrect");
    REQUIRE(entries.back().kind == physics_sim::SettingsMenuEntryKind::Back, "settings menu did not end with back");

    const auto presets = physics_sim::settings_window_size_presets();
    REQUIRE(physics_sim::next_window_size({1280, 720}, 1).width == presets[1].width, "next_window_size did not advance to the next preset");
    REQUIRE(physics_sim::next_window_size({1280, 720}, -1).width == presets[2].width, "next_window_size did not wrap backward");

    std::cout << "settings menu tests passed" << std::endl;
    return 0;
}
