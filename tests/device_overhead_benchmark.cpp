#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace
{
using Clock = std::chrono::steady_clock;

physics_sim::WaterSimulation2D make_simulation(bool devices)
{
    physics_sim::WaterSimulation2D simulation{80, 45, 1.0f};
    simulation.set_solver_settings(
        physics_sim::WaterSimulation2D::solver_settings_for_profile(physics_sim::FluidSolverProfile::Balanced));
    for (std::size_t x = 8; x < 72; ++x) simulation.set_solid_cell(x, 40, true);
    for (std::size_t y = 12; y <= 40; ++y)
    {
        simulation.set_solid_cell(8, y, true);
        simulation.set_solid_cell(71, y, true);
    }
    for (std::size_t y = 26; y < 40; ++y)
    {
        for (std::size_t x = 18; x < 62; ++x)
        {
            simulation.add_particle({{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f}, {}});
        }
    }
    if (devices)
    {
        physics_sim::WaterEmitter emitter;
        emitter.position = {40.0f, 8.0f};
        emitter.enabled = false;
        simulation.add_emitter(emitter);
        for (std::size_t index = 0; index < 8; ++index)
        {
            simulation.add_gate({12 + index * 7, 20, true});
            simulation.add_valve({13 + index * 7, 21, true});
            simulation.add_sensor({10 + index * 7, 24, 3, 3, true, false, false, {}});
        }
        simulation.add_drain({12, 34, 3, 3, false});
        simulation.add_pump({64, 34, 3, 3, false, {1.0f, 0.0f}, 10.0f});
    }
    return simulation;
}

double measure(bool devices)
{
    auto simulation = make_simulation(devices);
    constexpr int warmup_ticks = 60;
    constexpr int measured_ticks = 600;
    for (int tick = 0; tick < warmup_ticks; ++tick) simulation.step(1.0 / 120.0);
    const auto start = Clock::now();
    for (int tick = 0; tick < measured_ticks; ++tick) simulation.step(1.0 / 120.0);
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count() / measured_ticks;
}
} // namespace

int main()
{
    std::vector<double> baseline;
    std::vector<double> devices;
    for (int repetition = 0; repetition < 7; ++repetition)
    {
        baseline.push_back(measure(false));
        devices.push_back(measure(true));
    }
    std::sort(baseline.begin(), baseline.end());
    std::sort(devices.begin(), devices.end());
    const double base_median = baseline[baseline.size() / 2];
    const double device_median = devices[devices.size() / 2];
    const double overhead = base_median > 0.0 ? (device_median / base_median - 1.0) * 100.0 : 1000.0;
    std::printf("device-overhead baseline_ms=%.6f device_ms=%.6f overhead_percent=%.3f limit_percent=10.000\n",
        base_median, device_median, overhead);
    return overhead <= 10.0 ? 0 : 1;
}
