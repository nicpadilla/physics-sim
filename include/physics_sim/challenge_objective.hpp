#pragma once

#include <physics_sim/scene_document.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>

namespace physics_sim
{
enum class ChallengeStatus { Inactive, Running, Complete, Failed };

struct ChallengeProgress
{
    ChallengeStatus status = ChallengeStatus::Inactive;
    std::uint64_t held_ticks = 0;
    std::optional<std::uint64_t> completion_tick{};
    const char* failure_reason = "";
};

class ChallengeEvaluator
{
public:
    void reset() noexcept { progress_ = {}; }

    [[nodiscard]] const ChallengeProgress& progress() const noexcept { return progress_; }

    const ChallengeProgress& update(const std::optional<SceneChallenge>& challenge, const WaterSimulationMetrics& metrics, std::uint64_t tick) noexcept
    {
        if (!challenge.has_value()) { progress_ = {}; return progress_; }
        if (progress_.status == ChallengeStatus::Complete || progress_.status == ChallengeStatus::Failed) return progress_;
        progress_.status = ChallengeStatus::Running;
        if (challenge->maximum_emitted_mass && metrics.total_emitted_mass > *challenge->maximum_emitted_mass)
        { progress_.status = ChallengeStatus::Failed; progress_.failure_reason = "EMITTED WATER BUDGET EXCEEDED"; return progress_; }
        if (challenge->maximum_outflow_mass && metrics.total_outflow_mass > *challenge->maximum_outflow_mass)
        { progress_.status = ChallengeStatus::Failed; progress_.failure_reason = "OUTFLOW BUDGET EXCEEDED"; return progress_; }
        const bool sensors_satisfied = metrics.objective_sensors >= challenge->required_objective_sensors && metrics.objective_completed;
        progress_.held_ticks = sensors_satisfied ? progress_.held_ticks + 1 : 0;
        if (progress_.held_ticks >= challenge->hold_ticks)
        { progress_.status = ChallengeStatus::Complete; progress_.completion_tick = tick; }
        return progress_;
    }

private:
    ChallengeProgress progress_{};
};
} // namespace physics_sim
