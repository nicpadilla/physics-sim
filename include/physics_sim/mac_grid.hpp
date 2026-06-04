#pragma once

#include <physics_sim/indexing.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace physics_sim
{
class MacGrid2D
{
public:
    using size_type = std::size_t;

    MacGrid2D() = default;

    MacGrid2D(size_type width, size_type height, float cell_size = 1.0f)
    {
        resize(width, height, cell_size);
    }

    void resize(size_type width, size_type height, float cell_size = 1.0f)
    {
        width_ = width;
        height_ = height;
        cell_size_ = cell_size > 0.0f ? cell_size : 1.0f;

        pressure_.assign(cell_count(), 0.0f);
        divergence_.assign(cell_count(), 0.0f);
        solid_.assign(cell_count(), std::uint8_t{0});
        velocity_u_.assign(u_count(), 0.0f);
        velocity_v_.assign(v_count(), 0.0f);
    }

    [[nodiscard]] size_type width() const noexcept
    {
        return width_;
    }

    [[nodiscard]] size_type height() const noexcept
    {
        return height_;
    }

    [[nodiscard]] float cell_size() const noexcept
    {
        return cell_size_;
    }

    [[nodiscard]] size_type cell_count() const noexcept
    {
        return width_ * height_;
    }

    [[nodiscard]] size_type u_count() const noexcept
    {
        return (width_ + 1) * height_;
    }

    [[nodiscard]] size_type v_count() const noexcept
    {
        return width_ * (height_ + 1);
    }

    [[nodiscard]] size_type cell_index(size_type x, size_type y) const noexcept
    {
        return flatten_index(x, y, width_);
    }

    [[nodiscard]] bool contains(size_type x, size_type y) const noexcept
    {
        return in_bounds(x, y, width_, height_);
    }

    [[nodiscard]] float& pressure(size_type x, size_type y)
    {
        return pressure_.at(cell_index(x, y));
    }

    [[nodiscard]] const float& pressure(size_type x, size_type y) const
    {
        return pressure_.at(cell_index(x, y));
    }

    [[nodiscard]] float& divergence(size_type x, size_type y)
    {
        return divergence_.at(cell_index(x, y));
    }

    [[nodiscard]] const float& divergence(size_type x, size_type y) const
    {
        return divergence_.at(cell_index(x, y));
    }

    [[nodiscard]] float& velocity_u(size_type x, size_type y)
    {
        return velocity_u_.at(y * (width_ + 1) + x);
    }

    [[nodiscard]] const float& velocity_u(size_type x, size_type y) const
    {
        return velocity_u_.at(y * (width_ + 1) + x);
    }

    [[nodiscard]] float& u(size_type x, size_type y)
    {
        return velocity_u(x, y);
    }

    [[nodiscard]] const float& u(size_type x, size_type y) const
    {
        return velocity_u(x, y);
    }

    [[nodiscard]] float& velocity_v(size_type x, size_type y)
    {
        return velocity_v_.at(y * width_ + x);
    }

    [[nodiscard]] const float& velocity_v(size_type x, size_type y) const
    {
        return velocity_v_.at(y * width_ + x);
    }

    [[nodiscard]] float& v(size_type x, size_type y)
    {
        return velocity_v(x, y);
    }

    [[nodiscard]] const float& v(size_type x, size_type y) const
    {
        return velocity_v(x, y);
    }

    [[nodiscard]] bool solid(size_type x, size_type y) const
    {
        return solid_.at(cell_index(x, y)) != std::uint8_t{0};
    }

    void set_solid(size_type x, size_type y, bool value)
    {
        solid_.at(cell_index(x, y)) = value ? std::uint8_t{1} : std::uint8_t{0};
    }

    void clear_fields() noexcept
    {
        std::fill(pressure_.begin(), pressure_.end(), 0.0f);
        std::fill(divergence_.begin(), divergence_.end(), 0.0f);
        std::fill(velocity_u_.begin(), velocity_u_.end(), 0.0f);
        std::fill(velocity_v_.begin(), velocity_v_.end(), 0.0f);
    }

    void clear_solids() noexcept
    {
        std::fill(solid_.begin(), solid_.end(), std::uint8_t{0});
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return pressure_.size() == cell_count()
            && divergence_.size() == cell_count()
            && solid_.size() == cell_count()
            && velocity_u_.size() == u_count()
            && velocity_v_.size() == v_count();
    }

    [[nodiscard]] std::vector<float>& pressure_values() noexcept
    {
        return pressure_;
    }

    [[nodiscard]] const std::vector<float>& pressure_values() const noexcept
    {
        return pressure_;
    }

    [[nodiscard]] std::vector<float>& divergence_values() noexcept
    {
        return divergence_;
    }

    [[nodiscard]] const std::vector<float>& divergence_values() const noexcept
    {
        return divergence_;
    }

    [[nodiscard]] std::vector<float>& u_values() noexcept
    {
        return velocity_u_;
    }

    [[nodiscard]] const std::vector<float>& u_values() const noexcept
    {
        return velocity_u_;
    }

    [[nodiscard]] std::vector<float>& v_values() noexcept
    {
        return velocity_v_;
    }

    [[nodiscard]] const std::vector<float>& v_values() const noexcept
    {
        return velocity_v_;
    }

    [[nodiscard]] std::vector<float>& u_raw() noexcept
    {
        return velocity_u_;
    }

    [[nodiscard]] std::vector<float>& v_raw() noexcept
    {
        return velocity_v_;
    }

private:
    size_type width_ = 0;
    size_type height_ = 0;
    float cell_size_ = 1.0f;
    std::vector<float> pressure_;
    std::vector<float> divergence_;
    std::vector<float> velocity_u_;
    std::vector<float> velocity_v_;
    std::vector<std::uint8_t> solid_;
};
} // namespace physics_sim
