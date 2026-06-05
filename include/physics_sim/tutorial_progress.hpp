#pragma once

#include <cstddef>
#include <string_view>

namespace physics_sim
{
enum class TutorialStep
{
    CameraPan,
    CameraZoom,
    PauseResume,
    PaintWall,
    EraseWall,
    PlaceFixture,
    UseDevice,
    ResetOrRetry,
    SaveLoad,
    GalleryBrowse,
    Complete,
};

struct TutorialProgress
{
    bool camera_panned = false;
    bool camera_zoomed = false;
    bool pause_opened = false;
    bool pause_resumed = false;
    bool wall_painted = false;
    bool wall_erased = false;
    bool fixture_placed = false;
    bool device_used = false;
    bool reset_or_retry = false;
    bool save_or_load = false;
    bool gallery_browsed = false;
};

[[nodiscard]] inline constexpr std::size_t tutorial_step_count() noexcept
{
    return 10;
}

[[nodiscard]] inline constexpr TutorialStep tutorial_current_step(const TutorialProgress& progress) noexcept
{
    if (!progress.camera_panned)
    {
        return TutorialStep::CameraPan;
    }

    if (!progress.camera_zoomed)
    {
        return TutorialStep::CameraZoom;
    }

    if (!(progress.pause_opened && progress.pause_resumed))
    {
        return TutorialStep::PauseResume;
    }

    if (!progress.wall_painted)
    {
        return TutorialStep::PaintWall;
    }

    if (!progress.wall_erased)
    {
        return TutorialStep::EraseWall;
    }

    if (!progress.fixture_placed)
    {
        return TutorialStep::PlaceFixture;
    }

    if (!progress.device_used)
    {
        return TutorialStep::UseDevice;
    }

    if (!progress.reset_or_retry)
    {
        return TutorialStep::ResetOrRetry;
    }

    if (!progress.save_or_load)
    {
        return TutorialStep::SaveLoad;
    }

    if (!progress.gallery_browsed)
    {
        return TutorialStep::GalleryBrowse;
    }

    return TutorialStep::Complete;
}

[[nodiscard]] inline constexpr bool tutorial_is_complete(const TutorialProgress& progress) noexcept
{
    return tutorial_current_step(progress) == TutorialStep::Complete;
}

[[nodiscard]] inline constexpr std::string_view tutorial_step_title(TutorialStep step) noexcept
{
    switch (step)
    {
    case TutorialStep::CameraPan:
        return "PAN CAMERA";
    case TutorialStep::CameraZoom:
        return "ZOOM CAMERA";
    case TutorialStep::PauseResume:
        return "PAUSE, THEN RESUME";
    case TutorialStep::PaintWall:
        return "PAINT A WALL";
    case TutorialStep::EraseWall:
        return "ERASE PART OF IT";
    case TutorialStep::PlaceFixture:
        return "PLACE A FIXTURE";
    case TutorialStep::UseDevice:
        return "USE A DEVICE";
    case TutorialStep::ResetOrRetry:
        return "RESET OR RETRY";
    case TutorialStep::SaveLoad:
        return "SAVE AND LOAD THE SCENE.";
    case TutorialStep::GalleryBrowse:
        return "BROWSE THE GALLERY.";
    case TutorialStep::Complete:
        return "TUTORIAL COMPLETE";
    }

    return "TUTORIAL";
}

[[nodiscard]] inline constexpr std::string_view tutorial_step_description(TutorialStep step) noexcept
{
    switch (step)
    {
    case TutorialStep::CameraPan:
        return "Pan the camera.";
    case TutorialStep::CameraZoom:
        return "Zoom the camera.";
    case TutorialStep::PauseResume:
        return "Pause and resume the session.";
    case TutorialStep::PaintWall:
        return "Paint a wall.";
    case TutorialStep::EraseWall:
        return "Erase part of the wall.";
    case TutorialStep::PlaceFixture:
        return "Place a fixture.";
    case TutorialStep::UseDevice:
        return "Use a device.";
    case TutorialStep::ResetOrRetry:
        return "Reset or retry the scene.";
    case TutorialStep::SaveLoad:
        return "Save and load the scene.";
    case TutorialStep::GalleryBrowse:
        return "Browse the gallery.";
    case TutorialStep::Complete:
        return "You finished the guided tutorial.";
    }

    return "Learn the sandbox loop.";
}

inline constexpr void tutorial_mark_camera_panned(TutorialProgress& progress) noexcept
{
    progress.camera_panned = true;
}

inline constexpr void tutorial_mark_camera_zoomed(TutorialProgress& progress) noexcept
{
    progress.camera_zoomed = true;
}

inline constexpr void tutorial_mark_pause_opened(TutorialProgress& progress) noexcept
{
    progress.pause_opened = true;
}

inline constexpr void tutorial_mark_pause_resumed(TutorialProgress& progress) noexcept
{
    progress.pause_resumed = true;
}

inline constexpr void tutorial_mark_wall_painted(TutorialProgress& progress) noexcept
{
    progress.wall_painted = true;
}

inline constexpr void tutorial_mark_wall_erased(TutorialProgress& progress) noexcept
{
    progress.wall_erased = true;
}

inline constexpr void tutorial_mark_fixture_placed(TutorialProgress& progress) noexcept
{
    progress.fixture_placed = true;
}

inline constexpr void tutorial_mark_device_used(TutorialProgress& progress) noexcept
{
    progress.device_used = true;
}

inline constexpr void tutorial_mark_reset_or_retry(TutorialProgress& progress) noexcept
{
    progress.reset_or_retry = true;
}

inline constexpr void tutorial_mark_save_or_load(TutorialProgress& progress) noexcept
{
    progress.save_or_load = true;
}

inline constexpr void tutorial_mark_gallery_browsed(TutorialProgress& progress) noexcept
{
    progress.gallery_browsed = true;
}
} // namespace physics_sim
