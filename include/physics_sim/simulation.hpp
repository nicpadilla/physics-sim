#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/solver_profile.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace physics_sim
{
enum class SimulationEmitterKind
{
    Directional,
    Omni,
};

struct SimulationConfig
{
    std::size_t grid_width = 80;
    std::size_t grid_height = 45;
    float cell_size = 16.0f;
    double fixed_timestep = 1.0 / 120.0;
    float gravity_acceleration = 9.8f;
    FluidSolverProfile solver_profile = FluidSolverProfile::Balanced;
    std::uint64_t deterministic_seed = 0;
};

struct ClearFluidCommand
{
};

struct ResetSimulationCommand
{
};

struct SetSolidCellCommand
{
    std::size_t x = 0;
    std::size_t y = 0;
    bool solid = true;
};

struct AddEmitterCommand
{
    SimulationEmitterKind kind = SimulationEmitterKind::Directional;
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float initial_speed = 8.0f;
    float emission_rate = 120.0f;
    bool enabled = true;
};

struct ClearEmittersCommand
{
};

struct SetPausedCommand
{
    bool paused = true;
};

struct SingleStepCommand
{
};

using SimulationCommand = std::variant<
    ClearFluidCommand,
    ResetSimulationCommand,
    SetSolidCellCommand,
    AddEmitterCommand,
    ClearEmittersCommand,
    SetPausedCommand,
    SingleStepCommand>;

struct SimulationSnapshot
{
    std::uint64_t tick = 0;
    std::size_t grid_width = 0;
    std::size_t grid_height = 0;
    float cell_size = 1.0f;
    std::vector<FluidParticle> particles{};
    std::vector<float> volume_fractions{};
    std::vector<std::uint8_t> solid_cells{};
};

struct SimulationMetrics
{
    std::uint64_t tick = 0;
    std::size_t active_particles = 0;
    std::size_t active_cells = 0;
    std::size_t visible_fluid_cells = 0;
    std::size_t pressure_active_cells = 0;
    std::uint64_t total_emitted = 0;
    std::uint64_t total_removed = 0;
    std::uint64_t total_outflow = 0;
    double total_emitted_mass = 0.0;
    double total_removed_mass = 0.0;
    double total_outflow_mass = 0.0;
    double average_divergence = 0.0;
    double max_divergence = 0.0;
    double average_density_error = 0.0;
    double max_density_error = 0.0;
    double kinetic_energy = 0.0;
    double pressure_relative_residual = 0.0;
    std::size_t pressure_iterations = 0;
    bool pressure_converged = false;
};

class Simulation
{
public:
    explicit Simulation(const SimulationConfig& config = {});
    ~Simulation();

    Simulation(Simulation&&) noexcept;
    Simulation& operator=(Simulation&&) noexcept;
    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    [[nodiscard]] const SimulationConfig& config() const noexcept;
    [[nodiscard]] bool paused() const noexcept;
    void apply(const SimulationCommand& command);
    bool step();
    [[nodiscard]] SimulationSnapshot snapshot() const;
    [[nodiscard]] SimulationMetrics metrics() const noexcept;
    [[nodiscard]] std::string state_digest() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace physics_sim
