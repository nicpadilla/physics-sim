#define SDL_MAIN_HANDLED

#include <physics_sim/input_bindings.hpp>
#include <physics_sim/user_settings.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>

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
    namespace fs = std::filesystem;

    {
        physics_sim::UserSettings settings;
        settings.window_size = {1024, 768};
        settings.help_overlay_visible = true;
        settings.visual_mode = physics_sim::VisualMode::Surface;
        settings.solver_profile = physics_sim::FluidSolverProfile::Quality;
        settings.reduced_motion = true;
        settings.fullscreen = true;
        settings.high_contrast = true;
        settings.audio_muted = true;
        settings.audio_master_volume = 72;
        settings.audio_effects_volume = 88;
        settings.audio_music_volume = 24;
        settings.input_bindings.pause_resume = SDLK_p;
        settings.input_bindings.reset = SDLK_F12;
        settings.input_bindings.tool_hotkeys[0] = SDLK_0;

        const fs::path path = fs::temp_directory_path() / "physics-sim-user-settings.psettings";
        REQUIRE(physics_sim::save_user_settings(path, settings), "save_user_settings failed");

        const auto loaded = physics_sim::load_user_settings(path);
        REQUIRE(loaded.has_value(), "load_user_settings failed");
        REQUIRE(loaded->window_size.width == 1024, "loaded window width incorrect");
        REQUIRE(loaded->window_size.height == 768, "loaded window height incorrect");
        REQUIRE(loaded->help_overlay_visible, "loaded help overlay state incorrect");
        REQUIRE(loaded->visual_mode == physics_sim::VisualMode::Surface, "loaded visual mode incorrect");
        REQUIRE(loaded->solver_profile == physics_sim::FluidSolverProfile::Quality, "loaded solver profile incorrect");
        REQUIRE(loaded->reduced_motion, "loaded reduced-motion state incorrect");
        REQUIRE(loaded->fullscreen, "loaded fullscreen state incorrect");
        REQUIRE(loaded->high_contrast, "loaded high-contrast state incorrect");
        REQUIRE(loaded->audio_muted, "loaded audio muted state incorrect");
        REQUIRE(loaded->audio_master_volume == 72, "loaded master volume incorrect");
        REQUIRE(loaded->audio_effects_volume == 88, "loaded effects volume incorrect");
        REQUIRE(loaded->audio_music_volume == 24, "loaded music volume incorrect");
        REQUIRE(loaded->input_bindings.pause_resume == SDLK_p, "loaded pause binding incorrect");
        REQUIRE(loaded->input_bindings.reset == SDLK_F12, "loaded reset binding incorrect");
        REQUIRE(loaded->input_bindings.tool_hotkeys[0] == SDLK_0, "loaded tool hotkey incorrect");

        fs::remove(path);
    }

    {
        const auto parsed = physics_sim::parse_user_settings_text(
            "physics-sim-recovery-settings 1\n"
            "window 1280 720\n"
            "help_overlay 0\n"
            "visual_mode surface\n"
            "solver_profile balanced\n"
            "reduced_motion 1\n"
            "fullscreen 1\n"
            "high_contrast 1\n"
            "audio_muted 1\n"
            "audio_master 72\n"
            "audio_effects 88\n"
            "audio_music 24\n"
            "bind pause_resume p\n"
            "bind reset f12\n"
            "bind tool_hotkey_1 0\n");
        REQUIRE(parsed.has_value(), "parse_user_settings_text rejected valid settings");
        REQUIRE(parsed->visual_mode == physics_sim::VisualMode::Surface, "parse_user_settings_text lost surface visual mode");
        REQUIRE(parsed->solver_profile == physics_sim::FluidSolverProfile::Balanced, "parse_user_settings_text lost solver profile");
        REQUIRE(parsed->reduced_motion, "parse_user_settings_text lost reduced-motion state");
        REQUIRE(parsed->fullscreen, "parse_user_settings_text lost fullscreen state");
        REQUIRE(parsed->high_contrast, "parse_user_settings_text lost high-contrast state");
        REQUIRE(parsed->audio_muted, "parse_user_settings_text lost audio mute state");
        REQUIRE(parsed->audio_master_volume == 72, "parse_user_settings_text lost master volume");
        REQUIRE(parsed->audio_effects_volume == 88, "parse_user_settings_text lost effects volume");
        REQUIRE(parsed->audio_music_volume == 24, "parse_user_settings_text lost music volume");
        REQUIRE(parsed->input_bindings.pause_resume == SDLK_p, "parse_user_settings_text lost pause binding");
        REQUIRE(parsed->input_bindings.reset == SDLK_F12, "parse_user_settings_text lost reset binding");
        REQUIRE(parsed->input_bindings.tool_hotkeys[0] == SDLK_0, "parse_user_settings_text lost tool hotkey");
    }

    {
        const auto parsed = physics_sim::parse_user_settings_text(
            "physics-sim-recovery-settings 2\n"
            "window 1280 720\n"
            "help_overlay 0\n"
            "visual_mode mixed\n");
        REQUIRE(!parsed.has_value(), "parse_user_settings_text accepted unsupported version");
    }

    {
        const auto parsed = physics_sim::parse_user_settings_text(
            "physics-sim-recovery-settings 1\n"
            "window 0 720\n"
            "help_overlay 0\n"
            "visual_mode mixed\n");
        REQUIRE(!parsed.has_value(), "parse_user_settings_text accepted invalid window dimensions");
    }

    {
        const auto parsed = physics_sim::parse_user_settings_text(
            "physics-sim-recovery-settings 1\n"
            "window 1280 720\n"
            "help_overlay maybe\n"
            "visual_mode mixed\n");
        REQUIRE(!parsed.has_value(), "parse_user_settings_text accepted invalid help overlay token");
    }

    {
        const auto parsed = physics_sim::parse_user_settings_text(
            "physics-sim-recovery-settings 1\n"
            "window 1280 720\n"
            "help_overlay 0\n"
            "visual_mode mystery\n");
        REQUIRE(!parsed.has_value(), "parse_user_settings_text accepted invalid visual mode token");
    }

    {
        const auto legacy = physics_sim::parse_user_settings_text(
            "physics-sim-settings 3\n"
            "window 1280 720\n"
            "help_overlay 0\n"
            "visual_mode surface\n");
        REQUIRE(!legacy.has_value(), "parse_user_settings_text accepted pre-recovery settings");
    }

    {
        const fs::path path = fs::temp_directory_path() / "physics-sim-missing-user-settings.psettings";
        fs::remove(path);
        const auto loaded = physics_sim::load_user_settings(path);
        REQUIRE(!loaded.has_value(), "load_user_settings unexpectedly succeeded on a missing file");

        const auto defaults = physics_sim::load_user_settings_or_default(path);
        REQUIRE(defaults.window_size.width == 1280, "default window width incorrect");
        REQUIRE(defaults.window_size.height == 720, "default window height incorrect");
        REQUIRE(!defaults.help_overlay_visible, "default help overlay state incorrect");
        REQUIRE(defaults.visual_mode == physics_sim::VisualMode::Surface, "default visual mode incorrect");
        REQUIRE(defaults.solver_profile == physics_sim::FluidSolverProfile::Balanced, "default solver profile incorrect");
        REQUIRE(!defaults.reduced_motion, "default reduced-motion state incorrect");
    }

    return 0;
}
