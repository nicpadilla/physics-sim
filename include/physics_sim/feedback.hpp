#pragma once

#include <algorithm>
#include <chrono>

namespace physics_sim
{
[[nodiscard]] inline constexpr float feedback_message_alpha(
    std::chrono::duration<double> elapsed,
    std::chrono::duration<double> lifetime,
    bool reduced_motion) noexcept
{
    if (reduced_motion || lifetime.count() <= 0.0)
    {
        return 1.0f;
    }

    const double progress = std::clamp(elapsed.count() / lifetime.count(), 0.0, 1.0);
    return static_cast<float>(0.35 + 0.65 * (1.0 - progress));
}
} // namespace physics_sim
