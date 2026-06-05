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

    REQUIRE(physics_sim::tutorial_step_count() == 10, "tutorial step count was incorrect");
    REQUIRE(physics_sim::tutorial_step_title(TutorialStep::CameraPan) == std::string_view{"PAN CAMERA"}, "camera-pan title was incorrect");
    REQUIRE(physics_sim::tutorial_step_description(TutorialStep::SaveLoad) == std::string_view{"Save and load the scene."}, "save/load description was incorrect");

    physics_sim::TutorialProgress progress;
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::CameraPan, "tutorial did not start on the camera pan step");

    physics_sim::tutorial_mark_camera_panned(progress);
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::CameraZoom, "tutorial did not advance after camera pan");

    physics_sim::tutorial_mark_camera_zoomed(progress);
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::PauseResume, "tutorial did not advance after camera zoom");

    physics_sim::tutorial_mark_pause_opened(progress);
    physics_sim::tutorial_mark_pause_resumed(progress);
    physics_sim::tutorial_mark_wall_painted(progress);
    physics_sim::tutorial_mark_wall_erased(progress);
    physics_sim::tutorial_mark_fixture_placed(progress);
    physics_sim::tutorial_mark_device_used(progress);
    physics_sim::tutorial_mark_reset_or_retry(progress);
    physics_sim::tutorial_mark_save_or_load(progress);
    physics_sim::tutorial_mark_gallery_browsed(progress);

    REQUIRE(physics_sim::tutorial_is_complete(progress), "tutorial did not report completion after all milestones");
    REQUIRE(physics_sim::tutorial_current_step(progress) == TutorialStep::Complete, "tutorial did not report the complete step");

    physics_sim::tutorial_mark_camera_panned(progress);
    REQUIRE(physics_sim::tutorial_is_complete(progress), "tutorial completion regressed after duplicate progress updates");

    std::cout << "tutorial progress tests passed" << std::endl;
    return 0;
}
