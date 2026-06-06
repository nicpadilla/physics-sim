#pragma once

#include <physics_sim/fluid_particle.hpp>
#include <physics_sim/math.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace physics_sim
{
struct FluidDensitySettings
{
    float rest_density = 1.0f;
    float particle_volume = 1.0f;
    float kernel_radius = 1.0f;
};

struct FluidDensityMetrics
{
    std::size_t particle_count = 0;
    double min_density = 0.0;
    double max_density = 0.0;
    double average_density = 0.0;
    double average_density_error = 0.0;
    double max_density_error = 0.0;
    double average_neighbor_count = 0.0;
    std::size_t max_neighbor_count = 0;
};

struct DensityCorrectionSettings
{
    int iterations = 1;
    float max_correction = 0.2f;
    float epsilon = 1.0e-5f;
};

struct DensityCorrectionResult
{
    double total_mass_before = 0.0;
    double total_mass_after = 0.0;
    Vec2 center_of_mass_before{};
    Vec2 center_of_mass_after{};
    double max_density_error_before = 0.0;
    double max_density_error_after = 0.0;
    float max_applied_correction = 0.0f;
};

[[nodiscard]] inline float cubic_spline_kernel_2d(float distance, float kernel_radius) noexcept
{
    if (kernel_radius <= 0.0f || distance < 0.0f)
    {
        return 0.0f;
    }

    const float q = distance / kernel_radius;
    if (q >= 2.0f)
    {
        return 0.0f;
    }

    constexpr float pi = 3.14159265358979323846f;
    const float scale = 10.0f / (7.0f * pi * kernel_radius * kernel_radius);
    if (q < 1.0f)
    {
        return scale * (1.0f - 1.5f * q * q + 0.75f * q * q * q);
    }

    const float two_minus_q = 2.0f - q;
    return scale * 0.25f * two_minus_q * two_minus_q * two_minus_q;
}

[[nodiscard]] inline Vec2 cubic_spline_kernel_gradient_2d(Vec2 offset, float kernel_radius) noexcept
{
    const float distance = length(offset);
    if (kernel_radius <= 0.0f || distance <= 0.0f)
    {
        return {};
    }

    const float q = distance / kernel_radius;
    if (q >= 2.0f)
    {
        return {};
    }

    constexpr float pi = 3.14159265358979323846f;
    const float scale = 10.0f / (7.0f * pi * kernel_radius * kernel_radius);
    float derivative = 0.0f;
    if (q < 1.0f)
    {
        derivative = scale * (-3.0f * q + 2.25f * q * q) / kernel_radius;
    }
    else
    {
        const float two_minus_q = 2.0f - q;
        derivative = scale * (-0.75f * two_minus_q * two_minus_q) / kernel_radius;
    }

    return offset * (derivative / distance);
}

[[nodiscard]] inline double total_particle_mass(const std::vector<FluidParticle>& particles) noexcept
{
    double mass = 0.0;
    for (const auto& particle : particles)
    {
        mass += static_cast<double>(particle.mass);
    }
    return mass;
}

[[nodiscard]] inline Vec2 particle_center_of_mass(const std::vector<FluidParticle>& particles) noexcept
{
    const double mass = total_particle_mass(particles);
    if (mass <= 0.0)
    {
        return {};
    }

    Vec2 weighted_sum{};
    for (const auto& particle : particles)
    {
        weighted_sum.x += particle.position.x * particle.mass;
        weighted_sum.y += particle.position.y * particle.mass;
    }
    return weighted_sum / static_cast<float>(mass);
}

[[nodiscard]] inline FluidDensityMetrics update_particle_density_metrics(
    std::vector<FluidParticle>& particles,
    FluidDensitySettings settings) noexcept
{
    FluidDensityMetrics metrics;
    metrics.particle_count = particles.size();
    if (particles.empty())
    {
        return metrics;
    }

    settings.rest_density = settings.rest_density > 0.0f ? settings.rest_density : 1.0f;
    settings.particle_volume = settings.particle_volume > 0.0f ? settings.particle_volume : 1.0f;
    settings.kernel_radius = settings.kernel_radius > 0.0f ? settings.kernel_radius : 1.0f;

    float min_x = particles.front().position.x;
    float min_y = particles.front().position.y;
    float max_x = particles.front().position.x;
    float max_y = particles.front().position.y;

    for (auto& particle : particles)
    {
        particle.volume = particle.volume > 0.0f ? particle.volume : settings.particle_volume;
        particle.mass = settings.rest_density * particle.volume;
        particle.density = 0.0f;
        particle.neighbor_count = 0;

        min_x = std::min(min_x, particle.position.x);
        min_y = std::min(min_y, particle.position.y);
        max_x = std::max(max_x, particle.position.x);
        max_y = std::max(max_y, particle.position.y);
    }

    const float support_radius = settings.kernel_radius * 2.0f;
    const float bucket_size = std::max(support_radius, 0.0001f);
    const auto bucket_for = [&](float value, float origin) -> int
    {
        return static_cast<int>(std::floor((value - origin) / bucket_size));
    };

    const int bucket_width = std::max(1, bucket_for(max_x, min_x) + 1);
    const int bucket_height = std::max(1, bucket_for(max_y, min_y) + 1);
    const std::size_t bucket_count = static_cast<std::size_t>(bucket_width * bucket_height);
    std::vector<int> particle_bucket_indices(particles.size(), 0);
    std::vector<std::size_t> bucket_counts(bucket_count, 0);

    const auto bucket_index = [&](int x, int y) -> std::size_t
    {
        return static_cast<std::size_t>(y * bucket_width + x);
    };

    for (std::size_t index = 0; index < particles.size(); ++index)
    {
        const int bucket_x = std::clamp(bucket_for(particles[index].position.x, min_x), 0, bucket_width - 1);
        const int bucket_y = std::clamp(bucket_for(particles[index].position.y, min_y), 0, bucket_height - 1);
        const std::size_t bucket = bucket_index(bucket_x, bucket_y);
        particle_bucket_indices[index] = static_cast<int>(bucket);
        ++bucket_counts[bucket];
    }

    std::vector<std::size_t> bucket_offsets(bucket_count + 1, 0);
    for (std::size_t bucket = 0; bucket < bucket_count; ++bucket)
    {
        bucket_offsets[bucket + 1] = bucket_offsets[bucket] + bucket_counts[bucket];
    }

    std::vector<std::size_t> bucket_cursors = bucket_offsets;
    std::vector<std::size_t> bucket_particle_indices(particles.size(), 0);
    for (std::size_t particle_index = 0; particle_index < particles.size(); ++particle_index)
    {
        const std::size_t bucket = static_cast<std::size_t>(particle_bucket_indices[particle_index]);
        bucket_particle_indices[bucket_cursors[bucket]++] = particle_index;
    }

    double density_sum = 0.0;
    double density_error_sum = 0.0;
    double neighbor_sum = 0.0;
    metrics.min_density = std::numeric_limits<double>::max();
    const float support_radius_squared = support_radius * support_radius;

    for (std::size_t particle_index = 0; particle_index < particles.size(); ++particle_index)
    {
        auto& particle = particles[particle_index];
        const int center_bucket_x = std::clamp(bucket_for(particle.position.x, min_x), 0, bucket_width - 1);
        const int center_bucket_y = std::clamp(bucket_for(particle.position.y, min_y), 0, bucket_height - 1);

        double density = 0.0;
        std::size_t neighbor_count = 0;

        for (int offset_y = -1; offset_y <= 1; ++offset_y)
        {
            const int bucket_y = center_bucket_y + offset_y;
            if (bucket_y < 0 || bucket_y >= bucket_height)
            {
                continue;
            }

            for (int offset_x = -1; offset_x <= 1; ++offset_x)
            {
                const int bucket_x = center_bucket_x + offset_x;
                if (bucket_x < 0 || bucket_x >= bucket_width)
                {
                    continue;
                }

                const std::size_t bucket = bucket_index(bucket_x, bucket_y);
                for (std::size_t cursor = bucket_offsets[bucket]; cursor < bucket_offsets[bucket + 1]; ++cursor)
                {
                    const std::size_t neighbor_index = bucket_particle_indices[cursor];
                    const auto& neighbor = particles[neighbor_index];
                    const float distance_squared = length_squared(particle.position - neighbor.position);
                    if (distance_squared >= support_radius_squared)
                    {
                        continue;
                    }

                    const float distance_value = std::sqrt(distance_squared);
                    const float weight = cubic_spline_kernel_2d(distance_value, settings.kernel_radius);
                    if (weight <= 0.0f)
                    {
                        continue;
                    }

                    density += static_cast<double>(neighbor.mass) * static_cast<double>(weight);
                    if (neighbor_index != particle_index)
                    {
                        ++neighbor_count;
                    }
                }
            }
        }

        particle.density = static_cast<float>(density);
        particle.neighbor_count = neighbor_count;

        const double density_error = std::abs(density - static_cast<double>(settings.rest_density))
            / static_cast<double>(settings.rest_density);
        metrics.min_density = std::min(metrics.min_density, density);
        metrics.max_density = std::max(metrics.max_density, density);
        metrics.max_density_error = std::max(metrics.max_density_error, density_error);
        metrics.max_neighbor_count = std::max(metrics.max_neighbor_count, neighbor_count);
        density_sum += density;
        density_error_sum += density_error;
        neighbor_sum += static_cast<double>(neighbor_count);
    }

    const double count = static_cast<double>(particles.size());
    metrics.average_density = density_sum / count;
    metrics.average_density_error = density_error_sum / count;
    metrics.average_neighbor_count = neighbor_sum / count;
    if (metrics.min_density == std::numeric_limits<double>::max())
    {
        metrics.min_density = 0.0;
    }
    return metrics;
}

[[nodiscard]] inline DensityCorrectionResult apply_density_constraint_correction(
    std::vector<FluidParticle>& particles,
    FluidDensitySettings density_settings,
    DensityCorrectionSettings correction_settings) noexcept
{
    DensityCorrectionResult result;
    if (particles.empty())
    {
        return result;
    }

    density_settings.rest_density = density_settings.rest_density > 0.0f ? density_settings.rest_density : 1.0f;
    density_settings.particle_volume = density_settings.particle_volume > 0.0f ? density_settings.particle_volume : 1.0f;
    density_settings.kernel_radius = density_settings.kernel_radius > 0.0f ? density_settings.kernel_radius : 1.0f;
    correction_settings.iterations = std::max(0, correction_settings.iterations);
    correction_settings.max_correction = std::max(0.0f, correction_settings.max_correction);
    correction_settings.epsilon = correction_settings.epsilon > 0.0f ? correction_settings.epsilon : 1.0e-5f;

    FluidDensityMetrics before = update_particle_density_metrics(particles, density_settings);
    result.max_density_error_before = before.max_density_error;
    result.total_mass_before = total_particle_mass(particles);
    result.center_of_mass_before = particle_center_of_mass(particles);

    const float support_radius = density_settings.kernel_radius * 2.0f;
    const float support_radius_squared = support_radius * support_radius;

    struct DensityBucketGrid
    {
        float origin_x = 0.0f;
        float origin_y = 0.0f;
        float bucket_size = 1.0f;
        int width = 1;
        int height = 1;
        std::vector<std::size_t> offsets{};
        std::vector<std::size_t> particle_indices{};
    };

    const auto build_bucket_grid = [&](const std::vector<FluidParticle>& source) -> DensityBucketGrid
    {
        DensityBucketGrid grid;
        grid.bucket_size = std::max(density_settings.kernel_radius, 0.0001f);

        float min_x = source.front().position.x;
        float min_y = source.front().position.y;
        float max_x = source.front().position.x;
        float max_y = source.front().position.y;

        for (const auto& particle : source)
        {
            min_x = std::min(min_x, particle.position.x);
            min_y = std::min(min_y, particle.position.y);
            max_x = std::max(max_x, particle.position.x);
            max_y = std::max(max_y, particle.position.y);
        }

        grid.origin_x = min_x;
        grid.origin_y = min_y;

        const auto bucket_for = [&](float value, float origin) -> int
        {
            return static_cast<int>(std::floor((value - origin) / grid.bucket_size));
        };

        grid.width = std::max(1, bucket_for(max_x, min_x) + 1);
        grid.height = std::max(1, bucket_for(max_y, min_y) + 1);

        const std::size_t bucket_count = static_cast<std::size_t>(grid.width * grid.height);
        std::vector<std::size_t> bucket_counts(bucket_count, 0);
        std::vector<std::size_t> particle_bucket_indices(source.size(), 0);

        const auto bucket_index = [&](int x, int y) -> std::size_t
        {
            return static_cast<std::size_t>(y * grid.width + x);
        };

        for (std::size_t index = 0; index < source.size(); ++index)
        {
            const int bucket_x = std::clamp(bucket_for(source[index].position.x, min_x), 0, grid.width - 1);
            const int bucket_y = std::clamp(bucket_for(source[index].position.y, min_y), 0, grid.height - 1);
            const std::size_t bucket = bucket_index(bucket_x, bucket_y);
            particle_bucket_indices[index] = bucket;
            ++bucket_counts[bucket];
        }

        grid.offsets.assign(bucket_count + 1, 0);
        for (std::size_t bucket = 0; bucket < bucket_count; ++bucket)
        {
            grid.offsets[bucket + 1] = grid.offsets[bucket] + bucket_counts[bucket];
        }

        grid.particle_indices.assign(source.size(), 0);
        std::vector<std::size_t> cursors = grid.offsets;
        for (std::size_t particle_index = 0; particle_index < source.size(); ++particle_index)
        {
            const std::size_t bucket = particle_bucket_indices[particle_index];
            grid.particle_indices[cursors[bucket]++] = particle_index;
        }

        return grid;
    };

    std::vector<float> lambdas(particles.size(), 0.0f);
    std::vector<Vec2> corrections(particles.size(), {});

    for (int iteration = 0; iteration < correction_settings.iterations; ++iteration)
    {
        (void)update_particle_density_metrics(particles, density_settings);
        const DensityBucketGrid buckets = build_bucket_grid(particles);
        const auto bucket_for = [&](float value, float origin) -> int
        {
            return static_cast<int>(std::floor((value - origin) / buckets.bucket_size));
        };
        const auto bucket_index = [&](int x, int y) -> std::size_t
        {
            return static_cast<std::size_t>(y * buckets.width + x);
        };

        std::fill(lambdas.begin(), lambdas.end(), 0.0f);
        std::fill(corrections.begin(), corrections.end(), Vec2{});

        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            const float constraint = std::max(0.0f, particles[i].density / density_settings.rest_density - 1.0f);
            if (constraint <= 0.0f)
            {
                continue;
            }

            float gradient_sum = correction_settings.epsilon;
            const int center_bucket_x = std::clamp(bucket_for(particles[i].position.x, buckets.origin_x), 0, buckets.width - 1);
            const int center_bucket_y = std::clamp(bucket_for(particles[i].position.y, buckets.origin_y), 0, buckets.height - 1);

            for (int offset_y = -2; offset_y <= 2; ++offset_y)
            {
                const int bucket_y = center_bucket_y + offset_y;
                if (bucket_y < 0 || bucket_y >= buckets.height)
                {
                    continue;
                }

                for (int offset_x = -2; offset_x <= 2; ++offset_x)
                {
                    const int bucket_x = center_bucket_x + offset_x;
                    if (bucket_x < 0 || bucket_x >= buckets.width)
                    {
                        continue;
                    }

                    const std::size_t bucket = bucket_index(bucket_x, bucket_y);
                    for (std::size_t cursor = buckets.offsets[bucket]; cursor < buckets.offsets[bucket + 1]; ++cursor)
                    {
                        const std::size_t j = buckets.particle_indices[cursor];
                        if (i == j)
                        {
                            continue;
                        }

                        const Vec2 offset = particles[i].position - particles[j].position;
                        if (length_squared(offset) >= support_radius_squared)
                        {
                            continue;
                        }

                        const Vec2 gradient = cubic_spline_kernel_gradient_2d(offset, density_settings.kernel_radius)
                            * (particles[j].mass / density_settings.rest_density);
                        gradient_sum += length_squared(gradient);
                    }
                }
            }

            lambdas[i] = -constraint / gradient_sum;
        }

        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            Vec2 correction{};
            const int center_bucket_x = std::clamp(bucket_for(particles[i].position.x, buckets.origin_x), 0, buckets.width - 1);
            const int center_bucket_y = std::clamp(bucket_for(particles[i].position.y, buckets.origin_y), 0, buckets.height - 1);

            for (int offset_y = -2; offset_y <= 2; ++offset_y)
            {
                const int bucket_y = center_bucket_y + offset_y;
                if (bucket_y < 0 || bucket_y >= buckets.height)
                {
                    continue;
                }

                for (int offset_x = -2; offset_x <= 2; ++offset_x)
                {
                    const int bucket_x = center_bucket_x + offset_x;
                    if (bucket_x < 0 || bucket_x >= buckets.width)
                    {
                        continue;
                    }

                    const std::size_t bucket = bucket_index(bucket_x, bucket_y);
                    for (std::size_t cursor = buckets.offsets[bucket]; cursor < buckets.offsets[bucket + 1]; ++cursor)
                    {
                        const std::size_t j = buckets.particle_indices[cursor];
                        if (i == j)
                        {
                            continue;
                        }

                        const Vec2 offset = particles[i].position - particles[j].position;
                        if (length_squared(offset) >= support_radius_squared)
                        {
                            continue;
                        }

                        const Vec2 gradient = cubic_spline_kernel_gradient_2d(offset, density_settings.kernel_radius);
                        const float mass_scale = particles[j].mass / density_settings.rest_density;
                        correction = correction + gradient * ((lambdas[i] + lambdas[j]) * mass_scale / density_settings.rest_density);
                    }
                }
            }

            const float correction_length = length(correction);
            if (correction_length > correction_settings.max_correction && correction_length > 0.0f)
            {
                correction = correction * (correction_settings.max_correction / correction_length);
            }
            corrections[i] = correction;
            result.max_applied_correction = std::max(result.max_applied_correction, length(correction));
        }

        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            particles[i].position = particles[i].position + corrections[i];
        }
    }

    const Vec2 center_after_iterations = particle_center_of_mass(particles);
    const Vec2 recenter_offset = result.center_of_mass_before - center_after_iterations;
    for (auto& particle : particles)
    {
        particle.position = particle.position + recenter_offset;
    }

    const FluidDensityMetrics after = update_particle_density_metrics(particles, density_settings);
    result.max_density_error_after = after.max_density_error;
    result.total_mass_after = total_particle_mass(particles);
    result.center_of_mass_after = particle_center_of_mass(particles);
    return result;
}
} // namespace physics_sim
