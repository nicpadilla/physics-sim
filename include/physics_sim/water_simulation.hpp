#pragma once

#include <physics_sim/fluid_cell_classification.hpp>
#include <physics_sim/fluid_density.hpp>
#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/grid_transfer.hpp>
#include <physics_sim/mac_grid.hpp>
#include <physics_sim/solver_profile.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace physics_sim
{
enum class WaterEmitterKind
{
    Directional,
    Omni,
};

struct WaterEmitter
{
    WaterEmitterKind kind = WaterEmitterKind::Directional;
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float speed = 6.0f;
    float emission_rate = 0.0f;
    bool enabled = true;
    double emission_accumulator = 0.0;
    std::uint64_t emitted_particles = 0;
};

struct WaterGate
{
    std::size_t x = 0;
    std::size_t y = 0;
    bool open = false;
};

struct WaterSensor
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
    bool active = false;
    bool objective = false;
    std::string label{};
};

struct WaterDrain
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
};

struct WaterPump
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
    Vec2 direction{0.0f, 1.0f};
    float strength = 8.0f;
};

struct WaterValve
{
    std::size_t x = 0;
    std::size_t y = 0;
    bool open = false;
};

enum class FluidSolverQualityTier
{
    Live,
    Offline,
};

enum class WallBoundaryMaterial
{
    FreeSlip,
    Damped,
};

struct ParticleResamplingSettings
{
    bool enabled = false;
    std::size_t min_particles_per_fluid_cell = 2;
    std::size_t target_particles_per_fluid_cell = 4;
    std::size_t max_particles_per_fluid_cell = 8;
    std::size_t max_resampling_operations_per_step = 64;
    float split_offset_fraction = 0.20f;
    float min_split_particle_mass = 1.0e-6f;
};

struct FluidSolverSettings
{
    FluidSolverProfile profile = FluidSolverProfile::Balanced;
    FluidSolverQualityTier tier = FluidSolverQualityTier::Live;
    int pressure_max_iterations = 120;
    float pressure_relative_residual_target = 1.0e-4f;
    float rest_density = 1.0f;
    std::size_t particles_per_full_cell = 4;
    float density_kernel_radius_cells = 1.0f;
    int density_correction_iterations = 0;
    float max_density_correction_fraction = 0.2f;
    float flip_blend = 0.95f;
    float velocity_retention = 0.99f;
    float apic_affine_ratio = 0.0f;
    WallBoundaryMaterial wall_material = WallBoundaryMaterial::FreeSlip;
    float wall_tangential_velocity_retention = 1.0f;
    float viscosity_coefficient = 0.0f;
    float surface_tension_coefficient = 0.0f;
    float max_surface_velocity_delta_fraction = 0.15f;
    ParticleResamplingSettings resampling{};
    std::uint64_t density_metrics_interval_ticks = 120;
};

struct PressureSolveResult
{
    int iterations = 0;
    int max_iterations = 0;
    float target_relative_residual = 0.0f;
    float target_absolute_residual = 0.0f;
    float initial_residual = 0.0f;
    float final_residual = 0.0f;
    float absolute_residual = 0.0f;
    float relative_residual = 0.0f;
    float rhs_l2 = 0.0f;
    float solution_l2 = 0.0f;
    float pressure_dt = 0.0f;
    float rest_density = 0.0f;
    bool converged = false;
    std::size_t visible_cells = 0;
    std::size_t active_cells = 0;
    std::size_t pressure_active_cells = 0;
    double active_cell_overreach_ratio = 0.0;
};

struct WaterSimulationMetrics
{
    std::uint64_t total_emitted = 0;
    double total_emitted_mass = 0.0;
    std::uint64_t total_removed = 0;
    double total_removed_mass = 0.0;
    std::uint64_t total_outflow = 0;
    double total_outflow_mass = 0.0;
    std::size_t active_particles = 0;
    std::size_t active_cells = 0;
    std::size_t visible_fluid_cells = 0;
    std::size_t pressure_active_cells = 0;
    double active_cell_overreach_ratio = 0.0;
    std::size_t active_sensors = 0;
    std::size_t objective_sensors = 0;
    bool objective_completed = false;
    double average_divergence_after_projection = 0.0;
    double max_divergence_after_projection = 0.0;
    double min_density = 0.0;
    double max_density = 0.0;
    double average_density = 0.0;
    double average_density_error = 0.0;
    double max_density_error = 0.0;
    double average_neighbor_count = 0.0;
    std::size_t max_neighbor_count = 0;
    double kinetic_energy = 0.0;
    PressureSolveResult pressure_solve{};
};

class WaterSimulation2D
{
public:
    using size_type = std::size_t;

    WaterSimulation2D() = default;

    WaterSimulation2D(size_type width, size_type height, float cell_size = 1.0f)
    {
        resize(width, height, cell_size);
    }

    void resize(size_type width, size_type height, float cell_size = 1.0f)
    {
        grid_.resize(width, height, cell_size);
        particles_.clear();
        emitters_.clear();
        gates_.clear();
        sensors_.clear();
        drains_.clear();
        pumps_.clear();
        valves_.clear();
        fluid_cells_.assign(grid_.cell_count(), 0U);
        cell_states_.assign(grid_.cell_count(), FluidCellState::Air);
        cell_volume_fractions_.assign(grid_.cell_count(), 0.0f);
        cell_densities_.assign(grid_.cell_count(), 0.0f);
        u_previous_.assign(grid_.u_count(), 0.0f);
        v_previous_.assign(grid_.v_count(), 0.0f);
        pressure_next_.assign(grid_.cell_count(), 0.0f);
        total_emitted_ = 0;
        total_emitted_mass_ = 0.0;
        total_removed_ = 0;
        total_removed_mass_ = 0.0;
        total_outflow_ = 0;
        total_outflow_mass_ = 0.0;
        metrics_ = {};
    }

    [[nodiscard]] MacGrid2D& grid() noexcept
    {
        return grid_;
    }

    [[nodiscard]] const MacGrid2D& grid() const noexcept
    {
        return grid_;
    }

    [[nodiscard]] std::vector<FluidParticle>& particles() noexcept
    {
        return particles_;
    }

    [[nodiscard]] const std::vector<FluidParticle>& particles() const noexcept
    {
        return particles_;
    }

    [[nodiscard]] std::vector<WaterEmitter>& emitters() noexcept
    {
        return emitters_;
    }

    [[nodiscard]] const std::vector<WaterEmitter>& emitters() const noexcept
    {
        return emitters_;
    }

    [[nodiscard]] std::vector<WaterGate>& gates() noexcept
    {
        return gates_;
    }

    [[nodiscard]] const std::vector<WaterGate>& gates() const noexcept
    {
        return gates_;
    }

    [[nodiscard]] std::vector<WaterSensor>& sensors() noexcept
    {
        return sensors_;
    }

    [[nodiscard]] const std::vector<WaterSensor>& sensors() const noexcept
    {
        return sensors_;
    }

    [[nodiscard]] std::vector<WaterDrain>& drains() noexcept
    {
        return drains_;
    }

    [[nodiscard]] const std::vector<WaterDrain>& drains() const noexcept
    {
        return drains_;
    }

    [[nodiscard]] std::vector<WaterPump>& pumps() noexcept
    {
        return pumps_;
    }

    [[nodiscard]] const std::vector<WaterPump>& pumps() const noexcept
    {
        return pumps_;
    }

    [[nodiscard]] std::vector<WaterValve>& valves() noexcept
    {
        return valves_;
    }

    [[nodiscard]] const std::vector<WaterValve>& valves() const noexcept
    {
        return valves_;
    }

    [[nodiscard]] const WaterSimulationMetrics& metrics() const noexcept
    {
        return metrics_;
    }

    [[nodiscard]] static FluidSolverSettings solver_settings_for_profile(FluidSolverProfile profile) noexcept
    {
        FluidSolverSettings settings;
        settings.profile = profile;

        switch (profile)
        {
        case FluidSolverProfile::Fast:
            settings.tier = FluidSolverQualityTier::Live;
            settings.pressure_max_iterations = 120;
            settings.pressure_relative_residual_target = 1.0e-4f;
            settings.rest_density = 1.0f;
            settings.particles_per_full_cell = 1;
            settings.density_kernel_radius_cells = 0.5f;
            settings.density_correction_iterations = 0;
            settings.max_density_correction_fraction = 0.2f;
            settings.flip_blend = 0.95f;
            settings.velocity_retention = 0.99f;
            settings.apic_affine_ratio = 0.0f;
            settings.viscosity_coefficient = 0.0f;
            settings.surface_tension_coefficient = 0.0f;
            settings.max_surface_velocity_delta_fraction = 0.15f;
            settings.resampling = {};
            settings.density_metrics_interval_ticks = 120;
            break;

        case FluidSolverProfile::Balanced:
            settings.tier = FluidSolverQualityTier::Live;
            settings.pressure_max_iterations = 120;
            settings.pressure_relative_residual_target = 1.0e-2f;
            settings.rest_density = 1.0f;
            settings.particles_per_full_cell = 6;
            settings.density_kernel_radius_cells = 1.2f;
            settings.density_correction_iterations = 1;
            settings.max_density_correction_fraction = 0.02f;
            settings.flip_blend = 0.65f;
            settings.velocity_retention = 0.92f;
            settings.apic_affine_ratio = 0.0f;
            settings.viscosity_coefficient = 0.025f;
            settings.surface_tension_coefficient = 0.0f;
            settings.max_surface_velocity_delta_fraction = 0.10f;
            settings.resampling.enabled = true;
            settings.resampling.min_particles_per_fluid_cell = 1;
            settings.resampling.target_particles_per_fluid_cell = 6;
            settings.resampling.max_particles_per_fluid_cell = 10;
            settings.resampling.max_resampling_operations_per_step = 40;
            settings.resampling.split_offset_fraction = 0.16f;
            settings.resampling.min_split_particle_mass = 1.0e-6f;
            settings.density_metrics_interval_ticks = 60;
            break;

        case FluidSolverProfile::Quality:
            settings.tier = FluidSolverQualityTier::Offline;
            settings.pressure_max_iterations = 260;
            settings.pressure_relative_residual_target = 5.0e-2f;
            settings.rest_density = 1.0f;
            settings.particles_per_full_cell = 6;
            settings.density_kernel_radius_cells = 1.2f;
            settings.density_correction_iterations = 1;
            settings.max_density_correction_fraction = 0.03f;
            settings.flip_blend = 0.72f;
            settings.velocity_retention = 0.985f;
            settings.apic_affine_ratio = 0.10f;
            settings.viscosity_coefficient = 0.025f;
            settings.surface_tension_coefficient = 0.04f;
            settings.max_surface_velocity_delta_fraction = 0.07f;
            settings.resampling.enabled = true;
            settings.resampling.min_particles_per_fluid_cell = 1;
            settings.resampling.target_particles_per_fluid_cell = 7;
            settings.resampling.max_particles_per_fluid_cell = 14;
            settings.resampling.max_resampling_operations_per_step = 48;
            settings.resampling.split_offset_fraction = 0.16f;
            settings.resampling.min_split_particle_mass = 1.0e-6f;
            settings.density_metrics_interval_ticks = 120;
            break;
        }

        return settings;
    }

    [[nodiscard]] static FluidSolverSettings live_solver_settings() noexcept
    {
        return solver_settings_for_profile(FluidSolverProfile::Fast);
    }

    [[nodiscard]] static FluidSolverSettings offline_solver_settings() noexcept
    {
        return solver_settings_for_profile(FluidSolverProfile::Quality);
    }

    [[nodiscard]] const FluidSolverSettings& solver_settings() const noexcept
    {
        return solver_settings_;
    }

    void set_solver_settings(const FluidSolverSettings& settings) noexcept
    {
        solver_settings_ = sanitize_solver_settings(settings);
    }

    void refresh_density_metrics() noexcept
    {
        const FluidDensityMetrics density_metrics = update_particle_density_metrics(particles_, density_settings());
        metrics_.min_density = density_metrics.min_density;
        metrics_.max_density = density_metrics.max_density;
        metrics_.average_density = density_metrics.average_density;
        metrics_.average_density_error = density_metrics.average_density_error;
        metrics_.max_density_error = density_metrics.max_density_error;
        metrics_.average_neighbor_count = density_metrics.average_neighbor_count;
        metrics_.max_neighbor_count = density_metrics.max_neighbor_count;
    }

    void refresh_cell_classification() noexcept
    {
        const FluidCellClassification classification = classify_fluid_cells(grid_, particles_, density_settings());
        cell_states_ = classification.states;
        cell_volume_fractions_ = classification.volume_fractions;
        cell_densities_ = classification.densities;
    }

    void ensure_particle_transfer_properties() noexcept
    {
        const FluidDensitySettings settings = density_settings();
        for (auto& particle : particles_)
        {
            particle.volume = particle.volume > 0.0f ? particle.volume : settings.particle_volume;
            particle.mass = particle.mass > 0.0f ? particle.mass : settings.rest_density * particle.volume;
        }
    }

    [[nodiscard]] FluidCellState cell_state(size_type x, size_type y) const noexcept
    {
        if (!grid_.contains(x, y) || cell_states_.size() != grid_.cell_count())
        {
            return FluidCellState::Air;
        }
        return cell_states_[grid_.cell_index(x, y)];
    }

    [[nodiscard]] float cell_volume_fraction(size_type x, size_type y) const noexcept
    {
        if (!grid_.contains(x, y) || cell_volume_fractions_.size() != grid_.cell_count())
        {
            return 0.0f;
        }
        return cell_volume_fractions_[grid_.cell_index(x, y)];
    }

    [[nodiscard]] float cell_density(size_type x, size_type y) const noexcept
    {
        if (!grid_.contains(x, y) || cell_densities_.size() != grid_.cell_count())
        {
            return 0.0f;
        }
        return cell_densities_[grid_.cell_index(x, y)];
    }

    void add_emitter(const WaterEmitter& emitter)
    {
        emitters_.push_back(emitter);
    }

    void add_gate(const WaterGate& gate)
    {
        gates_.push_back(gate);
        sync_gate_cells();
    }

    void add_sensor(const WaterSensor& sensor)
    {
        sensors_.push_back(sensor);
        recount_sensor_metrics_from_state();
    }

    void add_drain(const WaterDrain& drain)
    {
        drains_.push_back(drain);
    }

    void add_pump(const WaterPump& pump)
    {
        pumps_.push_back(pump);
    }

    void add_valve(const WaterValve& valve)
    {
        valves_.push_back(valve);
        sync_gate_cells();
    }

    [[nodiscard]] bool toggle_gate_open(std::size_t index)
    {
        if (index >= gates_.size())
        {
            return false;
        }

        gates_[index].open = !gates_[index].open;
        sync_gate_cells();
        return true;
    }

    [[nodiscard]] bool set_gate_open(std::size_t index, bool open)
    {
        if (index >= gates_.size())
        {
            return false;
        }

        gates_[index].open = open;
        sync_gate_cells();
        return true;
    }

    [[nodiscard]] bool toggle_valve_open(std::size_t index)
    {
        if (index >= valves_.size())
        {
            return false;
        }

        valves_[index].open = !valves_[index].open;
        sync_gate_cells();
        return true;
    }

    [[nodiscard]] bool set_valve_open(std::size_t index, bool open)
    {
        if (index >= valves_.size())
        {
            return false;
        }

        valves_[index].open = open;
        sync_gate_cells();
        return true;
    }

    void refresh_sensor_states() noexcept
    {
        update_sensor_states();
    }

    void add_particle(const FluidParticle& particle)
    {
        particles_.push_back(particle);
    }

    void set_solid_cell(size_type x, size_type y, bool value)
    {
        if (grid_.contains(x, y))
        {
            grid_.set_solid(x, y, value);
        }
    }

    void paint_wall_line(Vec2 start, Vec2 end, bool solid)
    {
        if (grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        const auto to_cell_x = [&](float value) -> int
        {
            return static_cast<int>(std::floor(value / grid_.cell_size()));
        };

        const auto to_cell_y = [&](float value) -> int
        {
            return static_cast<int>(std::floor(value / grid_.cell_size()));
        };

        int x0 = to_cell_x(start.x);
        int y0 = to_cell_y(start.y);
        const int x1 = to_cell_x(end.x);
        const int y1 = to_cell_y(end.y);

        const int dx = std::abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (true)
        {
            if (x0 >= 0 && y0 >= 0)
            {
                set_solid_cell(static_cast<size_type>(x0), static_cast<size_type>(y0), solid);
            }

            if (x0 == x1 && y0 == y1)
            {
                break;
            }

            const int twice_err = err * 2;
            if (twice_err >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (twice_err <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void clear_fluid() noexcept
    {
        particles_.clear();
        grid_.clear_fields();
        fluid_cells_.assign(grid_.cell_count(), 0U);
        refresh_cell_classification();
        total_emitted_ = 0;
        total_emitted_mass_ = 0.0;
        total_removed_ = 0;
        total_removed_mass_ = 0.0;
        total_outflow_ = 0;
        total_outflow_mass_ = 0.0;
        simulation_tick_ = 0;
        metrics_ = {};
        sync_gate_cells();
        update_sensor_states();
        for (auto& emitter : emitters_)
        {
            emitter.emission_accumulator = 0.0;
            emitter.emitted_particles = 0;
        }
    }

    void clear_scene() noexcept
    {
        clear_fluid();
        grid_.clear_solids();
        emitters_.clear();
        gates_.clear();
        sensors_.clear();
        drains_.clear();
        pumps_.clear();
        valves_.clear();
        metrics_ = {};
    }

    void step(double dt)
    {
        step(dt, std::span<WaterEmitter>{});
    }

    void step(double dt, std::span<WaterEmitter> transient_emitters)
    {
        if (dt <= 0.0 || grid_.width() == 0 || grid_.height() == 0)
        {
            metrics_ = {};
            return;
        }

        const float step_seconds = static_cast<float>(dt);
        const float gravity = 9.8f;
        const float inv_cell_size = 1.0f / grid_.cell_size();

        sync_gate_cells();
        cull_out_of_domain_particles();
        emit_particles(step_seconds, transient_emitters);
        cull_out_of_domain_particles();
        remove_particles_in_drains();
        apply_pump_forces(step_seconds);

        for (auto& particle : particles_)
        {
            particle.velocity.y += gravity * step_seconds;
        }

        fluid_cells_.assign(grid_.cell_count(), 0U);
        grid_.clear_fields();

        scatter_particles_to_grid();
        apply_boundary_conditions();
        refresh_cell_classification();
        mark_fluid_cells();
        const std::size_t visible_fluid_cells = visible_fluid_cell_count();
        const std::size_t pressure_active_cells = pressure_active_cell_count();

        if (solver_settings_.viscosity_coefficient > 0.0f)
        {
            apply_viscosity(step_seconds);
        }

        if (solver_settings_.surface_tension_coefficient > 0.0f)
        {
            apply_surface_tension(step_seconds);
        }

        apply_boundary_conditions();

        u_previous_ = collect_u();
        v_previous_ = collect_v();

        const PressureSolveResult pressure_solve = project_pressures(step_seconds);
        apply_pressure_gradient(step_seconds);
        apply_boundary_conditions();

        double divergence_sum = 0.0;
        double divergence_max = 0.0;
        std::size_t divergence_count = 0;
        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = grid_.cell_index(x, y);
                if (!is_active_cell(idx))
                {
                    grid_.divergence(x, y) = 0.0f;
                    continue;
                }

                const float divergence = compute_divergence_for_cell(x, y, inv_cell_size);
                grid_.divergence(x, y) = divergence;
                divergence_sum += std::abs(static_cast<double>(divergence));
                divergence_max = std::max(divergence_max, std::abs(static_cast<double>(divergence)));
                ++divergence_count;
            }
        }

        transfer_grid_to_particles(step_seconds);
        if (solver_settings_.density_correction_iterations > 0)
        {
            apply_local_density_correction(step_seconds);
        }
        advect_particles(step_seconds);
        remove_particles_in_drains();
        cull_out_of_domain_particles();
        if (solver_settings_.resampling.enabled)
        {
            refresh_cell_classification();
            resample_particles();
            remove_particles_in_drains();
            cull_out_of_domain_particles();
            refresh_cell_classification();
        }

        metrics_.total_emitted = total_emitted_;
        metrics_.total_emitted_mass = total_emitted_mass_;
        metrics_.total_removed = total_removed_;
        metrics_.total_removed_mass = total_removed_mass_;
        metrics_.total_outflow = total_outflow_;
        metrics_.total_outflow_mass = total_outflow_mass_;
        metrics_.active_particles = particles_.size();
        metrics_.active_cells = divergence_count;
        metrics_.visible_fluid_cells = visible_fluid_cells;
        metrics_.pressure_active_cells = pressure_active_cells;
        metrics_.active_cell_overreach_ratio = visible_fluid_cells > 0
            ? static_cast<double>(pressure_active_cells) / static_cast<double>(visible_fluid_cells)
            : 0.0;
        metrics_.pressure_solve = pressure_solve;
        update_sensor_states();
        metrics_.average_divergence_after_projection = divergence_count > 0 ? divergence_sum / static_cast<double>(divergence_count) : 0.0;
        metrics_.max_divergence_after_projection = divergence_max;
        double kinetic_energy = 0.0;
        for (const auto& particle : particles_)
        {
            const double particle_mass = particle.mass > 0.0f
                ? static_cast<double>(particle.mass)
                : static_cast<double>(particle.volume > 0.0f ? particle.volume : density_settings().particle_volume);
            const double speed_squared = static_cast<double>(particle.velocity.x) * static_cast<double>(particle.velocity.x)
                + static_cast<double>(particle.velocity.y) * static_cast<double>(particle.velocity.y);
            kinetic_energy += 0.5 * particle_mass * speed_squared;
        }
        metrics_.kinetic_energy = kinetic_energy;
        ++simulation_tick_;
        if (particles_.size() <= 128 || simulation_tick_ % solver_settings_.density_metrics_interval_ticks == 0)
        {
            refresh_density_metrics();
        }
    }

private:
    [[nodiscard]] bool is_valid_cell(size_type x, size_type y) const noexcept
    {
        return grid_.contains(x, y);
    }

    [[nodiscard]] size_type safe_cell_index(size_type x, size_type y) const noexcept
    {
        return grid_.cell_index(x, y);
    }

    [[nodiscard]] bool is_active_cell(size_type index) const noexcept
    {
        return index < fluid_cells_.size() && fluid_cells_[index] != 0U;
    }

    [[nodiscard]] std::size_t pressure_active_cell_count() const noexcept
    {
        return static_cast<std::size_t>(std::count(fluid_cells_.begin(), fluid_cells_.end(), static_cast<std::uint8_t>(1U)));
    }

    [[nodiscard]] std::size_t visible_fluid_cell_count() const noexcept
    {
        return static_cast<std::size_t>(std::count(cell_states_.begin(), cell_states_.end(), FluidCellState::Fluid));
    }

    [[nodiscard]] FluidDensitySettings density_settings() const noexcept
    {
        const float cell_size = grid_.cell_size() > 0.0f ? grid_.cell_size() : 1.0f;
        const float particles_per_cell = static_cast<float>(std::max<std::size_t>(1, solver_settings_.particles_per_full_cell));
        FluidDensitySettings settings;
        settings.rest_density = solver_settings_.rest_density > 0.0f ? solver_settings_.rest_density : 1.0f;
        settings.particle_volume = (cell_size * cell_size) / particles_per_cell;
        settings.kernel_radius = cell_size * std::max(0.25f, solver_settings_.density_kernel_radius_cells);
        return settings;
    }

    void sync_gate_cells() noexcept
    {
        if (grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        for (const auto& gate : gates_)
        {
            if (grid_.contains(gate.x, gate.y))
            {
                grid_.set_solid(gate.x, gate.y, !gate.open);
            }
        }

        for (const auto& valve : valves_)
        {
            if (grid_.contains(valve.x, valve.y))
            {
                grid_.set_solid(valve.x, valve.y, !valve.open);
            }
        }
    }

    [[nodiscard]] bool region_contains(std::size_t left, std::size_t top, std::size_t width, std::size_t height, const Vec2& position) const noexcept
    {
        const float cell_size = grid_.cell_size();
        const float region_left = static_cast<float>(left) * cell_size;
        const float region_top = static_cast<float>(top) * cell_size;
        const float region_right = region_left + static_cast<float>(width) * cell_size;
        const float region_bottom = region_top + static_cast<float>(height) * cell_size;
        return position.x >= region_left && position.x < region_right && position.y >= region_top && position.y < region_bottom;
    }

    void remove_particles_in_drains() noexcept
    {
        if (particles_.empty() || drains_.empty())
        {
            return;
        }

        std::vector<FluidParticle> surviving;
        surviving.reserve(particles_.size());

        std::uint64_t removed_this_pass = 0;
        double removed_mass_this_pass = 0.0;
        for (const auto& particle : particles_)
        {
            bool drained = false;
            for (const auto& drain : drains_)
            {
                if (!drain.enabled)
                {
                    continue;
                }

                if (region_contains(drain.x, drain.y, drain.width, drain.height, particle.position))
                {
                    drained = true;
                    break;
                }
            }

            if (drained)
            {
                ++removed_this_pass;
                removed_mass_this_pass += particle_mass_for_metrics(particle);
                continue;
            }

            surviving.push_back(particle);
        }

        if (removed_this_pass != 0)
        {
            total_removed_ += removed_this_pass;
            total_removed_mass_ += removed_mass_this_pass;
        }

        particles_.swap(surviving);
    }

    void apply_pump_forces(float step_seconds) noexcept
    {
        if (particles_.empty() || pumps_.empty() || step_seconds <= 0.0f)
        {
            return;
        }

        for (const auto& pump : pumps_)
        {
            if (!pump.enabled)
            {
                continue;
            }

            const Vec2 direction = normalized_or_default(pump.direction, Vec2{0.0f, 1.0f});
            const float strength = std::max(0.0f, pump.strength) * step_seconds;

            for (auto& particle : particles_)
            {
                if (region_contains(pump.x, pump.y, pump.width, pump.height, particle.position))
                {
                    particle.velocity = particle.velocity + direction * strength;
                }
            }
        }
    }

    void update_sensor_states() noexcept
    {
        for (auto& sensor : sensors_)
        {
            bool active = false;
            if (sensor.enabled && !particles_.empty() && grid_.width() > 0 && grid_.height() > 0)
            {
                const float cell_size = grid_.cell_size();
                const float left = static_cast<float>(sensor.x) * cell_size;
                const float top = static_cast<float>(sensor.y) * cell_size;
                const float right = left + static_cast<float>(sensor.width) * cell_size;
                const float bottom = top + static_cast<float>(sensor.height) * cell_size;

                for (const auto& particle : particles_)
                {
                    if (particle.position.x >= left
                        && particle.position.x < right
                        && particle.position.y >= top
                        && particle.position.y < bottom)
                    {
                        active = true;
                        break;
                    }
                }
            }

            sensor.active = active;
        }

        recount_sensor_metrics_from_state();
    }

    void recount_sensor_metrics_from_state() noexcept
    {
        std::size_t active_sensors = 0;
        std::size_t objective_sensors = 0;
        bool objective_completed = true;

        for (const auto& sensor : sensors_)
        {
            if (sensor.active)
            {
                ++active_sensors;
            }

            if (sensor.objective && sensor.enabled)
            {
                ++objective_sensors;
                objective_completed = objective_completed && sensor.active;
            }
        }

        metrics_.active_sensors = active_sensors;
        metrics_.objective_sensors = objective_sensors;
        metrics_.objective_completed = objective_sensors > 0 && objective_completed;
    }

    [[nodiscard]] float domain_width() const noexcept
    {
        return static_cast<float>(grid_.width()) * grid_.cell_size();
    }

    [[nodiscard]] float domain_height() const noexcept
    {
        return static_cast<float>(grid_.height()) * grid_.cell_size();
    }

    [[nodiscard]] bool is_out_of_domain(const Vec2& position) const noexcept
    {
        return position.x < 0.0f
            || position.y < 0.0f
            || position.x >= domain_width()
            || position.y >= domain_height();
    }

    void cull_out_of_domain_particles() noexcept
    {
        if (particles_.empty())
        {
            return;
        }

        std::vector<FluidParticle> surviving;
        surviving.reserve(particles_.size());

        std::uint64_t removed_this_pass = 0;
        double removed_mass_this_pass = 0.0;
        for (const auto& particle : particles_)
        {
            if (is_out_of_domain(particle.position))
            {
                ++removed_this_pass;
                removed_mass_this_pass += particle_mass_for_metrics(particle);
                continue;
            }

            surviving.push_back(particle);
        }

        if (removed_this_pass != 0)
        {
            total_removed_ += removed_this_pass;
            total_outflow_ += removed_this_pass;
            total_outflow_mass_ += removed_mass_this_pass;
        }

        particles_.swap(surviving);
    }

    [[nodiscard]] static float clampf(float value, float min_value, float max_value) noexcept
    {
        return value < min_value ? min_value : (value > max_value ? max_value : value);
    }

    [[nodiscard]] static Vec2 normalized_or_default(Vec2 value, Vec2 fallback) noexcept
    {
        const float len = length(value);
        if (len <= 0.0f)
        {
            return fallback;
        }
        return value / len;
    }

    [[nodiscard]] static float lerp(float a, float b, float t) noexcept
    {
        return a + (b - a) * t;
    }

    template <typename IndexFn>
    [[nodiscard]] static float bilinear_sample(
        const std::vector<float>& values,
        size_type width,
        size_type height,
        float x,
        float y,
        IndexFn&& index_fn) noexcept
    {
        if (width == 0 || height == 0 || values.empty())
        {
            return 0.0f;
        }

        if (width == 1 && height == 1)
        {
            return values.front();
        }

        const float x_clamped = width > 1 ? clampf(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
        const float y_clamped = height > 1 ? clampf(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
        const size_type x0 = static_cast<size_type>(std::floor(x_clamped));
        const size_type y0 = static_cast<size_type>(std::floor(y_clamped));
        const size_type x1 = std::min(x0 + 1, width - 1);
        const size_type y1 = std::min(y0 + 1, height - 1);
        const float tx = x_clamped - static_cast<float>(x0);
        const float ty = y_clamped - static_cast<float>(y0);

        const float v00 = values[index_fn(x0, y0)];
        const float v10 = values[index_fn(x1, y0)];
        const float v01 = values[index_fn(x0, y1)];
        const float v11 = values[index_fn(x1, y1)];
        const float a = lerp(v00, v10, tx);
        const float b = lerp(v01, v11, tx);
        return lerp(a, b, ty);
    }

    template <typename IndexFn>
    static void bilinear_scatter(
        std::vector<float>& values,
        std::vector<float>& weights,
        size_type width,
        size_type height,
        float x,
        float y,
        float contribution,
        IndexFn&& index_fn) noexcept
    {
        if (width == 0 || height == 0 || values.empty())
        {
            return;
        }

        const float x_clamped = width > 1 ? clampf(x, 0.0f, static_cast<float>(width - 1) - 0.0001f) : 0.0f;
        const float y_clamped = height > 1 ? clampf(y, 0.0f, static_cast<float>(height - 1) - 0.0001f) : 0.0f;
        const size_type x0 = static_cast<size_type>(std::floor(x_clamped));
        const size_type y0 = static_cast<size_type>(std::floor(y_clamped));
        const size_type x1 = std::min(x0 + 1, width - 1);
        const size_type y1 = std::min(y0 + 1, height - 1);
        const float tx = x_clamped - static_cast<float>(x0);
        const float ty = y_clamped - static_cast<float>(y0);

        const float w00 = (1.0f - tx) * (1.0f - ty);
        const float w10 = tx * (1.0f - ty);
        const float w01 = (1.0f - tx) * ty;
        const float w11 = tx * ty;

        const auto accumulate = [&](size_type x_index, size_type y_index, float w) {
            const size_type idx = index_fn(x_index, y_index);
            values[idx] += contribution * w;
            weights[idx] += w;
        };

        accumulate(x0, y0, w00);
        accumulate(x1, y0, w10);
        accumulate(x0, y1, w01);
        accumulate(x1, y1, w11);
    }

    [[nodiscard]] Vec2 sample_velocity(const Vec2& position,
                                       const std::vector<float>& u_values,
                                       const std::vector<float>& v_values) const noexcept
    {
        const float inv = 1.0f / grid_.cell_size();
        const float u = bilinear_sample(
            u_values,
            grid_.width() + 1,
            grid_.height(),
            position.x * inv,
            position.y * inv - 0.5f,
            [&](size_type x, size_type y) { return y * (grid_.width() + 1) + x; });
        const float v = bilinear_sample(
            v_values,
            grid_.width(),
            grid_.height() + 1,
            position.x * inv - 0.5f,
            position.y * inv,
            [&](size_type x, size_type y) { return y * grid_.width() + x; });
        return {u, v};
    }

    void emit_from_emitter(WaterEmitter& emitter, float dt)
    {
        if (!emitter.enabled || emitter.emission_rate <= 0.0f)
        {
            return;
        }

        const double calibrated_particle_rate = static_cast<double>(emitter.emission_rate)
            * static_cast<double>(std::max<std::size_t>(1, solver_settings_.particles_per_full_cell));
        emitter.emission_accumulator += calibrated_particle_rate * static_cast<double>(dt);
        const auto direction = normalized_or_default(emitter.direction, Vec2{0.0f, 1.0f});

        while (emitter.emission_accumulator >= 1.0)
        {
            emitter.emission_accumulator -= 1.0;

            Vec2 spawn_position = emitter.position;
            Vec2 spawn_velocity = direction * emitter.speed * grid_.cell_size();
            const double seed = static_cast<double>(emitter.emitted_particles + 1);
            const double phase = std::fmod(seed * 0.6180339887498948482, 1.0);
            const float centered_phase = static_cast<float>(phase - 0.5);
            const Vec2 tangent{-direction.y, direction.x};
            const float aperture = grid_.cell_size() * 0.45f;
            spawn_position = spawn_position + tangent * (centered_phase * aperture);

            if (emitter.kind == WaterEmitterKind::Omni)
            {
                const double angle = std::fmod(seed * 2.39996322972865332, 6.2831853071795864769);
                const float c = static_cast<float>(std::cos(angle));
                const float s = static_cast<float>(std::sin(angle));
                Vec2 radial{c, s};
                spawn_velocity = radial * emitter.speed * grid_.cell_size();
                spawn_position = emitter.position + radial * (aperture * 0.5f);
            }

            const float particle_volume = density_settings().particle_volume;
            const float particle_mass = density_settings().rest_density * particle_volume;
            particles_.push_back({spawn_position, spawn_velocity, particle_mass, particle_volume});
            ++emitter.emitted_particles;
            ++total_emitted_;
            total_emitted_mass_ += static_cast<double>(particle_mass);
        }
    }

    void emit_particles(float dt, std::span<WaterEmitter> transient_emitters)
    {
        for (auto& emitter : emitters_)
        {
            emit_from_emitter(emitter, dt);
        }

        for (auto& emitter : transient_emitters)
        {
            emit_from_emitter(emitter, dt);
        }
    }

    void scatter_particles_to_grid()
    {
        u_weights_.assign(grid_.u_count(), 0.0f);
        v_weights_.assign(grid_.v_count(), 0.0f);
        if (u_previous_.size() != grid_.u_count())
        {
            u_previous_.assign(grid_.u_count(), 0.0f);
        }
        if (v_previous_.size() != grid_.v_count())
        {
            v_previous_.assign(grid_.v_count(), 0.0f);
        }

        ensure_particle_transfer_properties();
        const ParticleGridTransferSummary transfer_summary = scatter_particles_apic_to_grid(grid_, particles_, u_weights_, v_weights_);
        (void)transfer_summary;
        normalize_mac_grid_faces_by_mass(grid_, u_weights_, v_weights_);
    }

    void normalize_faces()
    {
        for (size_type i = 0; i < grid_.u_count(); ++i)
        {
            const float weight = u_weights_[i];
            grid_.u_raw()[i] = weight > 0.0f ? grid_.u_raw()[i] / weight : 0.0f;
        }
        for (size_type i = 0; i < grid_.v_count(); ++i)
        {
            const float weight = v_weights_[i];
            grid_.v_raw()[i] = weight > 0.0f ? grid_.v_raw()[i] / weight : 0.0f;
        }
    }

    void apply_boundary_conditions()
    {
        const auto width = grid_.width();
        const auto height = grid_.height();

        for (size_type y = 0; y < height; ++y)
        {
            grid_.u(0, y) = 0.0f;
            grid_.u(width, y) = 0.0f;
            for (size_type x = 0; x < width; ++x)
            {
                if (grid_.solid(x, y))
                {
                    grid_.u(x, y) = 0.0f;
                    grid_.u(x + 1, y) = 0.0f;
                    grid_.v(x, y) = 0.0f;
                    grid_.v(x, y + 1) = 0.0f;
                }
                if (x + 1 < width && grid_.solid(x, y))
                {
                    grid_.u(x + 1, y) = 0.0f;
                }
            }
        }

        for (size_type x = 0; x < width; ++x)
        {
            grid_.v(x, 0) = 0.0f;
            grid_.v(x, height) = 0.0f;
        }

        for (size_type y = 0; y < height; ++y)
        {
            for (size_type x = 0; x < width; ++x)
            {
                if (grid_.solid(x, y))
                {
                    if (x > 0)
                    {
                        grid_.u(x, y) = 0.0f;
                    }
                    if (x + 1 <= width)
                    {
                        grid_.u(x + 1, y) = 0.0f;
                    }
                    if (y > 0)
                    {
                        grid_.v(x, y) = 0.0f;
                    }
                    if (y + 1 <= height)
                    {
                        grid_.v(x, y + 1) = 0.0f;
                    }
                }
            }
        }
    }

    void mark_fluid_cells()
    {
        if (cell_volume_fractions_.size() != grid_.cell_count())
        {
            return;
        }

        constexpr float active_fraction_threshold = 1.0e-5f;
        constexpr float halo_fraction_threshold = 0.15f;
        const bool use_full_halo = solver_settings_.tier == FluidSolverQualityTier::Live;
        const auto activate_cell = [&](int x, int y)
        {
            if (x < 0 || y < 0 || static_cast<size_type>(x) >= grid_.width() || static_cast<size_type>(y) >= grid_.height())
            {
                return;
            }

            const size_type sx = static_cast<size_type>(x);
            const size_type sy = static_cast<size_type>(y);
            if (!grid_.solid(sx, sy))
            {
                fluid_cells_[safe_cell_index(sx, sy)] = 1U;
            }
        };

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = safe_cell_index(x, y);
                if (!grid_.solid(x, y) && cell_volume_fractions_[idx] > active_fraction_threshold)
                {
                    const int ix = static_cast<int>(x);
                    const int iy = static_cast<int>(y);
                    activate_cell(ix, iy);
                    if (use_full_halo)
                    {
                        activate_cell(ix - 1, iy);
                        activate_cell(ix + 1, iy);
                        activate_cell(ix, iy - 1);
                        activate_cell(ix, iy + 1);
                    }
                    else if (cell_volume_fractions_[idx] > halo_fraction_threshold)
                    {
                        activate_cell(ix - 1, iy);
                        activate_cell(ix + 1, iy);
                    }
                }
            }
        }
    }

    [[nodiscard]] float sample_volume_fraction(int x, int y) const noexcept
    {
        if (x < 0 || y < 0)
        {
            return 0.0f;
        }

        const size_type sx = static_cast<size_type>(x);
        const size_type sy = static_cast<size_type>(y);
        if (!grid_.contains(sx, sy) || grid_.solid(sx, sy) || cell_volume_fractions_.size() != grid_.cell_count())
        {
            return 0.0f;
        }

        return cell_volume_fractions_[grid_.cell_index(sx, sy)];
    }

    [[nodiscard]] Vec2 volume_fraction_normal(int x, int y) const noexcept
    {
        const float dx = grid_.cell_size() > 0.0f ? grid_.cell_size() : 1.0f;
        const float inv_dx = 1.0f / dx;
        const Vec2 gradient{
            (sample_volume_fraction(x + 1, y) - sample_volume_fraction(x - 1, y)) * 0.5f * inv_dx,
            (sample_volume_fraction(x, y + 1) - sample_volume_fraction(x, y - 1)) * 0.5f * inv_dx,
        };
        const float gradient_length = length(gradient);
        if (gradient_length <= 1.0e-6f)
        {
            return {};
        }

        return gradient / gradient_length;
    }

    void apply_viscosity(float step_seconds)
    {
        if (step_seconds <= 0.0f || grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        const float cell_size = grid_.cell_size() > 0.0f ? grid_.cell_size() : 1.0f;
        const float requested_diffusion = solver_settings_.viscosity_coefficient * step_seconds / (cell_size * cell_size);
        const float diffusion = std::clamp(requested_diffusion, 0.0f, 0.24f);
        if (diffusion <= 0.0f)
        {
            return;
        }

        std::vector<float> next_u = grid_.u_values();
        std::vector<float> next_v = grid_.v_values();

        const auto sample_u = [&](int x, int y) noexcept -> float
        {
            if (x < 0 || y < 0 || static_cast<size_type>(x) > grid_.width() || static_cast<size_type>(y) >= grid_.height())
            {
                return 0.0f;
            }
            return grid_.u(static_cast<size_type>(x), static_cast<size_type>(y));
        };

        const auto sample_v = [&](int x, int y) noexcept -> float
        {
            if (x < 0 || y < 0 || static_cast<size_type>(x) >= grid_.width() || static_cast<size_type>(y) > grid_.height())
            {
                return 0.0f;
            }
            return grid_.v(static_cast<size_type>(x), static_cast<size_type>(y));
        };

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x <= grid_.width(); ++x)
            {
                const float center = grid_.u(x, y);
                float neighbor_sum = 0.0f;
                std::size_t neighbor_count = 0;

                if (x > 0)
                {
                    neighbor_sum += sample_u(static_cast<int>(x) - 1, static_cast<int>(y));
                    ++neighbor_count;
                }
                if (x < grid_.width())
                {
                    neighbor_sum += sample_u(static_cast<int>(x) + 1, static_cast<int>(y));
                    ++neighbor_count;
                }
                if (y > 0)
                {
                    neighbor_sum += sample_u(static_cast<int>(x), static_cast<int>(y) - 1);
                    ++neighbor_count;
                }
                if (y + 1 < grid_.height())
                {
                    neighbor_sum += sample_u(static_cast<int>(x), static_cast<int>(y) + 1);
                    ++neighbor_count;
                }

                const float average = neighbor_count > 0 ? neighbor_sum / static_cast<float>(neighbor_count) : center;
                next_u[y * (grid_.width() + 1) + x] = center + (average - center) * diffusion;
            }
        }

        for (size_type y = 0; y <= grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const float center = grid_.v(x, y);
                float neighbor_sum = 0.0f;
                std::size_t neighbor_count = 0;

                if (x > 0)
                {
                    neighbor_sum += sample_v(static_cast<int>(x) - 1, static_cast<int>(y));
                    ++neighbor_count;
                }
                if (x + 1 < grid_.width())
                {
                    neighbor_sum += sample_v(static_cast<int>(x) + 1, static_cast<int>(y));
                    ++neighbor_count;
                }
                if (y > 0)
                {
                    neighbor_sum += sample_v(static_cast<int>(x), static_cast<int>(y) - 1);
                    ++neighbor_count;
                }
                if (y < grid_.height())
                {
                    neighbor_sum += sample_v(static_cast<int>(x), static_cast<int>(y) + 1);
                    ++neighbor_count;
                }

                const float average = neighbor_count > 0 ? neighbor_sum / static_cast<float>(neighbor_count) : center;
                next_v[y * grid_.width() + x] = center + (average - center) * diffusion;
            }
        }

        grid_.u_raw().swap(next_u);
        grid_.v_raw().swap(next_v);
    }

    void apply_surface_tension(float step_seconds)
    {
        if (step_seconds <= 0.0f || grid_.width() == 0 || grid_.height() == 0 || cell_volume_fractions_.size() != grid_.cell_count())
        {
            return;
        }

        const float cell_size = grid_.cell_size() > 0.0f ? grid_.cell_size() : 1.0f;
        const float inv_cell_size = 1.0f / cell_size;
        const float max_velocity_delta = solver_settings_.max_surface_velocity_delta_fraction * cell_size / std::max(step_seconds, 1.0e-6f);
        if (solver_settings_.surface_tension_coefficient <= 0.0f || max_velocity_delta <= 0.0f)
        {
            return;
        }

        std::vector<Vec2> cell_forces(grid_.cell_count(), Vec2{});

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                if (grid_.solid(x, y))
                {
                    continue;
                }

                const int ix = static_cast<int>(x);
                const int iy = static_cast<int>(y);
                const Vec2 normal = volume_fraction_normal(ix, iy);
                if (length_squared(normal) <= 0.0f)
                {
                    continue;
                }

                const Vec2 normal_left = volume_fraction_normal(ix - 1, iy);
                const Vec2 normal_right = volume_fraction_normal(ix + 1, iy);
                const Vec2 normal_up = volume_fraction_normal(ix, iy - 1);
                const Vec2 normal_down = volume_fraction_normal(ix, iy + 1);
                const float curvature = -((normal_right.x - normal_left.x) + (normal_down.y - normal_up.y)) * 0.5f * inv_cell_size;

                Vec2 weighted_sum{};
                float weight_sum = 0.0f;
                for (int offset_y = -1; offset_y <= 1; ++offset_y)
                {
                    for (int offset_x = -1; offset_x <= 1; ++offset_x)
                    {
                        const float weight = sample_volume_fraction(ix + offset_x, iy + offset_y);
                        if (weight <= 0.0f)
                        {
                            continue;
                        }

                        weighted_sum.x += (static_cast<float>(ix + offset_x) + 0.5f) * cell_size * weight;
                        weighted_sum.y += (static_cast<float>(iy + offset_y) + 0.5f) * cell_size * weight;
                        weight_sum += weight;
                    }
                }

                Vec2 cohesion_force{};
                if (weight_sum > 0.0f)
                {
                    const Vec2 centroid = weighted_sum / weight_sum;
                    const Vec2 cell_center{(static_cast<float>(x) + 0.5f) * cell_size, (static_cast<float>(y) + 0.5f) * cell_size};
                    cohesion_force = (centroid - cell_center) * (solver_settings_.surface_tension_coefficient * 0.5f * inv_cell_size);
                }

                cell_forces[grid_.cell_index(x, y)] = (normal * (-solver_settings_.surface_tension_coefficient * curvature)) + cohesion_force;
            }
        }

        std::vector<float> next_u = grid_.u_values();
        std::vector<float> next_v = grid_.v_values();

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 1; x < grid_.width(); ++x)
            {
                if (grid_.solid(x - 1, y) || grid_.solid(x, y))
                {
                    continue;
                }

                const Vec2 left_force = cell_forces[grid_.cell_index(x - 1, y)];
                const Vec2 right_force = cell_forces[grid_.cell_index(x, y)];
                float delta = 0.5f * (left_force.x + right_force.x) * step_seconds;
                delta = clampf(delta, -max_velocity_delta, max_velocity_delta);
                next_u[y * (grid_.width() + 1) + x] = grid_.u(x, y) + delta;
            }
        }

        for (size_type y = 1; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                if (grid_.solid(x, y - 1) || grid_.solid(x, y))
                {
                    continue;
                }

                const Vec2 top_force = cell_forces[grid_.cell_index(x, y - 1)];
                const Vec2 bottom_force = cell_forces[grid_.cell_index(x, y)];
                float delta = 0.5f * (top_force.y + bottom_force.y) * step_seconds;
                delta = clampf(delta, -max_velocity_delta, max_velocity_delta);
                next_v[y * grid_.width() + x] = grid_.v(x, y) + delta;
            }
        }

        grid_.u_raw().swap(next_u);
        grid_.v_raw().swap(next_v);

        if (particles_.size() > 1)
        {
            const Vec2 center = particle_center_of_mass(particles_);
            const float particle_delta_limit = max_velocity_delta * 0.25f;
            const float cohesion_delta = solver_settings_.surface_tension_coefficient * step_seconds * 0.01f;
            for (auto& particle : particles_)
            {
                const Vec2 to_center = center - particle.position;
                const float distance = length(to_center);
                if (distance <= 1.0e-6f)
                {
                    continue;
                }

                const size_type cell_x = static_cast<size_type>(std::floor(particle.position.x / cell_size));
                const size_type cell_y = static_cast<size_type>(std::floor(particle.position.y / cell_size));
                if (!grid_.contains(cell_x, cell_y))
                {
                    continue;
                }

                const int ix = static_cast<int>(cell_x);
                const int iy = static_cast<int>(cell_y);
                const bool exposed_surface = sample_volume_fraction(ix - 1, iy) <= 0.0f
                    || sample_volume_fraction(ix + 1, iy) <= 0.0f
                    || sample_volume_fraction(ix, iy - 1) <= 0.0f
                    || sample_volume_fraction(ix, iy + 1) <= 0.0f;
                if (!exposed_surface)
                {
                    continue;
                }

                const float applied_delta = std::min(particle_delta_limit, cohesion_delta);
                particle.velocity = particle.velocity + (to_center / distance) * applied_delta;
            }
        }
    }

    [[nodiscard]] PressureSolveResult project_pressures(float step_seconds)
    {
        PressureSolveResult result;
        result.visible_cells = visible_fluid_cell_count();
        result.pressure_dt = step_seconds;
        result.rest_density = solver_settings_.rest_density;
        const auto cell_count = grid_.cell_count();
        if (pressure_next_.size() != cell_count)
        {
            pressure_next_.assign(cell_count, 0.0f);
        }

        std::vector<int> cell_to_system(cell_count, -1);
        std::vector<size_type> system_to_cell;
        system_to_cell.reserve(cell_count);

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = safe_cell_index(x, y);
                if (is_active_cell(idx) && !grid_.solid(x, y))
                {
                    cell_to_system[idx] = static_cast<int>(system_to_cell.size());
                    system_to_cell.push_back(idx);
                }
            }
        }

        result.active_cells = system_to_cell.size();
        result.pressure_active_cells = result.active_cells;
        result.active_cell_overreach_ratio = result.visible_cells > 0
            ? static_cast<double>(result.pressure_active_cells) / static_cast<double>(result.visible_cells)
            : 0.0;
        if (system_to_cell.empty())
        {
            for (float& pressure : grid_.pressure_values())
            {
                pressure = 0.0f;
            }
            result.converged = true;
            return result;
        }

        const float cell_size = grid_.cell_size();
        const float inv_cell_size = 1.0f / cell_size;
        const float pressure_scale = solver_settings_.rest_density * cell_size * cell_size / std::max(step_seconds, 1.0e-6f);
        const std::size_t system_size = system_to_cell.size();
        result.max_iterations = std::max(1, solver_settings_.pressure_max_iterations);
        result.target_relative_residual = std::max(0.0f, solver_settings_.pressure_relative_residual_target);
        std::vector<double> pressure(system_size, 0.0);
        std::vector<double> rhs(system_size, 0.0);
        std::vector<double> residual(system_size, 0.0);
        std::vector<double> direction(system_size, 0.0);
        std::vector<double> preconditioned(system_size, 0.0);
        std::vector<double> applied(system_size, 0.0);
        std::vector<double> inverse_diagonal(system_size, 1.0);

        const auto cell_x = [&](size_type cell_index) noexcept -> size_type
        {
            return cell_index % grid_.width();
        };

        const auto cell_y = [&](size_type cell_index) noexcept -> size_type
        {
            return cell_index / grid_.width();
        };

        const auto neighbor_system_index = [&](int x, int y) noexcept -> int
        {
            if (x < 0 || y < 0)
            {
                return -1;
            }
            const size_type sx = static_cast<size_type>(x);
            const size_type sy = static_cast<size_type>(y);
            if (!grid_.contains(sx, sy) || grid_.solid(sx, sy))
            {
                return -1;
            }
            return cell_to_system[safe_cell_index(sx, sy)];
        };

        const auto for_each_neighbor = [&](size_type x, size_type y, const auto& callback)
        {
            callback(static_cast<int>(x) - 1, static_cast<int>(y));
            callback(static_cast<int>(x) + 1, static_cast<int>(y));
            callback(static_cast<int>(x), static_cast<int>(y) - 1);
            callback(static_cast<int>(x), static_cast<int>(y) + 1);
        };

        for (std::size_t row = 0; row < system_size; ++row)
        {
            const size_type idx = system_to_cell[row];
            const size_type x = cell_x(idx);
            const size_type y = cell_y(idx);
            rhs[row] = -static_cast<double>(compute_divergence_for_cell(x, y, inv_cell_size)) * static_cast<double>(pressure_scale);

            int diagonal = 0;
            for_each_neighbor(x, y, [&](int nx, int ny)
            {
                if (nx < 0 || ny < 0)
                {
                    return;
                }
                const size_type sx = static_cast<size_type>(nx);
                const size_type sy = static_cast<size_type>(ny);
                if (!grid_.contains(sx, sy) || grid_.solid(sx, sy))
                {
                    return;
                }
                ++diagonal;
            });
            inverse_diagonal[row] = diagonal > 0 ? 1.0 / static_cast<double>(diagonal) : 1.0;
        }

        const auto apply_operator = [&](const std::vector<double>& input, std::vector<double>& output)
        {
            std::fill(output.begin(), output.end(), 0.0);
            for (std::size_t row = 0; row < system_size; ++row)
            {
                const size_type idx = system_to_cell[row];
                const size_type x = cell_x(idx);
                const size_type y = cell_y(idx);

                double diagonal = 0.0;
                double neighbor_sum = 0.0;
                for_each_neighbor(x, y, [&](int nx, int ny)
                {
                    if (nx < 0 || ny < 0)
                    {
                        return;
                    }
                    const size_type sx = static_cast<size_type>(nx);
                    const size_type sy = static_cast<size_type>(ny);
                    if (!grid_.contains(sx, sy) || grid_.solid(sx, sy))
                    {
                        return;
                    }

                    ++diagonal;
                    const int neighbor_index = neighbor_system_index(nx, ny);
                    if (neighbor_index >= 0)
                    {
                        neighbor_sum += input[static_cast<std::size_t>(neighbor_index)];
                    }
                });

                output[row] = diagonal * input[row] - neighbor_sum;
            }
        };

        const auto dot = [](const std::vector<double>& lhs, const std::vector<double>& rhs_values) noexcept
        {
            double sum = 0.0;
            for (std::size_t i = 0; i < lhs.size(); ++i)
            {
                sum += lhs[i] * rhs_values[i];
            }
            return sum;
        };
        result.rhs_l2 = static_cast<float>(std::sqrt(std::max(0.0, dot(rhs, rhs))));

        apply_operator(pressure, applied);
        for (std::size_t i = 0; i < system_size; ++i)
        {
            residual[i] = rhs[i] - applied[i];
            preconditioned[i] = residual[i] * inverse_diagonal[i];
            direction[i] = preconditioned[i];
        }

        const double initial_residual = std::sqrt(std::max(0.0, dot(residual, residual)));
        result.initial_residual = static_cast<float>(initial_residual);
        result.final_residual = result.initial_residual;
        result.absolute_residual = result.final_residual;
        result.target_absolute_residual = static_cast<float>(initial_residual * static_cast<double>(result.target_relative_residual));
        result.relative_residual = initial_residual > 0.0 ? 1.0f : 0.0f;
        if (initial_residual <= 1.0e-12)
        {
            result.converged = true;
        }
        else
        {
            double residual_dot = dot(residual, preconditioned);
            const int max_iterations = result.max_iterations;
            const double target_relative_residual = static_cast<double>(result.target_relative_residual);

            for (int iteration = 1; iteration <= max_iterations; ++iteration)
            {
                apply_operator(direction, applied);
                const double denominator = dot(direction, applied);
                if (std::abs(denominator) <= 1.0e-20)
                {
                    break;
                }

                const double alpha = residual_dot / denominator;
                for (std::size_t i = 0; i < system_size; ++i)
                {
                    pressure[i] += alpha * direction[i];
                    residual[i] -= alpha * applied[i];
                }

                const double current_residual = std::sqrt(std::max(0.0, dot(residual, residual)));
                result.iterations = iteration;
                result.final_residual = static_cast<float>(current_residual);
                result.absolute_residual = result.final_residual;
                result.relative_residual = static_cast<float>(current_residual / initial_residual);
                if (result.relative_residual <= target_relative_residual)
                {
                    result.converged = true;
                    break;
                }

                for (std::size_t i = 0; i < system_size; ++i)
                {
                    preconditioned[i] = residual[i] * inverse_diagonal[i];
                }

                const double next_residual_dot = dot(residual, preconditioned);
                if (std::abs(residual_dot) <= 1.0e-20)
                {
                    break;
                }

                const double beta = next_residual_dot / residual_dot;
                for (std::size_t i = 0; i < system_size; ++i)
                {
                    direction[i] = preconditioned[i] + beta * direction[i];
                }
                residual_dot = next_residual_dot;
            }
        }

        for (float& value : grid_.pressure_values())
        {
            value = 0.0f;
        }
        result.solution_l2 = static_cast<float>(std::sqrt(std::max(0.0, dot(pressure, pressure))));
        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                const size_type idx = safe_cell_index(x, y);
                const int system_index = cell_to_system[idx];
                grid_.pressure(x, y) = system_index >= 0 ? static_cast<float>(pressure[static_cast<std::size_t>(system_index)]) : 0.0f;
            }
        }
        return result;
    }

    [[nodiscard]] float compute_divergence_for_cell(size_type x, size_type y, float inv_cell_size) const noexcept
    {
        const float left = face_velocity_u(static_cast<int>(x), static_cast<int>(y));
        const float right = face_velocity_u(static_cast<int>(x) + 1, static_cast<int>(y));
        const float top = face_velocity_v(static_cast<int>(x), static_cast<int>(y));
        const float bottom = face_velocity_v(static_cast<int>(x), static_cast<int>(y) + 1);
        return (right - left + bottom - top) * inv_cell_size;
    }

    [[nodiscard]] float face_velocity_u(int x, int y) const noexcept
    {
        if (x < 0 || y < 0 || static_cast<size_type>(y) >= grid_.height() || static_cast<size_type>(x) > grid_.width())
        {
            return 0.0f;
        }

        return grid_.u(static_cast<size_type>(x), static_cast<size_type>(y));
    }

    [[nodiscard]] float face_velocity_v(int x, int y) const noexcept
    {
        if (x < 0 || y < 0 || static_cast<size_type>(x) >= grid_.width() || static_cast<size_type>(y) > grid_.height())
        {
            return 0.0f;
        }

        return grid_.v(static_cast<size_type>(x), static_cast<size_type>(y));
    }

    void apply_pressure_gradient(float step_seconds)
    {
        const float inv = 1.0f / grid_.cell_size();
        const float gradient_scale = step_seconds / std::max(solver_settings_.rest_density, 1.0e-6f);

        for (size_type y = 0; y < grid_.height(); ++y)
        {
            for (size_type x = 0; x <= grid_.width(); ++x)
            {
                const float left_pressure = (x > 0 && !grid_.solid(x - 1, y)) ? grid_.pressure(x - 1, y) : 0.0f;
                const float right_pressure = (x < grid_.width() && !grid_.solid(std::min(x, grid_.width() - 1), y)) ? grid_.pressure(std::min(x, grid_.width() - 1), y) : 0.0f;

                if (x == 0 || x == grid_.width())
                {
                    grid_.u(x, y) = 0.0f;
                    continue;
                }

                const bool left_active = is_active_cell(safe_cell_index(x - 1, y));
                const bool right_active = is_active_cell(safe_cell_index(x, y));
                const float p_left = left_active && !grid_.solid(x - 1, y) ? left_pressure : 0.0f;
                const float p_right = right_active && !grid_.solid(x, y) ? right_pressure : 0.0f;
                grid_.u(x, y) -= (p_right - p_left) * inv * gradient_scale;
            }
        }

        for (size_type y = 0; y <= grid_.height(); ++y)
        {
            for (size_type x = 0; x < grid_.width(); ++x)
            {
                if (y == 0 || y == grid_.height())
                {
                    grid_.v(x, y) = 0.0f;
                    continue;
                }

                const bool top_active = is_active_cell(safe_cell_index(x, y - 1));
                const bool bottom_active = is_active_cell(safe_cell_index(x, y));
                const float p_top = top_active && !grid_.solid(x, y - 1) ? grid_.pressure(x, y - 1) : 0.0f;
                const float p_bottom = bottom_active && !grid_.solid(x, y) ? grid_.pressure(x, y) : 0.0f;
                grid_.v(x, y) -= (p_bottom - p_top) * inv * gradient_scale;
            }
        }
    }

    void transfer_grid_to_particles(float dt)
    {
        const float flip_blend = solver_settings_.flip_blend;
        const float velocity_retention = solver_settings_.velocity_retention;
        const float viscosity_factor = dt > 0.0f && solver_settings_.viscosity_coefficient > 0.0f
            ? std::clamp(
                solver_settings_.viscosity_coefficient * dt / (grid_.cell_size() * grid_.cell_size()),
                0.0f,
                0.24f)
            : 0.0f;
        const float particle_retention = clampf(1.0f - viscosity_factor * 0.5f, 0.0f, 1.0f);

        for (auto& particle : particles_)
        {
            const Vec2 pic_velocity = sample_velocity(particle.position, grid_.u_values(), grid_.v_values());
            const Vec2 previous_velocity = sample_velocity(particle.position, u_previous_, v_previous_);
            const Vec2 transferred_velocity = pic_flip_blend(particle.velocity, pic_velocity, previous_velocity, pic_velocity, flip_blend);
            particle.velocity.x = lerp(transferred_velocity.x, particle.velocity.x, velocity_retention);
            particle.velocity.y = lerp(transferred_velocity.y, particle.velocity.y, velocity_retention);
            if (viscosity_factor > 0.0f)
            {
                particle.velocity.x *= particle_retention;
                particle.velocity.y *= particle_retention;
            }

            update_particle_affine_velocity(particle);
        }
    }

    void apply_local_density_correction(float step_seconds)
    {
        if (particles_.size() < 2 || step_seconds <= 0.0f)
        {
            return;
        }

        std::vector<Vec2> previous_positions;
        previous_positions.reserve(particles_.size());
        for (const auto& particle : particles_)
        {
            previous_positions.push_back(particle.position);
        }

        DensityCorrectionSettings correction_settings;
        correction_settings.iterations = solver_settings_.density_correction_iterations;
        correction_settings.max_correction = grid_.cell_size() * solver_settings_.max_density_correction_fraction;
        const DensityCorrectionResult correction = physics_sim::apply_density_constraint_correction(
            particles_,
            density_settings(),
            correction_settings);
        (void)correction;

        const float max_velocity_delta = correction_settings.max_correction / std::max(step_seconds, 1.0e-6f);
        if (max_velocity_delta <= 0.0f)
        {
            return;
        }

        for (std::size_t index = 0; index < particles_.size() && index < previous_positions.size(); ++index)
        {
            const Vec2 delta = particles_[index].position - previous_positions[index];
            Vec2 correction_velocity = delta / step_seconds;
            const float correction_length = length(correction_velocity);
            if (correction_length > max_velocity_delta)
            {
                correction_velocity = correction_velocity * (max_velocity_delta / correction_length);
            }

            particles_[index].velocity = particles_[index].velocity + correction_velocity;
        }

        resolve_particles_out_of_solids();
    }

    void update_particle_affine_velocity(FluidParticle& particle) const noexcept
    {
        const float radius = std::max(grid_.cell_size() * 0.5f, 0.0001f);
        const float inv_span = 1.0f / (2.0f * radius);
        const Vec2 x_plus = sample_velocity(particle.position + Vec2{radius, 0.0f}, grid_.u_values(), grid_.v_values());
        const Vec2 x_minus = sample_velocity(particle.position - Vec2{radius, 0.0f}, grid_.u_values(), grid_.v_values());
        const Vec2 y_plus = sample_velocity(particle.position + Vec2{0.0f, radius}, grid_.u_values(), grid_.v_values());
        const Vec2 y_minus = sample_velocity(particle.position - Vec2{0.0f, radius}, grid_.u_values(), grid_.v_values());

        const float affine_ratio = solver_settings_.apic_affine_ratio;
        particle.affine_velocity = Mat2{
            (x_plus.x - x_minus.x) * inv_span * affine_ratio,
            (y_plus.x - y_minus.x) * inv_span * affine_ratio,
            (x_plus.y - x_minus.y) * inv_span * affine_ratio,
            (y_plus.y - y_minus.y) * inv_span * affine_ratio,
        };
    }

    void advect_particles(float dt)
    {
        const float epsilon = grid_.cell_size() * 0.001f;

        for (auto& particle : particles_)
        {
            const Vec2 previous_position = particle.position;
            Vec2 desired_position{
                previous_position.x + particle.velocity.x * dt,
                previous_position.y + particle.velocity.y * dt,
            };
            const auto damp_tangent_after_x_contact = [&]() noexcept
            {
                if (solver_settings_.wall_material == WallBoundaryMaterial::Damped)
                {
                    particle.velocity.y *= solver_settings_.wall_tangential_velocity_retention;
                }
            };
            const auto damp_tangent_after_y_contact = [&]() noexcept
            {
                if (solver_settings_.wall_material == WallBoundaryMaterial::Damped)
                {
                    particle.velocity.x *= solver_settings_.wall_tangential_velocity_retention;
                }
            };

            if (is_out_of_domain(desired_position))
            {
                particle.position = desired_position;
                continue;
            }

            desired_position.x = clampf(
                desired_position.x,
                epsilon,
                std::max(epsilon, domain_width() - epsilon));
            desired_position.y = clampf(
                desired_position.y,
                epsilon,
                std::max(epsilon, domain_height() - epsilon));

            auto solid_cell_at = [&](const Vec2& position, size_type& cell_x, size_type& cell_y) noexcept
            {
                cell_x = static_cast<size_type>(std::floor(position.x / grid_.cell_size()));
                cell_y = static_cast<size_type>(std::floor(position.y / grid_.cell_size()));
                return grid_.contains(cell_x, cell_y) && grid_.solid(cell_x, cell_y);
            };

            const Vec2 displacement = desired_position - previous_position;
            const float travel = length(displacement);
            if (travel > grid_.cell_size())
            {
                const int samples = std::clamp(static_cast<int>(std::ceil(travel / std::max(grid_.cell_size() * 0.5f, 0.0001f))), 2, 64);
                for (int sample = 1; sample <= samples; ++sample)
                {
                    const float t = static_cast<float>(sample) / static_cast<float>(samples);
                    const Vec2 candidate = previous_position + displacement * t;
                    size_type swept_x = 0;
                    size_type swept_y = 0;
                    if (solid_cell_at(candidate, swept_x, swept_y))
                    {
                        const float previous_t = static_cast<float>(sample - 1) / static_cast<float>(samples);
                        desired_position = previous_position + displacement * previous_t;
                        if (std::abs(displacement.x) >= std::abs(displacement.y))
                        {
                            particle.velocity.x = 0.0f;
                            damp_tangent_after_x_contact();
                        }
                        else
                        {
                            particle.velocity.y = 0.0f;
                            damp_tangent_after_y_contact();
                        }
                        break;
                    }
                }
            }

            Vec2 resolved_position{
                clampf(previous_position.x, epsilon, std::max(epsilon, domain_width() - epsilon)),
                clampf(previous_position.y, epsilon, std::max(epsilon, domain_height() - epsilon)),
            };

            size_type hit_x = 0;
            size_type hit_y = 0;
            const Vec2 x_candidate{desired_position.x, resolved_position.y};
            if (solid_cell_at(x_candidate, hit_x, hit_y))
            {
                if (particle.velocity.x > 0.0f)
                {
                    resolved_position.x = static_cast<float>(hit_x) * grid_.cell_size() - epsilon;
                }
                else if (particle.velocity.x < 0.0f)
                {
                    resolved_position.x = (static_cast<float>(hit_x) + 1.0f) * grid_.cell_size() + epsilon;
                }
                particle.velocity.x = 0.0f;
                damp_tangent_after_x_contact();
            }
            else
            {
                resolved_position.x = desired_position.x;
            }

            const Vec2 y_candidate{resolved_position.x, desired_position.y};
            if (solid_cell_at(y_candidate, hit_x, hit_y))
            {
                if (particle.velocity.y > 0.0f)
                {
                    resolved_position.y = static_cast<float>(hit_y) * grid_.cell_size() - epsilon;
                }
                else if (particle.velocity.y < 0.0f)
                {
                    resolved_position.y = (static_cast<float>(hit_y) + 1.0f) * grid_.cell_size() + epsilon;
                }
                particle.velocity.y = 0.0f;
                damp_tangent_after_y_contact();
            }
            else
            {
                resolved_position.y = desired_position.y;
            }

            if (solid_cell_at(resolved_position, hit_x, hit_y))
            {
                particle.position = resolved_position;
                resolve_particle_out_of_solids(particle);
                continue;
            }

            particle.position = resolved_position;
        }
    }

    void resolve_particles_out_of_solids() noexcept
    {
        for (auto& particle : particles_)
        {
            resolve_particle_out_of_solids(particle);
        }
    }

    void resolve_particle_out_of_solids(FluidParticle& particle) const noexcept
    {
        if (grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        const float cell_size = grid_.cell_size();
        const float epsilon = cell_size * 0.001f;
        particle.position.x = clampf(particle.position.x, epsilon, std::max(epsilon, domain_width() - epsilon));
        particle.position.y = clampf(particle.position.y, epsilon, std::max(epsilon, domain_height() - epsilon));

        const auto cell_for_position = [&](const Vec2& position, size_type& cell_x, size_type& cell_y) noexcept -> bool
        {
            if (position.x < 0.0f || position.y < 0.0f)
            {
                return false;
            }

            cell_x = static_cast<size_type>(std::floor(position.x / cell_size));
            cell_y = static_cast<size_type>(std::floor(position.y / cell_size));
            return grid_.contains(cell_x, cell_y);
        };

        size_type cell_x = 0;
        size_type cell_y = 0;
        if (!cell_for_position(particle.position, cell_x, cell_y) || !grid_.solid(cell_x, cell_y))
        {
            return;
        }

        const int origin_x = static_cast<int>(cell_x);
        const int origin_y = static_cast<int>(cell_y);
        const int search_radius = 4;
        bool have_candidate = false;
        Vec2 candidate_position{};
        Vec2 candidate_delta{};
        float candidate_distance_squared = std::numeric_limits<float>::max();

        for (int offset_y = -search_radius; offset_y <= search_radius; ++offset_y)
        {
            const int neighbor_y = origin_y + offset_y;
            if (neighbor_y < 0 || static_cast<size_type>(neighbor_y) >= grid_.height())
            {
                continue;
            }

            for (int offset_x = -search_radius; offset_x <= search_radius; ++offset_x)
            {
                const int neighbor_x = origin_x + offset_x;
                if (neighbor_x < 0 || static_cast<size_type>(neighbor_x) >= grid_.width())
                {
                    continue;
                }

                const size_type candidate_cell_x = static_cast<size_type>(neighbor_x);
                const size_type candidate_cell_y = static_cast<size_type>(neighbor_y);
                if (grid_.solid(candidate_cell_x, candidate_cell_y))
                {
                    continue;
                }

                const float min_x = static_cast<float>(candidate_cell_x) * cell_size + epsilon;
                const float min_y = static_cast<float>(candidate_cell_y) * cell_size + epsilon;
                const float max_x = static_cast<float>(candidate_cell_x + 1) * cell_size - epsilon;
                const float max_y = static_cast<float>(candidate_cell_y + 1) * cell_size - epsilon;
                const Vec2 projected_position{
                    clampf(particle.position.x, min_x, max_x),
                    clampf(particle.position.y, min_y, max_y),
                };
                const Vec2 delta = projected_position - particle.position;
                const float distance_squared = length_squared(delta);
                if (!have_candidate || distance_squared < candidate_distance_squared)
                {
                    have_candidate = true;
                    candidate_position = projected_position;
                    candidate_delta = delta;
                    candidate_distance_squared = distance_squared;
                }
            }
        }

        if (!have_candidate)
        {
            particle.velocity = {};
            return;
        }

        particle.position = candidate_position;
        if (std::abs(candidate_delta.x) >= std::abs(candidate_delta.y))
        {
            particle.velocity.x = 0.0f;
        }
        else
        {
            particle.velocity.y = 0.0f;
        }
    }

    void resample_particles()
    {
        if (!solver_settings_.resampling.enabled || particles_.empty() || grid_.width() == 0 || grid_.height() == 0)
        {
            return;
        }

        ensure_particle_transfer_properties();
        refresh_cell_classification();

        const float cell_size = grid_.cell_size();
        const float epsilon = cell_size * 0.001f;
        const std::size_t cell_count = grid_.cell_count();
        std::vector<std::vector<size_type>> buckets(cell_count);
        buckets.shrink_to_fit();

        const auto cell_for_position = [&](const Vec2& position, size_type& cell_x, size_type& cell_y) noexcept -> bool
        {
            if (position.x < 0.0f || position.y < 0.0f)
            {
                return false;
            }

            cell_x = static_cast<size_type>(std::floor(position.x / cell_size));
            cell_y = static_cast<size_type>(std::floor(position.y / cell_size));
            if (!grid_.contains(cell_x, cell_y) || grid_.solid(cell_x, cell_y))
            {
                return false;
            }

            return true;
        };

        for (size_type index = 0; index < particles_.size(); ++index)
        {
            size_type cell_x = 0;
            size_type cell_y = 0;
            if (!cell_for_position(particles_[index].position, cell_x, cell_y))
            {
                continue;
            }

            buckets[grid_.cell_index(cell_x, cell_y)].push_back(index);
        }

        std::vector<std::uint8_t> removed(particles_.size(), std::uint8_t{0});
        std::size_t operations = 0;
        const std::size_t max_operations = solver_settings_.resampling.max_resampling_operations_per_step;
        const std::size_t min_particles = solver_settings_.resampling.min_particles_per_fluid_cell;
        const std::size_t target_particles = solver_settings_.resampling.target_particles_per_fluid_cell;
        const float min_split_mass = solver_settings_.resampling.min_split_particle_mass;
        const float split_offset = solver_settings_.resampling.split_offset_fraction * cell_size;

        const auto prune_removed = [&](std::vector<size_type>& bucket)
        {
            bucket.erase(
                std::remove_if(
                    bucket.begin(),
                    bucket.end(),
                    [&](size_type particle_index) { return removed[particle_index] != 0; }),
                bucket.end());
        };

        const auto particle_volume_value = [&](const FluidParticle& particle) noexcept -> float
        {
            return particle.volume > 0.0f ? particle.volume : density_settings().particle_volume;
        };

        const auto find_best_pair = [&](const std::vector<size_type>& bucket) noexcept -> std::pair<size_type, size_type>
        {
            bool have_pair = false;
            double best_distance = 0.0;
            size_type best_first = 0;
            size_type best_second = 0;

            for (std::size_t i = 0; i < bucket.size(); ++i)
            {
                const size_type first_index = bucket[i];
                if (removed[first_index] != 0)
                {
                    continue;
                }

                for (std::size_t j = i + 1; j < bucket.size(); ++j)
                {
                    const size_type second_index = bucket[j];
                    if (removed[second_index] != 0)
                    {
                        continue;
                    }

                    const double distance = static_cast<double>(length_squared(particles_[first_index].position - particles_[second_index].position));
                    const size_type ordered_first = std::min(first_index, second_index);
                    const size_type ordered_second = std::max(first_index, second_index);
                    if (!have_pair
                        || distance < best_distance - 1.0e-12
                        || (std::abs(distance - best_distance) <= 1.0e-12
                            && (ordered_first < best_first || (ordered_first == best_first && ordered_second < best_second))))
                    {
                        have_pair = true;
                        best_distance = distance;
                        best_first = ordered_first;
                        best_second = ordered_second;
                    }
                }
            }

            return have_pair ? std::pair<size_type, size_type>{best_first, best_second} : std::pair<size_type, size_type>{particles_.size(), particles_.size()};
        };

        const auto merge_particles = [&](size_type first_index, size_type second_index) noexcept
        {
            const FluidParticle& first = particles_[first_index];
            const FluidParticle& second = particles_[second_index];
            const double first_mass = static_cast<double>(first.mass > 0.0f ? first.mass : density_settings().rest_density * particle_volume_value(first));
            const double second_mass = static_cast<double>(second.mass > 0.0f ? second.mass : density_settings().rest_density * particle_volume_value(second));
            const double total_mass = first_mass + second_mass;
            const double first_volume = static_cast<double>(particle_volume_value(first));
            const double second_volume = static_cast<double>(particle_volume_value(second));
            const double total_volume = first_volume + second_volume;

            FluidParticle merged = first;
            if (total_mass > 0.0)
            {
                merged.position = Vec2{
                    static_cast<float>((static_cast<double>(first.position.x) * first_mass + static_cast<double>(second.position.x) * second_mass) / total_mass),
                    static_cast<float>((static_cast<double>(first.position.y) * first_mass + static_cast<double>(second.position.y) * second_mass) / total_mass),
                };
                merged.velocity = Vec2{
                    static_cast<float>((static_cast<double>(first.velocity.x) * first_mass + static_cast<double>(second.velocity.x) * second_mass) / total_mass),
                    static_cast<float>((static_cast<double>(first.velocity.y) * first_mass + static_cast<double>(second.velocity.y) * second_mass) / total_mass),
                };
            }

            merged.mass = static_cast<float>(total_mass);
            merged.volume = static_cast<float>(total_volume);
            merged.density = 0.0f;
            merged.neighbor_count = 0;
            merged.affine_velocity = {};
            return merged;
        };

        const auto split_particle = [&](size_type cell_x, size_type cell_y, size_type particle_index, std::vector<size_type>& bucket)
        {
            if (operations >= max_operations)
            {
                return false;
            }

            const FluidParticle parent = particles_[particle_index];
            const float parent_mass = parent.mass > 0.0f ? parent.mass : density_settings().rest_density * particle_volume_value(parent);
            const float parent_volume = particle_volume_value(parent);
            if (parent_mass < 2.0f * min_split_mass)
            {
                return false;
            }

            Vec2 tangent{};
            const Vec2 normal = volume_fraction_normal(static_cast<int>(cell_x), static_cast<int>(cell_y));
            if (length_squared(normal) > 1.0e-6f)
            {
                tangent = Vec2{-normal.y, normal.x};
            }
            else
            {
                tangent = Vec2{1.0f, 0.0f};
            }

            const float tangent_length = length(tangent);
            if (tangent_length <= 1.0e-6f)
            {
                tangent = Vec2{1.0f, 0.0f};
            }
            else
            {
                tangent = tangent / tangent_length;
            }

            Vec2 child_a_position = parent.position + tangent * split_offset;
            Vec2 child_b_position = parent.position - tangent * split_offset;
            child_a_position.x = clampf(child_a_position.x, epsilon, std::max(epsilon, domain_width() - epsilon));
            child_a_position.y = clampf(child_a_position.y, epsilon, std::max(epsilon, domain_height() - epsilon));
            child_b_position.x = clampf(child_b_position.x, epsilon, std::max(epsilon, domain_width() - epsilon));
            child_b_position.y = clampf(child_b_position.y, epsilon, std::max(epsilon, domain_height() - epsilon));

            size_type child_a_cell_x = 0;
            size_type child_a_cell_y = 0;
            size_type child_b_cell_x = 0;
            size_type child_b_cell_y = 0;
            if (!cell_for_position(child_a_position, child_a_cell_x, child_a_cell_y)
                || !cell_for_position(child_b_position, child_b_cell_x, child_b_cell_y)
                || child_a_cell_x != cell_x
                || child_a_cell_y != cell_y
                || child_b_cell_x != cell_x
                || child_b_cell_y != cell_y)
            {
                return false;
            }

            FluidParticle child_a = parent;
            FluidParticle child_b = parent;
            child_a.position = child_a_position;
            child_b.position = child_b_position;
            child_a.mass = parent_mass * 0.5f;
            child_b.mass = parent_mass * 0.5f;
            child_a.volume = parent_volume * 0.5f;
            child_b.volume = parent_volume * 0.5f;
            child_a.density = 0.0f;
            child_b.density = 0.0f;
            child_a.neighbor_count = 0;
            child_b.neighbor_count = 0;
            child_a.affine_velocity = parent.affine_velocity;
            child_b.affine_velocity = parent.affine_velocity;

            particles_[particle_index] = child_a;
            particles_.push_back(child_b);
            removed.push_back(std::uint8_t{0});
            bucket.push_back(particles_.size() - 1);
            ++operations;
            return true;
        };

        for (size_type y = 0; y < grid_.height() && operations < max_operations; ++y)
        {
            for (size_type x = 0; x < grid_.width() && operations < max_operations; ++x)
            {
                if (grid_.solid(x, y))
                {
                    continue;
                }

                const std::size_t cell_index = grid_.cell_index(x, y);
                auto& bucket = buckets[cell_index];
                prune_removed(bucket);

                while (bucket.size() > target_particles && operations < max_operations)
                {
                    const auto pair = find_best_pair(bucket);
                    if (pair.first >= particles_.size() || pair.second >= particles_.size())
                    {
                        break;
                    }

                    particles_[pair.first] = merge_particles(pair.first, pair.second);
                    removed[pair.second] = std::uint8_t{1};
                    bucket.erase(std::remove(bucket.begin(), bucket.end(), pair.second), bucket.end());
                    ++operations;
                }

                prune_removed(bucket);
                const float fluid_fraction = cell_volume_fractions_.size() == cell_count ? cell_volume_fractions_[cell_index] : 0.0f;
                while (bucket.size() < min_particles && fluid_fraction >= 0.5f && operations < max_operations)
                {
                    size_type best_candidate = cell_count;
                    float best_mass = -1.0f;

                    for (int offset_y = -1; offset_y <= 1; ++offset_y)
                    {
                        const int neighbor_y = static_cast<int>(y) + offset_y;
                        if (neighbor_y < 0 || static_cast<size_type>(neighbor_y) >= grid_.height())
                        {
                            continue;
                        }

                        for (int offset_x = -1; offset_x <= 1; ++offset_x)
                        {
                            const int neighbor_x = static_cast<int>(x) + offset_x;
                            if (neighbor_x < 0 || static_cast<size_type>(neighbor_x) >= grid_.width())
                            {
                                continue;
                            }

                            const size_type neighbor_cell_x = static_cast<size_type>(neighbor_x);
                            const size_type neighbor_cell_y = static_cast<size_type>(neighbor_y);
                            if (grid_.solid(neighbor_cell_x, neighbor_cell_y))
                            {
                                continue;
                            }

                            const std::size_t neighbor_index = grid_.cell_index(neighbor_cell_x, neighbor_cell_y);
                            for (const size_type particle_index : buckets[neighbor_index])
                            {
                                if (removed[particle_index] != 0)
                                {
                                    continue;
                                }

                                const float mass = particles_[particle_index].mass > 0.0f ? particles_[particle_index].mass : density_settings().rest_density * particle_volume_value(particles_[particle_index]);
                                if (mass > best_mass || (std::abs(mass - best_mass) <= 1.0e-6f && particle_index < best_candidate))
                                {
                                    best_mass = mass;
                                    best_candidate = particle_index;
                                }
                            }
                        }
                    }

                    if (best_candidate >= particles_.size())
                    {
                        break;
                    }

                    if (!split_particle(x, y, best_candidate, bucket))
                    {
                        break;
                    }
                }
            }
        }

        std::vector<FluidParticle> resampled_particles;
        resampled_particles.reserve(particles_.size());
        for (size_type index = 0; index < particles_.size(); ++index)
        {
            if (index < removed.size() && removed[index] != 0)
            {
                continue;
            }

            resampled_particles.push_back(particles_[index]);
        }

        particles_.swap(resampled_particles);
        refresh_cell_classification();
    }

    [[nodiscard]] std::vector<float> collect_u() const
    {
        return grid_.u_values();
    }

    [[nodiscard]] std::vector<float> collect_v() const
    {
        return grid_.v_values();
    }

    MacGrid2D grid_{};
    std::vector<FluidParticle> particles_{};
    std::vector<WaterEmitter> emitters_{};
    std::vector<WaterGate> gates_{};
    std::vector<WaterSensor> sensors_{};
    std::vector<WaterDrain> drains_{};
    std::vector<WaterPump> pumps_{};
    std::vector<WaterValve> valves_{};
    std::vector<std::uint8_t> fluid_cells_{};
    std::vector<FluidCellState> cell_states_{};
    std::vector<float> cell_volume_fractions_{};
    std::vector<float> cell_densities_{};
    std::vector<float> u_weights_{};
    std::vector<float> v_weights_{};
    std::vector<float> u_previous_{};
    std::vector<float> v_previous_{};
    std::vector<float> pressure_next_{};
    WaterSimulationMetrics metrics_{};
    FluidSolverSettings solver_settings_ = solver_settings_for_profile(FluidSolverProfile::Balanced);
    std::uint64_t total_emitted_ = 0;
    double total_emitted_mass_ = 0.0;
    std::uint64_t total_removed_ = 0;
    double total_removed_mass_ = 0.0;
    std::uint64_t total_outflow_ = 0;
    double total_outflow_mass_ = 0.0;
    std::uint64_t simulation_tick_ = 0;

    [[nodiscard]] static FluidSolverSettings sanitize_solver_settings(FluidSolverSettings settings) noexcept
    {
        settings.pressure_max_iterations = std::max(1, settings.pressure_max_iterations);
        settings.pressure_relative_residual_target = std::max(0.0f, settings.pressure_relative_residual_target);
        settings.rest_density = std::max(0.0001f, settings.rest_density);
        settings.particles_per_full_cell = std::max<std::size_t>(1, settings.particles_per_full_cell);
        settings.density_kernel_radius_cells = std::max(0.25f, settings.density_kernel_radius_cells);
        settings.density_correction_iterations = std::max(0, settings.density_correction_iterations);
        settings.max_density_correction_fraction = std::max(0.0f, settings.max_density_correction_fraction);
        settings.flip_blend = std::clamp(settings.flip_blend, 0.0f, 1.0f);
        settings.velocity_retention = std::clamp(settings.velocity_retention, 0.0f, 1.0f);
        settings.apic_affine_ratio = std::clamp(settings.apic_affine_ratio, 0.0f, 1.0f);
        settings.wall_tangential_velocity_retention = std::clamp(settings.wall_tangential_velocity_retention, 0.0f, 1.0f);
        settings.viscosity_coefficient = std::max(0.0f, settings.viscosity_coefficient);
        settings.surface_tension_coefficient = std::max(0.0f, settings.surface_tension_coefficient);
        settings.max_surface_velocity_delta_fraction = std::max(0.0f, settings.max_surface_velocity_delta_fraction);
        settings.resampling.min_particles_per_fluid_cell = std::max<std::size_t>(1, settings.resampling.min_particles_per_fluid_cell);
        settings.resampling.target_particles_per_fluid_cell = std::max(settings.resampling.min_particles_per_fluid_cell, settings.resampling.target_particles_per_fluid_cell);
        settings.resampling.max_particles_per_fluid_cell = std::max(settings.resampling.target_particles_per_fluid_cell, settings.resampling.max_particles_per_fluid_cell);
        settings.resampling.max_resampling_operations_per_step = std::max<std::size_t>(1, settings.resampling.max_resampling_operations_per_step);
        settings.resampling.split_offset_fraction = std::clamp(settings.resampling.split_offset_fraction, 0.0f, 0.5f);
        settings.resampling.min_split_particle_mass = std::max(0.0f, settings.resampling.min_split_particle_mass);
        settings.density_metrics_interval_ticks = std::max<std::uint64_t>(1, settings.density_metrics_interval_ticks);
        return settings;
    }

    [[nodiscard]] double particle_mass_for_metrics(const FluidParticle& particle) const noexcept
    {
        const FluidDensitySettings settings = density_settings();
        const float particle_volume = particle.volume > 0.0f ? particle.volume : settings.particle_volume;
        const float particle_mass = particle.mass > 0.0f ? particle.mass : settings.rest_density * particle_volume;
        return static_cast<double>(particle_mass);
    }
};
} // namespace physics_sim
