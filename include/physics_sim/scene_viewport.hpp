#pragma once

#include <physics_sim/math.hpp>

#include <algorithm>
#include <cmath>

namespace physics_sim
{
class SceneViewport
{
public:
    void set_world_size(Vec2 world_size) noexcept
    {
        world_size_.x = world_size.x > 0.0f ? world_size.x : 1.0f;
        world_size_.y = world_size.y > 0.0f ? world_size.y : 1.0f;
        reset();
    }

    void set_window_size(int width, int height) noexcept
    {
        window_size_.x = width > 0 ? static_cast<float>(width) : 1.0f;
        window_size_.y = height > 0 ? static_cast<float>(height) : 1.0f;
        update_scale();
    }

    void reset() noexcept
    {
        center_world_ = {world_size_.x * 0.5f, world_size_.y * 0.5f};
        zoom_ = 1.0f;
        update_scale();
    }

    [[nodiscard]] float scale() const noexcept
    {
        return scale_;
    }

    [[nodiscard]] Vec2 window_to_world(Vec2 screen_position) const noexcept
    {
        const Vec2 screen_offset = screen_position - window_center();
        return center_world_ + (screen_offset / scale_);
    }

    [[nodiscard]] Vec2 world_to_window(Vec2 world_position) const noexcept
    {
        const Vec2 world_offset = world_position - center_world_;
        return window_center() + (world_offset * scale_);
    }

    void pan_pixels(Vec2 delta_pixels) noexcept
    {
        center_world_ = center_world_ - (delta_pixels / scale_);
    }

    void zoom_at(float factor, Vec2 anchor_screen) noexcept
    {
        if (factor <= 0.0f)
        {
            return;
        }

        const Vec2 anchor_world = window_to_world(anchor_screen);
        zoom_ = clamp(zoom_ * factor, min_zoom_, max_zoom_);
        update_scale();
        center_world_ = anchor_world - ((anchor_screen - window_center()) / scale_);
    }

    [[nodiscard]] Vec2 center_world() const noexcept
    {
        return center_world_;
    }

    [[nodiscard]] Vec2 window_size() const noexcept
    {
        return window_size_;
    }

    [[nodiscard]] Vec2 world_size() const noexcept
    {
        return world_size_;
    }

private:
    void update_scale() noexcept
    {
        const float scale_x = world_size_.x > 0.0f ? window_size_.x / world_size_.x : 1.0f;
        const float scale_y = world_size_.y > 0.0f ? window_size_.y / world_size_.y : 1.0f;
        base_scale_ = std::max(0.0001f, std::min(scale_x, scale_y));
        scale_ = base_scale_ * zoom_;
    }

    [[nodiscard]] Vec2 window_center() const noexcept
    {
        return {window_size_.x * 0.5f, window_size_.y * 0.5f};
    }

    Vec2 world_size_{1280.0f, 720.0f};
    Vec2 window_size_{1280.0f, 720.0f};
    Vec2 center_world_{640.0f, 360.0f};
    float zoom_ = 1.0f;
    float base_scale_ = 1.0f;
    float scale_ = 1.0f;
    const float min_zoom_ = 0.25f;
    const float max_zoom_ = 8.0f;
};
} // namespace physics_sim
