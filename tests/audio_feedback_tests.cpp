#define SDL_MAIN_HANDLED

#include <physics_sim/audio_feedback.hpp>

#include <cmath>
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
    REQUIRE(std::string_view{physics_sim::audio_cue_name(physics_sim::AudioCue::Save)} == "save", "audio cue name was incorrect");

    physics_sim::AudioSettings settings;
    settings.master_volume = 100;
    settings.effects_volume = 100;
    settings.music_volume = 100;

    const auto save_samples = physics_sim::render_audio_cue(physics_sim::AudioCue::Save, settings);
    const auto load_samples = physics_sim::render_audio_cue(physics_sim::AudioCue::Load, settings);
    REQUIRE(!save_samples.empty(), "save cue did not render any samples");
    REQUIRE(!load_samples.empty(), "load cue did not render any samples");
    REQUIRE(save_samples.size() == load_samples.size(), "cue durations should be stable for matching cue families");
    REQUIRE(std::fabs(save_samples.front()) < 0.1f, "audio cue should start near silence");
    REQUIRE(std::fabs(save_samples[save_samples.size() / 2]) > 0.001f, "audio cue should produce audible samples");

    settings.muted = true;
    REQUIRE(physics_sim::render_audio_cue(physics_sim::AudioCue::UiConfirm, settings).empty(), "muted audio should render no samples");

    physics_sim::AudioPlayer player;
    REQUIRE(!player.initialize(settings, true), "force-disabled audio should not initialize playback");
    REQUIRE(!player.available(), "disabled audio player should not report available");

    std::cout << "audio feedback tests passed" << std::endl;
    return 0;
}
