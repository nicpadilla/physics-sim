#pragma once

#include <physics_sim/input_bindings.hpp>
#include <physics_sim/tutorial_progress.hpp>

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace physics_sim
{
[[nodiscard]] inline std::vector<std::string> build_help_overlay_lines(const InputBindings& bindings)
{
    std::vector<std::string> lines;
    lines.reserve(22);
    lines.emplace_back("HELP");
    lines.emplace_back("LMB paint / place / select");
    lines.emplace_back("RMB clear selection");
    lines.emplace_back("1-9 tools, " + keycode_label(bindings.tool_prev) + "/" + keycode_label(bindings.tool_next) + " cycle, " + keycode_label(bindings.cycle_visual_mode) + " visual mode");
    lines.emplace_back(keycode_label(bindings.toggle_help) + " toggle help");
    lines.emplace_back("Esc opens pause menu / backs out");
    lines.emplace_back("Enter or click confirm menus");
    lines.emplace_back(keycode_label(bindings.undo) + " undo, " + keycode_label(bindings.redo) + " redo");
    lines.emplace_back("Arrows/WASD set direction");
    lines.emplace_back("Ctrl+Arrows/WASD move selected");
    lines.emplace_back(keycode_label(bindings.rotate_counterclockwise) + "/" + keycode_label(bindings.rotate_clockwise) + " rotate selected or tool");
    lines.emplace_back(keycode_label(bindings.speed_down) + "/" + keycode_label(bindings.speed_up) + " speed, " + keycode_label(bindings.emission_down) + "/" + keycode_label(bindings.emission_up) + " emission");
    lines.emplace_back("T toggle selected device");
    lines.emplace_back("Gate tool places a door");
    lines.emplace_back("Sensor tool places trigger");
    lines.emplace_back("Drain tool removes water");
    lines.emplace_back("Pump tool pushes flow");
    lines.emplace_back("Valve tool routes flow");
    lines.emplace_back(keycode_label(bindings.pause_resume) + " pause, " + keycode_label(bindings.step_once) + " step");
    lines.emplace_back(keycode_label(bindings.reset_fluid) + " clear fluid, " + keycode_label(bindings.reset) + " retry current scene");
    lines.emplace_back("PgUp/PgDn browse scenes");
    lines.emplace_back("Delete remove selected or clear scene");
    return lines;
}

[[nodiscard]] inline std::vector<std::string> build_tutorial_overlay_lines(
    const TutorialProgress& progress,
    const InputBindings& bindings)
{
    const TutorialStep step = tutorial_current_step(progress);
    const std::array<std::pair<TutorialStep, const char*>, 10> steps{
        std::make_pair(TutorialStep::CameraPan, "Pan the camera"),
        std::make_pair(TutorialStep::CameraZoom, "Zoom the camera"),
        std::make_pair(TutorialStep::PauseResume, "Pause and resume"),
        std::make_pair(TutorialStep::PaintWall, "Paint a wall"),
        std::make_pair(TutorialStep::EraseWall, "Erase part of it"),
        std::make_pair(TutorialStep::PlaceFixture, "Place a fixture"),
        std::make_pair(TutorialStep::UseDevice, "Use a device"),
        std::make_pair(TutorialStep::ResetOrRetry, "Reset or retry"),
        std::make_pair(TutorialStep::SaveLoad, "Save and load"),
        std::make_pair(TutorialStep::GalleryBrowse, "Browse the gallery"),
    };

    std::vector<std::string> lines;
    lines.reserve(14);
    lines.emplace_back("TUTORIAL");
    lines.emplace_back(std::string{"NOW: "} + std::string{tutorial_step_title(step)});
    lines.emplace_back(std::string{tutorial_step_description(step)});
    lines.emplace_back("ESC OPENS PAUSE MENU");
    lines.emplace_back(keycode_label(bindings.pause_resume) + " ALSO PAUSES OR RESUMES");
    lines.emplace_back("");

    for (const auto& [tutorial_step, label] : steps)
    {
        const auto step_rank = static_cast<std::size_t>(tutorial_step);
        const auto current_rank = static_cast<std::size_t>(step);
        lines.emplace_back(std::string{step_rank == current_rank ? "> " : (step_rank < current_rank ? "[x] " : "[ ] ")} + label);
    }

    if (tutorial_is_complete(progress))
    {
        lines.emplace_back("");
        lines.emplace_back("TUTORIAL COMPLETE");
    }

    return lines;
}
} // namespace physics_sim
