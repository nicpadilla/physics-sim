#pragma once

#include <physics_sim/math.hpp>
#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <cmath>

namespace physics_sim
{
enum class SceneTool
{
    PaintWall,
    EraseWall,
    DirectionalEmitter,
    OmniEmitter,
};

class SceneController
{
public:
    explicit SceneController(WaterSimulation2D& simulation) noexcept
        : simulation_(&simulation)
    {
    }

    void set_tool(SceneTool tool) noexcept
    {
        if (tool_ != tool)
        {
            stroke_active_ = false;
        }

        tool_ = tool;
    }

    [[nodiscard]] SceneTool tool() const noexcept
    {
        return tool_;
    }

    void set_emitter_direction(Vec2 direction) noexcept
    {
        const float direction_length = length(direction);
        emitter_direction_ = direction_length > 0.0f ? direction / direction_length : Vec2{0.0f, 1.0f};
    }

    void rotate_emitter_direction(float radians) noexcept
    {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        set_emitter_direction({emitter_direction_.x * c - emitter_direction_.y * s, emitter_direction_.x * s + emitter_direction_.y * c});
    }

    [[nodiscard]] Vec2 emitter_direction() const noexcept
    {
        return emitter_direction_;
    }

    void set_emitter_speed(float speed) noexcept
    {
        emitter_speed_ = std::max(0.0f, speed);
    }

    void set_emission_rate(float rate) noexcept
    {
        emission_rate_ = std::max(0.0f, rate);
    }

    void begin_stroke(Vec2 world_position) noexcept
    {
        if (!is_wall_tool())
        {
            place_fixture(world_position);
            return;
        }

        stroke_active_ = true;
        stroke_previous_ = world_position;
        paint_segment(world_position, world_position);
    }

    void drag_stroke(Vec2 world_position) noexcept
    {
        if (!stroke_active_)
        {
            return;
        }

        paint_segment(stroke_previous_, world_position);
        stroke_previous_ = world_position;
    }

    void end_stroke(Vec2 world_position) noexcept
    {
        if (!stroke_active_)
        {
            return;
        }

        paint_segment(stroke_previous_, world_position);
        stroke_active_ = false;
    }

    void place_fixture(Vec2 world_position) noexcept
    {
        if (simulation_ == nullptr)
        {
            return;
        }

        WaterEmitter emitter;
        emitter.position = world_position;
        emitter.direction = emitter_direction_;
        emitter.speed = emitter_speed_;
        emitter.emission_rate = emission_rate_;

        switch (tool_)
        {
        case SceneTool::DirectionalEmitter:
            emitter.kind = WaterEmitterKind::Directional;
            simulation_->add_emitter(emitter);
            break;
        case SceneTool::OmniEmitter:
            emitter.kind = WaterEmitterKind::Omni;
            simulation_->add_emitter(emitter);
            break;
        default:
            break;
        }
    }

    void reset_scene() noexcept
    {
        if (simulation_ != nullptr)
        {
            simulation_->clear_scene();
        }

        stroke_active_ = false;
    }

private:
    [[nodiscard]] bool is_wall_tool() const noexcept
    {
        return tool_ == SceneTool::PaintWall || tool_ == SceneTool::EraseWall;
    }

    void paint_segment(Vec2 start, Vec2 end) noexcept
    {
        if (simulation_ == nullptr)
        {
            return;
        }

        simulation_->paint_wall_line(start, end, tool_ == SceneTool::PaintWall);
    }

    WaterSimulation2D* simulation_ = nullptr;
    SceneTool tool_ = SceneTool::PaintWall;
    bool stroke_active_ = false;
    Vec2 stroke_previous_{};
    Vec2 emitter_direction_{0.0f, 1.0f};
    float emitter_speed_ = 6.0f;
    float emission_rate_ = 96.0f;
};
} // namespace physics_sim
