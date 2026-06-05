#pragma once

#include <physics_sim/math.hpp>

#include <cstddef>

namespace physics_sim
{
struct FluidParticle
{
    Vec2 position{};
    Vec2 velocity{};
    float mass = 0.0f;
    float volume = 0.0f;
    float density = 0.0f;
    Mat2 affine_velocity{};
    std::size_t neighbor_count = 0;
};
} // namespace physics_sim
