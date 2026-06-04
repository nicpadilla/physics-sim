#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>

namespace physics_sim
{
class FixedStepDriver
{
public:
    using duration = std::chrono::duration<double>;

    struct AdvanceResult
    {
        std::size_t steps = 0;
        bool clamped = false;
    };

    explicit FixedStepDriver(duration fixed_step = duration{1.0 / 120.0}, std::size_t max_steps_per_frame = 8)
        : fixed_step_(fixed_step.count() > 0.0 ? fixed_step : duration{1.0 / 120.0}),
          max_steps_per_frame_(std::max<std::size_t>(1, max_steps_per_frame)),
          max_accumulated_(fixed_step_ * static_cast<double>(std::max<std::size_t>(1, max_steps_per_frame)))
    {
    }

    void set_paused(bool paused) noexcept
    {
        paused_ = paused;
        if (!paused_)
        {
            step_requested_ = false;
        }
    }

    void toggle_pause() noexcept
    {
        set_paused(!paused_);
    }

    void request_step() noexcept
    {
        paused_ = true;
        step_requested_ = true;
    }

    bool paused() const noexcept
    {
        return paused_;
    }

    bool step_requested() const noexcept
    {
        return step_requested_;
    }

    duration fixed_step() const noexcept
    {
        return fixed_step_;
    }

    duration accumulator() const noexcept
    {
        return accumulator_;
    }

    std::size_t max_steps_per_frame() const noexcept
    {
        return max_steps_per_frame_;
    }

    void reset() noexcept
    {
        accumulator_ = duration::zero();
        step_requested_ = false;
    }

    template <typename StepFn>
    AdvanceResult advance(duration frame_dt, StepFn&& step_fn)
    {
        AdvanceResult result;

        if (paused_)
        {
            if (step_requested_)
            {
                step_requested_ = false;
                step_fn(fixed_step_);
                result.steps = 1;
            }

            return result;
        }

        if (frame_dt < duration::zero())
        {
            frame_dt = duration::zero();
        }

        accumulator_ += frame_dt;

        if (accumulator_ > max_accumulated_)
        {
            accumulator_ = max_accumulated_;
            result.clamped = true;
        }

        while (accumulator_ >= fixed_step_ && result.steps < max_steps_per_frame_)
        {
            accumulator_ -= fixed_step_;
            step_fn(fixed_step_);
            ++result.steps;
        }

        if (accumulator_ >= fixed_step_)
        {
            accumulator_ = duration::zero();
            result.clamped = true;
        }

        return result;
    }

private:
    duration fixed_step_;
    duration accumulator_{};
    duration max_accumulated_;
    std::size_t max_steps_per_frame_;
    bool paused_ = false;
    bool step_requested_ = false;
};
} // namespace physics_sim

