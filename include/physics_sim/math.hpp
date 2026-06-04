#pragma once

#include <cmath>

namespace physics_sim
{
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

constexpr Vec2 operator+(const Vec2& lhs, const Vec2& rhs) noexcept
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

constexpr Vec2 operator-(const Vec2& lhs, const Vec2& rhs) noexcept
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

constexpr Vec2 operator*(const Vec2& value, float scale) noexcept
{
    return {value.x * scale, value.y * scale};
}

constexpr Vec2 operator/(const Vec2& value, float scale) noexcept
{
    return {value.x / scale, value.y / scale};
}

inline float length_squared(const Vec2& value) noexcept
{
    return value.x * value.x + value.y * value.y;
}

inline float length(const Vec2& value) noexcept
{
    return std::sqrt(length_squared(value));
}

inline Vec2 normalize(const Vec2& value) noexcept
{
    const float len = length(value);
    if (len <= 0.0f)
    {
        return {};
    }

    return value / len;
}

template <typename T>
constexpr T clamp(const T& value, const T& min_value, const T& max_value) noexcept
{
    return value < min_value ? min_value : (value > max_value ? max_value : value);
}
} // namespace physics_sim

