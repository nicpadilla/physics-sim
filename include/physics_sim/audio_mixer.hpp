#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace physics_sim
{
struct AudioLoopSignals
{
    float pour = 0.0f;
    float flow = 0.0f;
    float impact = 0.0f;
    float pump = 0.0f;
    float drain = 0.0f;
    float fixture = 0.0f;
    float objective = 0.0f;
};

struct AudioMixerDiagnostics
{
    float peak = 0.0f;
    std::size_t active_voices = 0;
};

class ContinuousAudioMixer
{
public:
    void reset() noexcept;
    [[nodiscard]] AudioMixerDiagnostics render(
        const AudioLoopSignals& signals,
        std::span<float> output,
        double sample_rate,
        float volume_scale) noexcept;

private:
    std::array<double, 7> phases_{};
    std::array<float, 7> envelopes_{};
    std::uint32_t noise_state_ = 0x8f3a21c5u;
};
} // namespace physics_sim
