#define SDL_MAIN_HANDLED

#include <physics_sim/audio_feedback.hpp>

#include <cmath>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

int main(int argc, char* argv[])
{
    bool enforce_cpu_budget = false;
    std::string raw_output_path;
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument{argv[index]};
        if (argument == "--enforce-cpu-budget")
        {
            enforce_cpu_budget = true;
        }
        else if (argument == "--dump-raw-f32" && index + 1 < argc)
        {
            raw_output_path = argv[++index];
        }
    }
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
    player.handle_device_removed(1);
    REQUIRE(!player.recover(), "force-disabled audio must not reopen unexpectedly");

    physics_sim::AudioLoopSignals all_signals{1.0f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f};
    physics_sim::ContinuousAudioMixer mixer;
    std::vector<float> waveform(48000, 0.0f);
    const auto diagnostics = mixer.render(all_signals, waveform, 48000.0, 1.0f);
    REQUIRE(diagnostics.active_voices == 7, "continuous mixer voice accounting was incorrect");
    REQUIRE(diagnostics.active_voices <= 8, "continuous mixer exceeded the voice cap");
    REQUIRE(diagnostics.peak <= 0.891251f, "continuous mixer exceeded -1 dBFS");
    const auto first_audible = std::find_if(waveform.begin(), waveform.end(), [](float sample) { return std::fabs(sample) > 0.001f; });
    REQUIRE(first_audible != waveform.end() && std::distance(waveform.begin(), first_audible) < 4800, "audio layer response exceeded 100 ms");

    std::vector<float> release(9600, 0.0f);
    const auto release_diagnostics = mixer.render({}, release, 48000.0, 1.0f);
    REQUIRE(release_diagnostics.peak <= 0.891251f, "release envelope clipped");
    REQUIRE(std::fabs(release.back()) < std::fabs(waveform.back()) + 0.01f, "release envelope did not decay");
    REQUIRE(std::fabs(release.front() - waveform.back()) < 0.2f, "release transition introduced a click");

    physics_sim::ContinuousAudioMixer silent_mixer;
    std::vector<float> silence(960, 1.0f);
    static_cast<void>(silent_mixer.render(all_signals, silence, 48000.0, 0.0f));
    REQUIRE(std::all_of(silence.begin(), silence.end(), [](float sample) { return sample == 0.0f; }), "zero volume must produce exact silence");

    physics_sim::ContinuousAudioMixer deterministic_a;
    physics_sim::ContinuousAudioMixer deterministic_b;
    std::vector<float> a(4096, 0.0f);
    std::vector<float> b(4096, 0.0f);
    static_cast<void>(deterministic_a.render(all_signals, a, 48000.0, 0.75f));
    static_cast<void>(deterministic_b.render(all_signals, b, 48000.0, 0.75f));
    REQUIRE(a == b, "procedural audio rendering was not deterministic");

    physics_sim::ContinuousAudioMixer benchmark_mixer;
    std::vector<float> benchmark_block(960, 0.0f);
    const auto benchmark_start = std::chrono::steady_clock::now();
    for (int block = 0; block < 500; ++block)
    {
        static_cast<void>(benchmark_mixer.render(all_signals, benchmark_block, 48000.0, 1.0f));
    }
    const std::chrono::duration<double> benchmark_elapsed = std::chrono::steady_clock::now() - benchmark_start;
    const double cpu_percent = benchmark_elapsed.count() / 10.0 * 100.0;
    if (enforce_cpu_budget)
    {
        REQUIRE(cpu_percent < 1.0, "continuous audio processing exceeded 1% CPU");
    }

    if (!raw_output_path.empty())
    {
        std::ofstream output(raw_output_path, std::ios::binary | std::ios::trunc);
        REQUIRE(static_cast<bool>(output), "audio evidence output could not be opened");
        output.write(reinterpret_cast<const char*>(waveform.data()), static_cast<std::streamsize>(waveform.size() * sizeof(float)));
        REQUIRE(static_cast<bool>(output), "audio evidence output failed");
    }

    std::cout << "audio feedback tests passed peak=" << diagnostics.peak
              << " voices=" << diagnostics.active_voices
              << " cpu_percent=" << cpu_percent << std::endl;
    return 0;
}
