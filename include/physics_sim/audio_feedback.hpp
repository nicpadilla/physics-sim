#pragma once

#include <SDL.h>

#include <physics_sim/audio_mixer.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace physics_sim
{
enum class AudioCue
{
    UiSelect,
    UiConfirm,
    UiCancel,
    InvalidAction,
    Save,
    Load,
    Reset,
    DeviceTrigger,
    ObjectiveComplete,
};

struct AudioSettings
{
    bool muted = false;
    int master_volume = 100;
    int effects_volume = 100;
    int music_volume = 25;
};

[[nodiscard]] inline constexpr const char* audio_cue_name(AudioCue cue) noexcept
{
    switch (cue)
    {
    case AudioCue::UiSelect:
        return "ui-select";
    case AudioCue::UiConfirm:
        return "ui-confirm";
    case AudioCue::UiCancel:
        return "ui-cancel";
    case AudioCue::InvalidAction:
        return "invalid";
    case AudioCue::Save:
        return "save";
    case AudioCue::Load:
        return "load";
    case AudioCue::Reset:
        return "reset";
    case AudioCue::DeviceTrigger:
        return "trigger";
    case AudioCue::ObjectiveComplete:
        return "objective-complete";
    }

    return "unknown";
}

[[nodiscard]] inline constexpr float audio_cue_duration_seconds(AudioCue cue) noexcept
{
    switch (cue)
    {
    case AudioCue::UiSelect:
        return 0.04f;
    case AudioCue::UiConfirm:
        return 0.06f;
    case AudioCue::UiCancel:
        return 0.06f;
    case AudioCue::InvalidAction:
        return 0.08f;
    case AudioCue::Save:
        return 0.09f;
    case AudioCue::Load:
        return 0.09f;
    case AudioCue::Reset:
        return 0.07f;
    case AudioCue::DeviceTrigger:
        return 0.07f;
    case AudioCue::ObjectiveComplete:
        return 0.16f;
    }

    return 0.05f;
}

[[nodiscard]] inline constexpr float audio_cue_base_frequency(AudioCue cue) noexcept
{
    switch (cue)
    {
    case AudioCue::UiSelect:
        return 880.0f;
    case AudioCue::UiConfirm:
        return 660.0f;
    case AudioCue::UiCancel:
        return 330.0f;
    case AudioCue::InvalidAction:
        return 140.0f;
    case AudioCue::Save:
        return 520.0f;
    case AudioCue::Load:
        return 780.0f;
    case AudioCue::Reset:
        return 220.0f;
    case AudioCue::DeviceTrigger:
        return 1200.0f;
    case AudioCue::ObjectiveComplete:
        return 440.0f;
    }

    return 440.0f;
}

[[nodiscard]] inline std::vector<float> render_audio_cue(
    AudioCue cue,
    const AudioSettings& settings,
    double sample_rate = 48000.0)
{
    if (settings.muted || settings.master_volume <= 0 || settings.effects_volume <= 0 || sample_rate <= 0.0)
    {
        return {};
    }

    const double volume_scale = std::clamp(static_cast<double>(settings.master_volume) / 100.0 * static_cast<double>(settings.effects_volume) / 100.0, 0.0, 1.0);
    const double music_scale = std::clamp(static_cast<double>(settings.master_volume) / 100.0 * static_cast<double>(settings.music_volume) / 100.0, 0.0, 1.0);
    const float duration = audio_cue_duration_seconds(cue);
    const std::size_t sample_count = static_cast<std::size_t>(std::max(1.0, std::ceil(sample_rate * static_cast<double>(duration))));
    std::vector<float> samples(sample_count, 0.0f);

    const auto clamp_sample = [](double value) noexcept -> float
    {
        return static_cast<float>(std::clamp(value, -1.0, 1.0));
    };

    for (std::size_t index = 0; index < sample_count; ++index)
    {
        const double t = static_cast<double>(index) / sample_rate;
        const double decay = std::exp(-t * 24.0);
        double sample = 0.0;

        switch (cue)
        {
        case AudioCue::UiSelect:
            sample = std::sin(2.0 * 3.14159265358979323846 * audio_cue_base_frequency(cue) * t) * decay;
            break;
        case AudioCue::UiConfirm:
            sample = 0.7 * std::sin(2.0 * 3.14159265358979323846 * 660.0 * t)
                + 0.4 * std::sin(2.0 * 3.14159265358979323846 * 990.0 * t);
            sample *= decay;
            break;
        case AudioCue::UiCancel:
            sample = std::sin(2.0 * 3.14159265358979323846 * (330.0 - t * 120.0) * t) * decay;
            break;
        case AudioCue::InvalidAction:
            sample = std::sin(2.0 * 3.14159265358979323846 * 140.0 * t) * (0.8 + 0.2 * std::sin(2.0 * 3.14159265358979323846 * 9.0 * t));
            sample *= decay;
            break;
        case AudioCue::Save:
            sample = std::sin(2.0 * 3.14159265358979323846 * (520.0 + t * 180.0) * t) * decay;
            break;
        case AudioCue::Load:
            sample = std::sin(2.0 * 3.14159265358979323846 * (780.0 - t * 180.0) * t) * decay;
            break;
        case AudioCue::Reset:
            sample = std::sin(2.0 * 3.14159265358979323846 * 220.0 * t) * decay * (1.0 - std::min(1.0, t * 12.0));
            break;
        case AudioCue::DeviceTrigger:
            sample = std::sin(2.0 * 3.14159265358979323846 * 1200.0 * t) * decay;
            break;
        case AudioCue::ObjectiveComplete:
        {
            const double mellow = std::sin(2.0 * 3.14159265358979323846 * 440.0 * t)
                + 0.7 * std::sin(2.0 * 3.14159265358979323846 * 554.37 * t)
                + 0.5 * std::sin(2.0 * 3.14159265358979323846 * 659.25 * t);
            sample = mellow * std::exp(-t * 8.0) * (0.5 + 0.5 * music_scale);
            break;
        }
        }

        const double scale = cue == AudioCue::ObjectiveComplete ? music_scale : volume_scale;
        samples[index] = clamp_sample(sample * scale * 0.85);
    }

    return samples;
}

class AudioPlayer
{
public:
    AudioPlayer() = default;

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    ~AudioPlayer()
    {
        shutdown();
    }

    [[nodiscard]] bool initialize(const AudioSettings& settings, bool force_disabled, std::string* error_out = nullptr)
    {
        shutdown();
        settings_ = settings;
        force_disabled_ = force_disabled;
        continuous_mixer_.reset();

        if (force_disabled)
        {
            disabled_ = true;
            return false;
        }

        SDL_AudioSpec desired{};
        desired.freq = 48000;
        desired.format = AUDIO_F32SYS;
        desired.channels = 1;
        desired.samples = 2048;

        SDL_AudioSpec obtained{};
        device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
        if (device_ == 0)
        {
            disabled_ = true;
            if (error_out != nullptr)
            {
                *error_out = SDL_GetError();
            }
            return false;
        }

        sample_rate_ = obtained.freq > 0 ? static_cast<double>(obtained.freq) : static_cast<double>(desired.freq);
        disabled_ = false;
        SDL_PauseAudioDevice(device_, 0);
        return true;
    }

    void shutdown() noexcept
    {
        if (device_ != 0)
        {
            SDL_CloseAudioDevice(device_);
            device_ = 0;
        }
        disabled_ = true;
    }

    void apply_settings(const AudioSettings& settings) noexcept
    {
        settings_ = settings;
        if (device_ != 0 && (settings_.muted || settings_.master_volume <= 0 || settings_.effects_volume <= 0))
        {
            SDL_ClearQueuedAudio(device_);
            continuous_mixer_.reset();
        }
    }

    [[nodiscard]] bool available() const noexcept
    {
        return device_ != 0 && !disabled_;
    }

    [[nodiscard]] bool play(AudioCue cue) noexcept
    {
        if (!available())
        {
            return false;
        }

        const auto samples = render_audio_cue(cue, settings_, sample_rate_);
        if (samples.empty())
        {
            return false;
        }

        if (SDL_QueueAudio(device_, samples.data(), static_cast<Uint32>(samples.size() * sizeof(float))) != 0)
        {
            return false;
        }

        return true;
    }

    [[nodiscard]] bool update_continuous(const AudioLoopSignals& signals) noexcept
    {
        if (!available()) return false;
        if (settings_.muted || settings_.master_volume <= 0 || settings_.effects_volume <= 0)
        {
            SDL_ClearQueuedAudio(device_);
            continuous_mixer_.reset();
            last_diagnostics_ = {};
            return true;
        }
        constexpr std::size_t block_samples = 960; // 20 ms at 48 kHz
        const Uint32 queued_bytes = SDL_GetQueuedAudioSize(device_);
        const double queued_seconds = static_cast<double>(queued_bytes) / (sample_rate_ * sizeof(float));
        if (queued_seconds >= 0.080) return true;
        std::array<float, block_samples> block{};
        const float volume = std::clamp(
            static_cast<float>(settings_.master_volume) / 100.0f * static_cast<float>(settings_.effects_volume) / 100.0f,
            0.0f,
            1.0f);
        last_diagnostics_ = continuous_mixer_.render(signals, block, sample_rate_, volume);
        if (SDL_QueueAudio(device_, block.data(), static_cast<Uint32>(block.size() * sizeof(float))) != 0)
        {
            disabled_ = true;
            return false;
        }
        return true;
    }

    void handle_device_removed(SDL_AudioDeviceID removed_device) noexcept
    {
        if (device_ != 0 && removed_device == device_) shutdown();
    }

    [[nodiscard]] bool recover(std::string* error_out = nullptr)
    {
        if (available()) return true;
        if (force_disabled_) return false;
        return initialize(settings_, false, error_out);
    }

    [[nodiscard]] AudioMixerDiagnostics diagnostics() const noexcept { return last_diagnostics_; }

private:
    SDL_AudioDeviceID device_ = 0;
    AudioSettings settings_{};
    double sample_rate_ = 48000.0;
    bool disabled_ = true;
    bool force_disabled_ = false;
    ContinuousAudioMixer continuous_mixer_{};
    AudioMixerDiagnostics last_diagnostics_{};
};
} // namespace physics_sim
