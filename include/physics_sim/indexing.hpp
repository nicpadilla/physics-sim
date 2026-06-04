#pragma once

#include <cstddef>
#include <limits>

namespace physics_sim
{
inline constexpr std::size_t index_npos = std::numeric_limits<std::size_t>::max();

constexpr std::size_t flatten_index(std::size_t x, std::size_t y, std::size_t width) noexcept
{
    return y * width + x;
}

constexpr bool in_bounds(std::size_t x, std::size_t y, std::size_t width, std::size_t height) noexcept
{
    return x < width && y < height;
}

constexpr std::size_t safe_flatten_index(std::size_t x, std::size_t y, std::size_t width, std::size_t height) noexcept
{
    return in_bounds(x, y, width, height) ? flatten_index(x, y, width) : index_npos;
}
} // namespace physics_sim

