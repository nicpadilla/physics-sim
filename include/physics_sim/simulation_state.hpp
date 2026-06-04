#pragma once

#include <chrono>
#include <cstdint>

namespace physics_sim
{
struct SimulationState
{
    using duration = std::chrono::duration<double>;

    std::uint64_t tick_count = 0;
    duration simulated_time{};

    void reset() noexcept
    {
        tick_count = 0;
        simulated_time = duration::zero();
    }

    void advance(duration step) noexcept
    {
        ++tick_count;
        simulated_time += step;
    }
};
} // namespace physics_sim

