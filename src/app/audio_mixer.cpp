#include <physics_sim/audio_mixer.hpp>

#include <algorithm>
#include <cmath>

namespace physics_sim
{
namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr float peak_limit = 0.89125094f; // -1 dBFS

[[nodiscard]] float clamp_signal(float value) noexcept
{
    return std::clamp(std::isfinite(value) ? value : 0.0f, 0.0f, 1.0f);
}
} // namespace

void ContinuousAudioMixer::reset() noexcept
{
    phases_.fill(0.0);
    envelopes_.fill(0.0f);
    noise_state_ = 0x8f3a21c5u;
}

AudioMixerDiagnostics ContinuousAudioMixer::render(
    const AudioLoopSignals& input,
    std::span<float> output,
    double sample_rate,
    float volume_scale) noexcept
{
    AudioMixerDiagnostics diagnostics;
    if (output.empty() || sample_rate <= 0.0 || volume_scale <= 0.0f)
    {
        std::fill(output.begin(), output.end(), 0.0f);
        return diagnostics;
    }

    const std::array<float, 7> targets{
        clamp_signal(input.pour), clamp_signal(input.flow), clamp_signal(input.impact),
        clamp_signal(input.pump), clamp_signal(input.drain), clamp_signal(input.fixture),
        clamp_signal(input.objective)};
    const std::array<double, 7> frequencies{170.0, 74.0, 310.0, 92.0, 128.0, 245.0, 196.0};
    for (float target : targets) if (target > 0.001f) ++diagnostics.active_voices;

    const float attack = static_cast<float>(1.0 - std::exp(-1.0 / (sample_rate * 0.025)));
    const float release = static_cast<float>(1.0 - std::exp(-1.0 / (sample_rate * 0.140)));
    const float gain = std::clamp(volume_scale, 0.0f, 1.0f);
    for (float& destination : output)
    {
        for (std::size_t layer = 0; layer < envelopes_.size(); ++layer)
        {
            const float coefficient = targets[layer] > envelopes_[layer] ? attack : release;
            envelopes_[layer] += (targets[layer] - envelopes_[layer]) * coefficient;
            phases_[layer] += 2.0 * pi * frequencies[layer] / sample_rate;
            if (phases_[layer] >= 2.0 * pi) phases_[layer] -= 2.0 * pi;
        }
        noise_state_ = noise_state_ * 1664525u + 1013904223u;
        const float noise = (static_cast<float>((noise_state_ >> 8) & 0xffffu) / 32767.5f) - 1.0f;
        float mixed = 0.0f;
        mixed += envelopes_[0] * (0.055f * noise + 0.025f * static_cast<float>(std::sin(phases_[0])));
        mixed += envelopes_[1] * (0.045f * noise + 0.025f * static_cast<float>(std::sin(phases_[1])));
        mixed += envelopes_[2] * (0.050f * noise + 0.018f * static_cast<float>(std::sin(phases_[2])));
        mixed += envelopes_[3] * (0.045f * static_cast<float>(std::sin(phases_[3])) + 0.018f * static_cast<float>(std::sin(phases_[3] * 2.0)));
        mixed += envelopes_[4] * (0.040f * noise + 0.018f * static_cast<float>(std::sin(phases_[4])));
        mixed += envelopes_[5] * 0.022f * static_cast<float>(std::sin(phases_[5]));
        mixed += envelopes_[6] * (0.018f * static_cast<float>(std::sin(phases_[6])) + 0.012f * static_cast<float>(std::sin(phases_[6] * 1.5)));
        destination = peak_limit * std::tanh(mixed * gain / peak_limit);
        diagnostics.peak = std::max(diagnostics.peak, std::fabs(destination));
    }
    return diagnostics;
}
} // namespace physics_sim
