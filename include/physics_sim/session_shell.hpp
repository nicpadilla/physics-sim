#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>

namespace physics_sim
{
enum class SessionShellScreen
{
    Playing,
    MainMenu,
    SceneBrowser,
    SaveBrowser,
    Settings,
    About,
    PauseMenu,
};

enum class SessionShellCommandKind
{
    None,
    StartPlaying,
    StartTutorial,
    Quit,
    Resume,
    RetryCurrentScene,
    ResetFluid,
    ClearScene,
    SaveScene,
    LoadScene,
    OpenSaveBrowser,
    ReturnToMainMenu,
    OpenSceneBrowser,
    OpenSettings,
    OpenAbout,
    LoadSceneAtIndex,
    LoadSaveAtIndex,
    ToggleHelpOverlay,
    CycleVisualMode,
    Back,
};

struct SessionShellCommand
{
    SessionShellCommandKind kind = SessionShellCommandKind::None;
    std::optional<std::size_t> scene_index{};
};

struct SessionShellState
{
    SessionShellScreen screen = SessionShellScreen::MainMenu;
    SessionShellScreen return_screen = SessionShellScreen::MainMenu;
    std::size_t selection = 0;
};

[[nodiscard]] inline constexpr std::size_t session_shell_option_count(
    SessionShellScreen screen,
    std::size_t gallery_scene_count,
    std::size_t save_entry_count = 0) noexcept
{
    switch (screen)
    {
    case SessionShellScreen::Playing:
        return 0;
    case SessionShellScreen::MainMenu:
        return 6;
    case SessionShellScreen::SceneBrowser:
        return gallery_scene_count + 1;
    case SessionShellScreen::SaveBrowser:
        return save_entry_count;
    case SessionShellScreen::Settings:
        return 3;
    case SessionShellScreen::About:
        return 1;
    case SessionShellScreen::PauseMenu:
        return 8;
    }

    return 0;
}

[[nodiscard]] inline constexpr std::string_view session_shell_screen_title(SessionShellScreen screen) noexcept
{
    switch (screen)
    {
    case SessionShellScreen::Playing:
        return "PLAYING";
    case SessionShellScreen::MainMenu:
        return "MAIN MENU";
    case SessionShellScreen::SceneBrowser:
        return "SCENE BROWSER";
    case SessionShellScreen::SaveBrowser:
        return "LOAD SAVES";
    case SessionShellScreen::Settings:
        return "SETTINGS";
    case SessionShellScreen::About:
        return "ABOUT";
    case SessionShellScreen::PauseMenu:
        return "PAUSE MENU";
    }

    return "MENU";
}

[[nodiscard]] inline constexpr std::string_view session_shell_main_menu_label(std::size_t index) noexcept
{
    switch (index)
    {
    case 0:
        return "Continue Sandbox";
    case 1:
        return "Tutorial";
    case 2:
        return "Scene Browser";
    case 3:
        return "Settings";
    case 4:
        return "About";
    case 5:
        return "Quit";
    default:
        return "";
    }
}

[[nodiscard]] inline constexpr std::string_view session_shell_pause_menu_label(std::size_t index) noexcept
{
    switch (index)
    {
    case 0:
        return "Resume";
    case 1:
        return "Retry Current Scene";
    case 2:
        return "Reset Fluid";
    case 3:
        return "Clear Scene";
    case 4:
        return "Save Scene";
    case 5:
        return "Load Save";
    case 6:
        return "Settings";
    case 7:
        return "Return to Menu";
    default:
        return "";
    }
}

[[nodiscard]] inline constexpr std::string_view session_shell_scene_browser_label(std::size_t index, std::size_t gallery_scene_count) noexcept
{
    if (index < gallery_scene_count)
    {
        return "Scene";
    }

    return "Back";
}

[[nodiscard]] inline constexpr std::string_view session_shell_settings_label(std::size_t index) noexcept
{
    switch (index)
    {
    case 0:
        return "Toggle Help Overlay";
    case 1:
        return "Cycle Visual Mode";
    case 2:
        return "Back";
    default:
        return "";
    }
}

[[nodiscard]] inline constexpr std::string_view session_shell_about_label(std::size_t index) noexcept
{
    return index == 0 ? std::string_view{"Back"} : std::string_view{};
}

inline void session_shell_wrap_selection(SessionShellState& state, std::size_t item_count, int direction) noexcept
{
    if (item_count == 0 || direction == 0)
    {
        return;
    }

    const int step = direction > 0 ? 1 : -1;
    const int size = static_cast<int>(item_count);
    const int next_index = (static_cast<int>(state.selection) + step + size) % size;
    state.selection = static_cast<std::size_t>(next_index);
}

[[nodiscard]] inline SessionShellCommand session_shell_activate(
    SessionShellState& state,
    std::size_t gallery_scene_count,
    std::size_t save_entry_count = 0) noexcept
{
    const auto open_submenu = [&](SessionShellScreen screen) noexcept
    {
        state.return_screen = state.screen;
        state.screen = screen;
        state.selection = 0;
    };

    switch (state.screen)
    {
    case SessionShellScreen::Playing:
        return {};
    case SessionShellScreen::MainMenu:
        switch (state.selection)
        {
        case 0:
            state.screen = SessionShellScreen::Playing;
            state.selection = 0;
            return {SessionShellCommandKind::StartPlaying, std::nullopt};
        case 1:
            state.screen = SessionShellScreen::Playing;
            state.selection = 0;
            return {SessionShellCommandKind::StartTutorial, std::nullopt};
        case 2:
            open_submenu(SessionShellScreen::SceneBrowser);
            return {SessionShellCommandKind::OpenSceneBrowser, std::nullopt};
        case 3:
            open_submenu(SessionShellScreen::Settings);
            return {SessionShellCommandKind::OpenSettings, std::nullopt};
        case 4:
            open_submenu(SessionShellScreen::About);
            return {SessionShellCommandKind::OpenAbout, std::nullopt};
        case 5:
            return {SessionShellCommandKind::Quit, std::nullopt};
        default:
            return {};
        }
    case SessionShellScreen::SceneBrowser:
        if (state.selection < gallery_scene_count)
        {
            const std::size_t selected_scene_index = state.selection;
            state.screen = SessionShellScreen::Playing;
            state.selection = 0;
            return {SessionShellCommandKind::LoadSceneAtIndex, selected_scene_index};
        }

        state.screen = state.return_screen;
        state.selection = 0;
        return {SessionShellCommandKind::Back, std::nullopt};
    case SessionShellScreen::SaveBrowser:
        if (state.selection < save_entry_count)
        {
            const std::size_t selected_save_index = state.selection;
            state.screen = SessionShellScreen::Playing;
            state.selection = 0;
            return {SessionShellCommandKind::LoadSaveAtIndex, selected_save_index};
        }

        state.screen = state.return_screen;
        state.selection = 0;
        return {SessionShellCommandKind::Back, std::nullopt};
    case SessionShellScreen::Settings:
        switch (state.selection)
        {
        case 0:
            return {SessionShellCommandKind::ToggleHelpOverlay, std::nullopt};
        case 1:
            return {SessionShellCommandKind::CycleVisualMode, std::nullopt};
        case 2:
            state.screen = state.return_screen;
            state.selection = 0;
            return {SessionShellCommandKind::Back, std::nullopt};
        default:
            return {};
        }
    case SessionShellScreen::About:
        state.screen = state.return_screen;
        state.selection = 0;
        return {SessionShellCommandKind::Back, std::nullopt};
    case SessionShellScreen::PauseMenu:
        switch (state.selection)
        {
        case 0:
            state.screen = SessionShellScreen::Playing;
            state.selection = 0;
            return {SessionShellCommandKind::Resume, std::nullopt};
        case 1:
            return {SessionShellCommandKind::RetryCurrentScene, std::nullopt};
        case 2:
            return {SessionShellCommandKind::ResetFluid, std::nullopt};
        case 3:
            return {SessionShellCommandKind::ClearScene, std::nullopt};
        case 4:
            return {SessionShellCommandKind::SaveScene, std::nullopt};
        case 5:
            open_submenu(SessionShellScreen::SaveBrowser);
            return {SessionShellCommandKind::OpenSaveBrowser, std::nullopt};
        case 6:
            open_submenu(SessionShellScreen::Settings);
            return {SessionShellCommandKind::OpenSettings, std::nullopt};
        case 7:
            state.screen = SessionShellScreen::MainMenu;
            state.selection = 0;
            return {SessionShellCommandKind::ReturnToMainMenu, std::nullopt};
        default:
            return {};
        }
    }

    return {};
}

inline void session_shell_back(SessionShellState& state) noexcept
{
    if (state.screen == SessionShellScreen::SceneBrowser
        || state.screen == SessionShellScreen::SaveBrowser
        || state.screen == SessionShellScreen::Settings
        || state.screen == SessionShellScreen::About)
    {
        state.screen = state.return_screen;
        state.selection = 0;
    }
    else if (state.screen == SessionShellScreen::PauseMenu)
    {
        state.screen = SessionShellScreen::Playing;
        state.selection = 0;
    }
    else if (state.screen == SessionShellScreen::MainMenu)
    {
        state.screen = SessionShellScreen::Playing;
        state.selection = 0;
    }
}
} // namespace physics_sim
