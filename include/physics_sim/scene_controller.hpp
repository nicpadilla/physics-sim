#pragma once

#include <physics_sim/math.hpp>
#include <physics_sim/scene_document.hpp>
#include <physics_sim/water_simulation.hpp>

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

namespace physics_sim
{
enum class SceneTool
{
    PaintWall,
    EraseWall,
    DirectionalEmitter,
    OmniEmitter,
    Gate,
    Sensor,
    Drain,
    Pump,
    Valve,
};

[[nodiscard]] inline SceneTool next_scene_tool(SceneTool tool, int direction) noexcept
{
    if (direction == 0)
    {
        return tool;
    }

    constexpr std::array<SceneTool, 9> order{
        SceneTool::PaintWall,
        SceneTool::EraseWall,
        SceneTool::DirectionalEmitter,
        SceneTool::OmniEmitter,
        SceneTool::Gate,
        SceneTool::Sensor,
        SceneTool::Drain,
        SceneTool::Pump,
        SceneTool::Valve,
    };

    std::size_t index = 0;
    for (; index < order.size(); ++index)
    {
        if (order[index] == tool)
        {
            break;
        }
    }

    if (index >= order.size())
    {
        index = 0;
    }

    const int step = direction > 0 ? 1 : -1;
    const int size = static_cast<int>(order.size());
    const int next_index = (static_cast<int>(index) + step + size) % size;
    return order[static_cast<std::size_t>(next_index)];
}

class SceneController
{
public:
    explicit SceneController(WaterSimulation2D& simulation) noexcept
        : simulation_(&simulation)
    {
        sync_history();
    }

    void set_tool(SceneTool tool) noexcept
    {
        if (tool_ != tool)
        {
            stroke_active_ = false;
            clear_selection();
        }

        tool_ = tool;
    }

    [[nodiscard]] SceneTool tool() const noexcept
    {
        return tool_;
    }

    [[nodiscard]] bool has_selected_fixture() const noexcept
    {
        return selected_emitter_index_.has_value();
    }

    [[nodiscard]] std::optional<std::size_t> selected_fixture_index() const noexcept
    {
        return selected_emitter_index_;
    }

    [[nodiscard]] WaterEmitter* selected_fixture() noexcept
    {
        if (simulation_ == nullptr || !selected_emitter_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_emitter_index_;
        if (index >= simulation_->emitters().size())
        {
            return nullptr;
        }

        return &simulation_->emitters()[index];
    }

    [[nodiscard]] const WaterEmitter* selected_fixture() const noexcept
    {
        if (simulation_ == nullptr || !selected_emitter_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_emitter_index_;
        if (index >= simulation_->emitters().size())
        {
            return nullptr;
        }

        return &simulation_->emitters()[index];
    }

    [[nodiscard]] float emitter_speed() const noexcept
    {
        return emitter_speed_;
    }

    [[nodiscard]] float emission_rate() const noexcept
    {
        return emission_rate_;
    }

    [[nodiscard]] bool has_selected_gate() const noexcept
    {
        return selected_gate_index_.has_value();
    }

    [[nodiscard]] bool has_selected_sensor() const noexcept
    {
        return selected_sensor_index_.has_value();
    }

    [[nodiscard]] bool has_selected_valve() const noexcept
    {
        return selected_valve_index_.has_value();
    }

    [[nodiscard]] std::optional<std::size_t> selected_gate_index() const noexcept
    {
        return selected_gate_index_;
    }

    [[nodiscard]] std::optional<std::size_t> selected_sensor_index() const noexcept
    {
        return selected_sensor_index_;
    }

    [[nodiscard]] std::optional<std::size_t> selected_valve_index() const noexcept
    {
        return selected_valve_index_;
    }

    [[nodiscard]] WaterGate* selected_gate() noexcept
    {
        if (simulation_ == nullptr || !selected_gate_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_gate_index_;
        if (index >= simulation_->gates().size())
        {
            return nullptr;
        }

        return &simulation_->gates()[index];
    }

    [[nodiscard]] const WaterGate* selected_gate() const noexcept
    {
        if (simulation_ == nullptr || !selected_gate_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_gate_index_;
        if (index >= simulation_->gates().size())
        {
            return nullptr;
        }

        return &simulation_->gates()[index];
    }

    [[nodiscard]] WaterSensor* selected_sensor() noexcept
    {
        if (simulation_ == nullptr || !selected_sensor_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_sensor_index_;
        if (index >= simulation_->sensors().size())
        {
            return nullptr;
        }

        return &simulation_->sensors()[index];
    }

    [[nodiscard]] const WaterSensor* selected_sensor() const noexcept
    {
        if (simulation_ == nullptr || !selected_sensor_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_sensor_index_;
        if (index >= simulation_->sensors().size())
        {
            return nullptr;
        }

        return &simulation_->sensors()[index];
    }

    [[nodiscard]] WaterValve* selected_valve() noexcept
    {
        if (simulation_ == nullptr || !selected_valve_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_valve_index_;
        if (index >= simulation_->valves().size())
        {
            return nullptr;
        }

        return &simulation_->valves()[index];
    }

    [[nodiscard]] const WaterValve* selected_valve() const noexcept
    {
        if (simulation_ == nullptr || !selected_valve_index_.has_value())
        {
            return nullptr;
        }

        const std::size_t index = *selected_valve_index_;
        if (index >= simulation_->valves().size())
        {
            return nullptr;
        }

        return &simulation_->valves()[index];
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

    void clear_selection() noexcept
    {
        selected_emitter_index_.reset();
        selected_gate_index_.reset();
        selected_sensor_index_.reset();
        selected_valve_index_.reset();
    }

    [[nodiscard]] bool select_gate_at(Vec2 world_position, float hit_radius) noexcept
    {
        if (simulation_ == nullptr || simulation_->gates().empty())
        {
            selected_gate_index_.reset();
            selected_emitter_index_.reset();
            selected_sensor_index_.reset();
            selected_valve_index_.reset();
            return false;
        }

        const float hit_radius_squared = hit_radius * hit_radius;
        std::optional<std::size_t> best_index;
        float best_distance_squared = hit_radius_squared;

        for (std::size_t i = 0; i < simulation_->gates().size(); ++i)
        {
            const auto& gate = simulation_->gates()[i];
            const float center_x = (static_cast<float>(gate.x) + 0.5f) * simulation_->grid().cell_size();
            const float center_y = (static_cast<float>(gate.y) + 0.5f) * simulation_->grid().cell_size();
            const float dx = center_x - world_position.x;
            const float dy = center_y - world_position.y;
            const float distance_squared = dx * dx + dy * dy;
            if (distance_squared <= best_distance_squared)
            {
                best_distance_squared = distance_squared;
                best_index = i;
            }
        }

        if (best_index.has_value())
        {
            selected_emitter_index_.reset();
            selected_sensor_index_.reset();
            selected_valve_index_.reset();
        }
        selected_gate_index_ = best_index;
        return selected_gate_index_.has_value();
    }

    [[nodiscard]] bool select_sensor_at(Vec2 world_position) noexcept
    {
        if (simulation_ == nullptr || simulation_->sensors().empty())
        {
            selected_sensor_index_.reset();
            selected_emitter_index_.reset();
            selected_gate_index_.reset();
            selected_valve_index_.reset();
            return false;
        }

        std::optional<std::size_t> best_index;
        for (std::size_t i = 0; i < simulation_->sensors().size(); ++i)
        {
            const auto& sensor = simulation_->sensors()[i];
            const float left = static_cast<float>(sensor.x) * simulation_->grid().cell_size();
            const float top = static_cast<float>(sensor.y) * simulation_->grid().cell_size();
            const float right = left + static_cast<float>(sensor.width) * simulation_->grid().cell_size();
            const float bottom = top + static_cast<float>(sensor.height) * simulation_->grid().cell_size();
            if (world_position.x >= left && world_position.x < right && world_position.y >= top && world_position.y < bottom)
            {
                best_index = i;
                break;
            }
        }

        if (best_index.has_value())
        {
            selected_emitter_index_.reset();
            selected_gate_index_.reset();
            selected_valve_index_.reset();
        }
        selected_sensor_index_ = best_index;
        return selected_sensor_index_.has_value();
    }

    [[nodiscard]] bool select_valve_at(Vec2 world_position) noexcept
    {
        if (simulation_ == nullptr || simulation_->valves().empty())
        {
            selected_valve_index_.reset();
            selected_emitter_index_.reset();
            selected_gate_index_.reset();
            selected_sensor_index_.reset();
            return false;
        }

        std::optional<std::size_t> best_index;
        for (std::size_t i = 0; i < simulation_->valves().size(); ++i)
        {
            const auto& valve = simulation_->valves()[i];
            const float center_x = (static_cast<float>(valve.x) + 0.5f) * simulation_->grid().cell_size();
            const float center_y = (static_cast<float>(valve.y) + 0.5f) * simulation_->grid().cell_size();
            const float dx = center_x - world_position.x;
            const float dy = center_y - world_position.y;
            const float distance_squared = dx * dx + dy * dy;
            if (distance_squared <= simulation_->grid().cell_size() * simulation_->grid().cell_size())
            {
                best_index = i;
                break;
            }
        }

        if (best_index.has_value())
        {
            selected_emitter_index_.reset();
            selected_gate_index_.reset();
            selected_sensor_index_.reset();
        }

        selected_valve_index_ = best_index;
        return selected_valve_index_.has_value();
    }

    [[nodiscard]] bool select_fixture_at(Vec2 world_position, float hit_radius) noexcept
    {
        if (simulation_ == nullptr || simulation_->emitters().empty())
        {
            selected_emitter_index_.reset();
            return false;
        }

        const float hit_radius_squared = hit_radius * hit_radius;
        std::optional<std::size_t> best_index;
        float best_distance_squared = hit_radius_squared;

        for (std::size_t i = 0; i < simulation_->emitters().size(); ++i)
        {
            const auto& emitter = simulation_->emitters()[i];
            const float dx = emitter.position.x - world_position.x;
            const float dy = emitter.position.y - world_position.y;
            const float distance_squared = dx * dx + dy * dy;
            if (distance_squared <= best_distance_squared)
            {
                best_distance_squared = distance_squared;
                best_index = i;
            }
        }

        selected_emitter_index_ = best_index;
        if (selected_emitter_index_.has_value())
        {
            selected_gate_index_.reset();
            selected_sensor_index_.reset();
            selected_valve_index_.reset();
        }
        return selected_emitter_index_.has_value();
    }

    [[nodiscard]] bool set_selected_fixture_direction(Vec2 direction) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        const float direction_length = length(direction);
        selected->direction = direction_length > 0.0f ? direction / direction_length : Vec2{0.0f, 1.0f};
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool rotate_selected_fixture(float radians) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        const float c = std::cos(radians);
        const float s = std::sin(radians);
        const Vec2 rotated{
            selected->direction.x * c - selected->direction.y * s,
            selected->direction.x * s + selected->direction.y * c,
        };
        selected->direction = normalize(rotated);
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool move_selected_fixture(Vec2 delta) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        selected->position.x += delta.x;
        selected->position.y += delta.y;
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool set_selected_fixture_speed(float speed) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        selected->speed = std::max(0.0f, speed);
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool adjust_selected_fixture_speed(float delta) noexcept
    {
        auto* selected = selected_fixture();
        return selected != nullptr && set_selected_fixture_speed(selected->speed + delta);
    }

    [[nodiscard]] bool set_selected_fixture_emission_rate(float rate) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        selected->emission_rate = std::max(0.0f, rate);
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool adjust_selected_fixture_emission_rate(float delta) noexcept
    {
        auto* selected = selected_fixture();
        return selected != nullptr && set_selected_fixture_emission_rate(selected->emission_rate + delta);
    }

    [[nodiscard]] bool set_selected_fixture_enabled(bool enabled) noexcept
    {
        auto* selected = selected_fixture();
        if (selected == nullptr)
        {
            return false;
        }

        selected->enabled = enabled;
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool toggle_selected_fixture_enabled() noexcept
    {
        auto* selected = selected_fixture();
        return selected != nullptr && set_selected_fixture_enabled(!selected->enabled);
    }

    [[nodiscard]] bool can_place_fixture(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_emitter_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        if (!grid.contains(x, y))
        {
            return false;
        }

        return !grid.solid(x, y);
    }

    [[nodiscard]] bool can_place_gate(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_gate_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        if (!grid.contains(x, y))
        {
            return false;
        }

        return cell_is_clear(x, y);
    }

    [[nodiscard]] bool can_place_sensor(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_sensor_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        constexpr std::size_t sensor_width = 3;
        constexpr std::size_t sensor_height = 3;
        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        return region_is_clear(x, y, sensor_width, sensor_height);
    }

    [[nodiscard]] bool can_place_drain(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_drain_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        constexpr std::size_t region_width = 3;
        constexpr std::size_t region_height = 3;
        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        return region_is_clear(x, y, region_width, region_height);
    }

    [[nodiscard]] bool can_place_pump(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_pump_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        constexpr std::size_t region_width = 3;
        constexpr std::size_t region_height = 3;
        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        return region_is_clear(x, y, region_width, region_height);
    }

    [[nodiscard]] bool can_place_valve(Vec2 world_position) const noexcept
    {
        if (simulation_ == nullptr || !is_valve_tool())
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (grid.width() == 0 || grid.height() == 0)
        {
            return false;
        }

        const int cell_x = static_cast<int>(std::floor(world_position.x / grid.cell_size()));
        const int cell_y = static_cast<int>(std::floor(world_position.y / grid.cell_size()));
        if (cell_x < 0 || cell_y < 0)
        {
            return false;
        }

        const auto x = static_cast<std::size_t>(cell_x);
        const auto y = static_cast<std::size_t>(cell_y);
        return cell_is_clear(x, y);
    }

    void begin_stroke(Vec2 world_position) noexcept
    {
        if (is_emitter_tool())
        {
            place_fixture(world_position);
            return;
        }

        if (is_gate_tool())
        {
            place_gate(world_position);
            return;
        }

        if (is_sensor_tool())
        {
            place_sensor(world_position);
            return;
        }

        if (is_drain_tool())
        {
            place_drain(world_position);
            return;
        }

        if (is_pump_tool())
        {
            place_pump(world_position);
            return;
        }

        if (is_valve_tool())
        {
            if (!select_valve_at(world_position))
            {
                place_valve(world_position);
            }
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
        record_history_snapshot();
    }

    bool place_fixture(Vec2 world_position) noexcept
    {
        if (!can_place_fixture(world_position))
        {
            return false;
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

        if (tool_ == SceneTool::DirectionalEmitter || tool_ == SceneTool::OmniEmitter)
        {
            if (!simulation_->emitters().empty())
            {
                selected_emitter_index_ = simulation_->emitters().size() - 1;
                selected_gate_index_.reset();
                selected_sensor_index_.reset();
                selected_valve_index_.reset();
            }
            record_history_snapshot();
            return true;
        }

        return false;
    }

    bool place_gate(Vec2 world_position) noexcept
    {
        if (!can_place_gate(world_position) || simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        const std::size_t cell_x = static_cast<std::size_t>(std::floor(world_position.x / grid.cell_size()));
        const std::size_t cell_y = static_cast<std::size_t>(std::floor(world_position.y / grid.cell_size()));

        simulation_->add_gate(WaterGate{cell_x, cell_y, false});
        selected_emitter_index_.reset();
        selected_sensor_index_.reset();
        selected_valve_index_.reset();
        selected_gate_index_ = simulation_->gates().size() - 1;
        record_history_snapshot();
        return true;
    }

    bool place_sensor(Vec2 world_position) noexcept
    {
        if (!can_place_sensor(world_position) || simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        const std::size_t cell_x = static_cast<std::size_t>(std::floor(world_position.x / grid.cell_size()));
        const std::size_t cell_y = static_cast<std::size_t>(std::floor(world_position.y / grid.cell_size()));

        simulation_->add_sensor(WaterSensor{cell_x, cell_y, 3, 3, true, false, false, {}});
        simulation_->refresh_sensor_states();
        selected_emitter_index_.reset();
        selected_gate_index_.reset();
        selected_valve_index_.reset();
        selected_sensor_index_ = simulation_->sensors().size() - 1;
        record_history_snapshot();
        return true;
    }

    bool place_drain(Vec2 world_position) noexcept
    {
        if (!can_place_drain(world_position) || simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        const std::size_t cell_x = static_cast<std::size_t>(std::floor(world_position.x / grid.cell_size()));
        const std::size_t cell_y = static_cast<std::size_t>(std::floor(world_position.y / grid.cell_size()));

        simulation_->add_drain(WaterDrain{cell_x, cell_y, 3, 3, true});
        selected_emitter_index_.reset();
        selected_gate_index_.reset();
        selected_sensor_index_.reset();
        selected_valve_index_.reset();
        record_history_snapshot();
        return true;
    }

    bool place_pump(Vec2 world_position) noexcept
    {
        if (!can_place_pump(world_position) || simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        const std::size_t cell_x = static_cast<std::size_t>(std::floor(world_position.x / grid.cell_size()));
        const std::size_t cell_y = static_cast<std::size_t>(std::floor(world_position.y / grid.cell_size()));

        simulation_->add_pump(WaterPump{cell_x, cell_y, 3, 3, true, emitter_direction_, 12.0f});
        selected_emitter_index_.reset();
        selected_gate_index_.reset();
        selected_sensor_index_.reset();
        selected_valve_index_.reset();
        record_history_snapshot();
        return true;
    }

    bool place_valve(Vec2 world_position) noexcept
    {
        if (!can_place_valve(world_position) || simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        const std::size_t cell_x = static_cast<std::size_t>(std::floor(world_position.x / grid.cell_size()));
        const std::size_t cell_y = static_cast<std::size_t>(std::floor(world_position.y / grid.cell_size()));

        simulation_->add_valve(WaterValve{cell_x, cell_y, false});
        selected_emitter_index_.reset();
        selected_gate_index_.reset();
        selected_sensor_index_.reset();
        selected_valve_index_ = simulation_->valves().size() - 1;
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool delete_selected_fixture() noexcept
    {
        if (simulation_ == nullptr || !selected_emitter_index_.has_value())
        {
            return false;
        }

        const std::size_t selected_index = *selected_emitter_index_;
        if (selected_index >= simulation_->emitters().size())
        {
            selected_emitter_index_.reset();
            return false;
        }

        simulation_->emitters().erase(simulation_->emitters().begin() + static_cast<std::ptrdiff_t>(selected_index));
        selected_emitter_index_.reset();
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool delete_selected_gate() noexcept
    {
        if (simulation_ == nullptr || !selected_gate_index_.has_value())
        {
            return false;
        }

        const std::size_t selected_index = *selected_gate_index_;
        if (selected_index >= simulation_->gates().size())
        {
            selected_gate_index_.reset();
            return false;
        }

        simulation_->set_solid_cell(simulation_->gates()[selected_index].x, simulation_->gates()[selected_index].y, false);
        simulation_->gates().erase(simulation_->gates().begin() + static_cast<std::ptrdiff_t>(selected_index));
        selected_gate_index_.reset();
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool delete_selected_sensor() noexcept
    {
        if (simulation_ == nullptr || !selected_sensor_index_.has_value())
        {
            return false;
        }

        const std::size_t selected_index = *selected_sensor_index_;
        if (selected_index >= simulation_->sensors().size())
        {
            selected_sensor_index_.reset();
            return false;
        }

        simulation_->sensors().erase(simulation_->sensors().begin() + static_cast<std::ptrdiff_t>(selected_index));
        selected_sensor_index_.reset();
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool delete_selected_valve() noexcept
    {
        if (simulation_ == nullptr || !selected_valve_index_.has_value())
        {
            return false;
        }

        const std::size_t selected_index = *selected_valve_index_;
        if (selected_index >= simulation_->valves().size())
        {
            selected_valve_index_.reset();
            return false;
        }

        simulation_->set_solid_cell(simulation_->valves()[selected_index].x, simulation_->valves()[selected_index].y, false);
        simulation_->valves().erase(simulation_->valves().begin() + static_cast<std::ptrdiff_t>(selected_index));
        selected_valve_index_.reset();
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool toggle_selected_gate_open() noexcept
    {
        if (simulation_ == nullptr || !selected_gate_index_.has_value())
        {
            return false;
        }

        const bool toggled = simulation_->toggle_gate_open(*selected_gate_index_);
        if (toggled)
        {
            record_history_snapshot();
        }
        return toggled;
    }

    [[nodiscard]] bool toggle_selected_sensor_enabled() noexcept
    {
        auto* sensor = selected_sensor();
        if (sensor == nullptr)
        {
            return false;
        }

        sensor->enabled = !sensor->enabled;
        if (!sensor->enabled)
        {
            sensor->active = false;
        }
        if (simulation_ != nullptr)
        {
            simulation_->refresh_sensor_states();
        }
        record_history_snapshot();
        return true;
    }

    [[nodiscard]] bool toggle_selected_valve_open() noexcept
    {
        if (simulation_ == nullptr || !selected_valve_index_.has_value())
        {
            return false;
        }

        const bool toggled = simulation_->toggle_valve_open(*selected_valve_index_);
        if (toggled)
        {
            record_history_snapshot();
        }
        return toggled;
    }

    void reset_scene() noexcept
    {
        if (simulation_ != nullptr)
        {
            simulation_->clear_scene();
        }

        stroke_active_ = false;
        clear_selection();
        sync_history();
    }

    void sync_history() noexcept
    {
        history_.clear();
        history_index_ = 0;

        if (simulation_ != nullptr)
        {
            history_.push_back(capture_scene(*simulation_));
        }
    }

    [[nodiscard]] bool undo_scene_edit() noexcept
    {
        if (history_.empty() || history_index_ == 0)
        {
            return false;
        }

        --history_index_;
        restore_history_entry();
        return true;
    }

    [[nodiscard]] bool redo_scene_edit() noexcept
    {
        if (history_.empty() || history_index_ + 1 >= history_.size())
        {
            return false;
        }

        ++history_index_;
        restore_history_entry();
        return true;
    }

private:
    [[nodiscard]] bool is_wall_tool() const noexcept
    {
        return tool_ == SceneTool::PaintWall || tool_ == SceneTool::EraseWall;
    }

    [[nodiscard]] bool is_emitter_tool() const noexcept
    {
        return tool_ == SceneTool::DirectionalEmitter || tool_ == SceneTool::OmniEmitter;
    }

    [[nodiscard]] bool is_gate_tool() const noexcept
    {
        return tool_ == SceneTool::Gate;
    }

    [[nodiscard]] bool is_sensor_tool() const noexcept
    {
        return tool_ == SceneTool::Sensor;
    }

    [[nodiscard]] bool is_drain_tool() const noexcept
    {
        return tool_ == SceneTool::Drain;
    }

    [[nodiscard]] bool is_pump_tool() const noexcept
    {
        return tool_ == SceneTool::Pump;
    }

    [[nodiscard]] bool is_valve_tool() const noexcept
    {
        return tool_ == SceneTool::Valve;
    }

    [[nodiscard]] bool cell_is_clear(std::size_t x, std::size_t y) const noexcept
    {
        if (simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (!grid.contains(x, y) || grid.solid(x, y))
        {
            return false;
        }

        const float cell_size = grid.cell_size();
        for (const auto& emitter : simulation_->emitters())
        {
            const int emitter_x = static_cast<int>(std::floor(emitter.position.x / cell_size));
            const int emitter_y = static_cast<int>(std::floor(emitter.position.y / cell_size));
            if (emitter_x == static_cast<int>(x) && emitter_y == static_cast<int>(y))
            {
                return false;
            }
        }

        for (const auto& gate : simulation_->gates())
        {
            if (gate.x == x && gate.y == y)
            {
                return false;
            }
        }

        for (const auto& valve : simulation_->valves())
        {
            if (valve.x == x && valve.y == y)
            {
                return false;
            }
        }

        const auto overlaps_rect = [x, y](std::size_t left, std::size_t top, std::size_t width, std::size_t height) noexcept
        {
            return x >= left && x < left + width && y >= top && y < top + height;
        };

        for (const auto& sensor : simulation_->sensors())
        {
            if (overlaps_rect(sensor.x, sensor.y, sensor.width, sensor.height))
            {
                return false;
            }
        }

        for (const auto& drain : simulation_->drains())
        {
            if (overlaps_rect(drain.x, drain.y, drain.width, drain.height))
            {
                return false;
            }
        }

        for (const auto& pump : simulation_->pumps())
        {
            if (overlaps_rect(pump.x, pump.y, pump.width, pump.height))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool region_is_clear(std::size_t left, std::size_t top, std::size_t width, std::size_t height) const noexcept
    {
        if (simulation_ == nullptr)
        {
            return false;
        }

        const auto& grid = simulation_->grid();
        if (left + width > grid.width() || top + height > grid.height())
        {
            return false;
        }

        for (std::size_t offset_y = 0; offset_y < height; ++offset_y)
        {
            for (std::size_t offset_x = 0; offset_x < width; ++offset_x)
            {
                if (!cell_is_clear(left + offset_x, top + offset_y))
                {
                    return false;
                }
            }
        }

        return true;
    }

    void paint_segment(Vec2 start, Vec2 end) noexcept
    {
        if (simulation_ == nullptr)
        {
            return;
        }

        simulation_->paint_wall_line(start, end, tool_ == SceneTool::PaintWall);
    }

    void record_history_snapshot() noexcept
    {
        if (simulation_ == nullptr)
        {
            return;
        }

        if (history_.empty())
        {
            sync_history();
            return;
        }

        if (history_index_ + 1 < history_.size())
        {
            history_.erase(history_.begin() + static_cast<std::ptrdiff_t>(history_index_ + 1), history_.end());
        }

        history_.push_back(capture_scene(*simulation_));
        history_index_ = history_.size() - 1;
    }

    void restore_history_entry() noexcept
    {
        if (simulation_ == nullptr || history_.empty() || history_index_ >= history_.size())
        {
            return;
        }

        const SceneDocument& document = history_[history_index_];
        apply_scene(document, *simulation_);

        clear_selection();
    }

    WaterSimulation2D* simulation_ = nullptr;
    SceneTool tool_ = SceneTool::PaintWall;
    bool stroke_active_ = false;
    Vec2 stroke_previous_{};
    Vec2 emitter_direction_{0.0f, 1.0f};
    float emitter_speed_ = 6.0f;
    float emission_rate_ = 96.0f;
    std::optional<std::size_t> selected_emitter_index_{};
    std::optional<std::size_t> selected_gate_index_{};
    std::optional<std::size_t> selected_sensor_index_{};
    std::optional<std::size_t> selected_valve_index_{};
    std::vector<SceneDocument> history_{};
    std::size_t history_index_ = 0;
};
} // namespace physics_sim
