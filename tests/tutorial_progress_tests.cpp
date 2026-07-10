#include <physics_sim/tutorial_progress.hpp>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace
{
[[noreturn]] void fail(const char* message)
{
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message)); \
        } \
    } while (false)
} // namespace

int main()
{
    using physics_sim::TutorialStep;

    REQUIRE(physics_sim::tutorial_step_count() == 6, "tutorial step count was incorrect");
    REQUIRE(physics_sim::tutorial_step_title(TutorialStep::PourWater) == std::string_view{"POUR WATER"}, "pour-water title was incorrect");
    REQUIRE(physics_sim::tutorial_step_description(TutorialStep::SaveLoad) == std::string_view{"Save and load the scene."}, "save/load description was incorrect");

    physics_sim::TutorialProgress progress;
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::PourWater, "tutorial did not start on the pour-water step");

    physics_sim::tutorial_mark_water_poured(progress);
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::PaintWall, "tutorial did not advance after pouring");

    physics_sim::tutorial_mark_wall_painted(progress);
    physics_sim::tutorial_mark_wall_erased(progress);
    physics_sim::tutorial_mark_pause_opened(progress);
    physics_sim::tutorial_mark_pause_resumed(progress);
    physics_sim::tutorial_mark_reset_or_retry(progress);
    physics_sim::tutorial_mark_save_or_load(progress);

    REQUIRE(physics_sim::tutorial_is_complete(progress), "tutorial did not report completion after all milestones");
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::Complete, "tutorial did not report the complete step");

    physics_sim::tutorial_mark_camera_panned(progress);
    REQUIRE(physics_sim::tutorial_is_complete(progress), "tutorial completion regressed after duplicate progress updates");

    std::cout << "tutorial progress tests passed" << std::endl;
    return 0;
}
