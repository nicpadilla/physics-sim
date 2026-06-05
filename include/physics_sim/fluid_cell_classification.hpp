#pragma once

#include <physics_sim/fluid_density.hpp>
#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/mac_grid.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace physics_sim
{
enum class FluidCellState
{
    Air,
    Fluid,
    Solid,
};

struct FluidCellClassification
{
    std::size_t width = 0;
    std::size_t height = 0;
    std::vector<FluidCellState> states{};
    std::vector<float> volume_fractions{};
    std::vector<float> densities{};
    std::size_t fluid_cell_count = 0;
    std::size_t air_cell_count = 0;
    std::size_t solid_cell_count = 0;

    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y) const noexcept
    {
        return y * width + x;
    }

    [[nodiscard]] FluidCellState state(std::size_t x, std::size_t y) const noexcept
    {
        return states[index(x, y)];
    }

    [[nodiscard]] float volume_fraction(std::size_t x, std::size_t y) const noexcept
    {
        return volume_fractions[index(x, y)];
    }

    [[nodiscard]] float density(std::size_t x, std::size_t y) const noexcept
    {
        return densities[index(x, y)];
    }
};

[[nodiscard]] inline FluidCellClassification classify_fluid_cells(
    const MacGrid2D& grid,
    const std::vector<FluidParticle>& particles,
    FluidDensitySettings settings) noexcept
{
    FluidCellClassification classification;
    classification.width = grid.width();
    classification.height = grid.height();

    const std::size_t cell_count = grid.cell_count();
    classification.states.assign(cell_count, FluidCellState::Air);
    classification.volume_fractions.assign(cell_count, 0.0f);
    classification.densities.assign(cell_count, 0.0f);

    if (grid.width() == 0 || grid.height() == 0)
    {
        return classification;
    }

    settings.rest_density = settings.rest_density > 0.0f ? settings.rest_density : 1.0f;
    settings.particle_volume = settings.particle_volume > 0.0f ? settings.particle_volume : 1.0f;

    std::vector<double> volume_sums(cell_count, 0.0);
    std::vector<double> mass_sums(cell_count, 0.0);
    const double cell_area = static_cast<double>(grid.cell_size()) * static_cast<double>(grid.cell_size());
    const double epsilon = std::max(cell_area, 1.0) * 1.0e-9;

    for (const auto& particle : particles)
    {
        if (particle.position.x < 0.0f || particle.position.y < 0.0f)
        {
            continue;
        }

        const auto cell_x = static_cast<std::size_t>(std::floor(particle.position.x / grid.cell_size()));
        const auto cell_y = static_cast<std::size_t>(std::floor(particle.position.y / grid.cell_size()));
        if (!grid.contains(cell_x, cell_y))
        {
            continue;
        }

        const std::size_t idx = grid.cell_index(cell_x, cell_y);
        if (grid.solid(cell_x, cell_y))
        {
            continue;
        }

        const double volume = particle.volume > 0.0f ? static_cast<double>(particle.volume) : static_cast<double>(settings.particle_volume);
        const double mass = particle.mass > 0.0f ? static_cast<double>(particle.mass) : static_cast<double>(settings.rest_density) * volume;
        volume_sums[idx] += volume;
        mass_sums[idx] += mass;
    }

    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            const std::size_t idx = grid.cell_index(x, y);
            if (grid.solid(x, y))
            {
                classification.states[idx] = FluidCellState::Solid;
                ++classification.solid_cell_count;
                continue;
            }

            const double volume_fraction = cell_area > 0.0 ? std::clamp(volume_sums[idx] / cell_area, 0.0, 1.0) : 0.0;
            classification.volume_fractions[idx] = static_cast<float>(volume_fraction);
            if (volume_fraction > 0.0)
            {
                classification.states[idx] = FluidCellState::Fluid;
                classification.densities[idx] = static_cast<float>(mass_sums[idx] / std::max(volume_fraction * cell_area, epsilon));
                ++classification.fluid_cell_count;
            }
            else
            {
                classification.states[idx] = FluidCellState::Air;
                classification.densities[idx] = 0.0f;
                ++classification.air_cell_count;
            }
        }
    }

    return classification;
}
} // namespace physics_sim
