#include <physics_sim/simulation.hpp>

#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <bit>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace physics_sim
{
class Simulation::Impl
{
  public:
    explicit Impl(SimulationConfig requested) : config{sanitize(requested)}, water{config.grid_width, config.grid_height, config.cell_size}
    {
        apply_config();
    }

    static SimulationConfig sanitize(SimulationConfig config)
    {
        config.grid_width = std::max<std::size_t>(1, config.grid_width);
        config.grid_height = std::max<std::size_t>(1, config.grid_height);
        config.cell_size = config.cell_size > 0.0f ? config.cell_size : 1.0f;
        config.fixed_timestep = config.fixed_timestep > 0.0 ? config.fixed_timestep : 1.0 / 120.0;
        config.gravity_acceleration = std::max(0.0f, config.gravity_acceleration);
        return config;
    }

    void apply_config()
    {
        auto settings = WaterSimulation2D::solver_settings_for_profile(config.solver_profile);
        settings.gravity_acceleration = config.gravity_acceleration;
        water.set_solver_settings(settings);
    }

    void reset()
    {
        water = WaterSimulation2D{config.grid_width, config.grid_height, config.cell_size};
        tick = 0;
        paused = false;
        single_step_pending = false;
        apply_config();
    }

    SimulationConfig config{};
    WaterSimulation2D water{};
    std::uint64_t tick = 0;
    bool paused = false;
    bool single_step_pending = false;
};

Simulation::Simulation(const SimulationConfig &config) : impl_{std::make_unique<Impl>(config)}
{
}

Simulation::~Simulation() = default;
Simulation::Simulation(Simulation &&) noexcept = default;
Simulation &Simulation::operator=(Simulation &&) noexcept = default;

const SimulationConfig &Simulation::config() const noexcept
{
    return impl_->config;
}

bool Simulation::paused() const noexcept
{
    return impl_->paused;
}

void Simulation::apply(const SimulationCommand &command)
{
    std::visit(
        [this](const auto &value)
        {
            using Command = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Command, ClearFluidCommand>)
            {
                impl_->water.clear_fluid();
                impl_->tick = 0;
            }
            else if constexpr (std::is_same_v<Command, ResetSimulationCommand>)
            {
                impl_->reset();
            }
            else if constexpr (std::is_same_v<Command, SetSolidCellCommand>)
            {
                if (value.x >= impl_->config.grid_width || value.y >= impl_->config.grid_height)
                {
                    throw std::out_of_range{"solid cell command is outside the simulation grid"};
                }
                impl_->water.grid().set_solid(value.x, value.y, value.solid);
            }
            else if constexpr (std::is_same_v<Command, AddEmitterCommand>)
            {
                WaterEmitter emitter;
                emitter.kind = value.kind == SimulationEmitterKind::Omni ? WaterEmitterKind::Omni : WaterEmitterKind::Directional;
                emitter.position = value.position;
                emitter.direction = value.direction;
                emitter.speed = value.initial_speed;
                emitter.emission_rate = value.emission_rate;
                emitter.enabled = value.enabled;
                impl_->water.emitters().push_back(emitter);
            }
            else if constexpr (std::is_same_v<Command, ClearEmittersCommand>)
            {
                impl_->water.emitters().clear();
            }
            else if constexpr (std::is_same_v<Command, SetPausedCommand>)
            {
                impl_->paused = value.paused;
                if (!impl_->paused)
                {
                    impl_->single_step_pending = false;
                }
            }
            else if constexpr (std::is_same_v<Command, SingleStepCommand>)
            {
                if (impl_->paused)
                {
                    impl_->single_step_pending = true;
                }
            }
        },
        command);
}

bool Simulation::step()
{
    if (impl_->paused && !impl_->single_step_pending)
    {
        return false;
    }
    impl_->water.step(impl_->config.fixed_timestep);
    ++impl_->tick;
    impl_->single_step_pending = false;
    return true;
}

SimulationSnapshot Simulation::snapshot() const
{
    SimulationSnapshot result;
    result.tick = impl_->tick;
    result.grid_width = impl_->water.grid().width();
    result.grid_height = impl_->water.grid().height();
    result.cell_size = impl_->water.grid().cell_size();
    result.particles = impl_->water.particles();
    result.volume_fractions.reserve(result.grid_width * result.grid_height);
    result.solid_cells.reserve(result.grid_width * result.grid_height);
    for (std::size_t y = 0; y < result.grid_height; ++y)
    {
        for (std::size_t x = 0; x < result.grid_width; ++x)
        {
            result.volume_fractions.push_back(impl_->water.cell_volume_fraction(x, y));
            result.solid_cells.push_back(impl_->water.grid().solid(x, y) ? std::uint8_t{1} : std::uint8_t{0});
        }
    }
    return result;
}

SimulationMetrics Simulation::metrics() const noexcept
{
    const auto &source = impl_->water.metrics();
    SimulationMetrics result;
    result.tick = impl_->tick;
    result.active_particles = source.active_particles;
    result.active_cells = source.active_cells;
    result.visible_fluid_cells = source.visible_fluid_cells;
    result.pressure_active_cells = source.pressure_active_cells;
    result.total_emitted = source.total_emitted;
    result.total_removed = source.total_removed;
    result.total_outflow = source.total_outflow;
    result.total_emitted_mass = source.total_emitted_mass;
    result.total_removed_mass = source.total_removed_mass;
    result.total_outflow_mass = source.total_outflow_mass;
    result.average_divergence = source.average_divergence_after_projection;
    result.max_divergence = source.max_divergence_after_projection;
    result.average_density_error = source.average_density_error;
    result.max_density_error = source.max_density_error;
    result.kinetic_energy = source.kinetic_energy;
    result.pressure_relative_residual = source.pressure_solve.relative_residual;
    result.pressure_iterations = source.pressure_solve.iterations;
    result.pressure_converged = source.pressure_solve.converged;
    return result;
}

std::string Simulation::state_digest() const
{
    const auto state = snapshot();
    std::uint64_t hash = 14695981039346656037ULL;
    const auto mix = [&hash](std::uint64_t value)
    {
        for (int byte = 0; byte < 8; ++byte)
        {
            hash ^= static_cast<std::uint8_t>((value >> (byte * 8)) & 0xffU);
            hash *= 1099511628211ULL;
        }
    };
    const auto mix_float = [&mix](float value) { mix(std::bit_cast<std::uint32_t>(value)); };

    mix(state.tick);
    mix(state.grid_width);
    mix(state.grid_height);
    mix_float(state.cell_size);
    for (const auto &particle : state.particles)
    {
        mix_float(particle.position.x);
        mix_float(particle.position.y);
        mix_float(particle.velocity.x);
        mix_float(particle.velocity.y);
        mix_float(particle.mass);
        mix_float(particle.volume);
        mix_float(particle.density);
        mix(particle.neighbor_count);
    }
    for (const float fraction : state.volume_fractions)
    {
        mix_float(fraction);
    }
    for (const std::uint8_t solid : state.solid_cells)
    {
        mix(solid);
    }

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
}
} // namespace physics_sim
