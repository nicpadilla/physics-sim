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
    lines.reserve(17);
    lines.emplace_back("HELP");
    lines.emplace_back("Pointer water: hold LMB to pour");
    lines.emplace_back("LMB draws or erases with the selected wall tool");
    lines.emplace_back("0 pour, 1 draw wall, 2 erase wall");
    lines.emplace_back("A opens Advanced Tools; 3-9 select devices");
    lines.emplace_back("3 hose, 4 omni, 5 gate, 6 sensor");
    lines.emplace_back("7 drain, 8 pump, 9 valve");
    lines.emplace_back("T toggles selected devices; Delete removes them");
    lines.emplace_back(keycode_label(bindings.tool_prev) + "/" + keycode_label(bindings.tool_next) + " cycle sandbox tools");
    lines.emplace_back(keycode_label(bindings.toggle_help) + " toggle help");
    lines.emplace_back("Esc opens pause menu / backs out");
    lines.emplace_back("Enter or click confirm menus");
    lines.emplace_back(keycode_label(bindings.undo) + " undo, " + keycode_label(bindings.redo) + " redo");
    lines.emplace_back(keycode_label(bindings.pause_resume) + " pause, " + keycode_label(bindings.step_once) + " step");
    lines.emplace_back(keycode_label(bindings.reset_fluid) + " clear fluid, " + keycode_label(bindings.reset) + " retry current scene");
    lines.emplace_back(keycode_label(bindings.save_scene) + " save, " + keycode_label(bindings.load_scene) + " load");
    lines.emplace_back("L opens the laboratory");
    return lines;
}

[[nodiscard]] inline std::vector<std::string> build_tutorial_overlay_lines(
    const TutorialProgress& progress,
    const InputBindings& bindings)
{
    const TutorialStep step = tutorial_current_step(progress);
    const std::array<std::pair<TutorialStep, const char*>, 6> steps{
        std::make_pair(TutorialStep::PourWater, "Pour water"),
        std::make_pair(TutorialStep::PaintWall, "Paint a wall"),
        std::make_pair(TutorialStep::EraseWall, "Erase part of it"),
        std::make_pair(TutorialStep::PauseResume, "Pause and resume"),
        std::make_pair(TutorialStep::ResetOrRetry, "Reset or retry"),
        std::make_pair(TutorialStep::SaveLoad, "Save and load"),
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
