#include <physics_sim/session_shell.hpp>

#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string_view>

namespace
{
void fail(const char* message)
{
    std::cerr << message << std::endl;
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
}

int main()
{
    using physics_sim::SessionShellCommandKind;
    using physics_sim::SessionShellScreen;

    {
        physics_sim::SessionShellState state;
        REQUIRE(state.screen == SessionShellScreen::MainMenu, "session shell did not start on the main menu");
        REQUIRE(physics_sim::session_shell_option_count(state.screen, 6) == 5, "main menu option count was incorrect");
        REQUIRE(physics_sim::session_shell_main_menu_label(0) == std::string_view{"Continue Sandbox"}, "main menu label 0 was incorrect");
        REQUIRE(physics_sim::session_shell_screen_title(SessionShellScreen::PauseMenu) == std::string_view{"PAUSE MENU"}, "pause menu title was incorrect");

        physics_sim::session_shell_wrap_selection(state, 5, -1);
        REQUIRE(state.selection == 4, "main menu selection did not wrap upward");

        const auto quit = physics_sim::session_shell_activate(state, 6);
        REQUIRE(quit.kind == SessionShellCommandKind::Quit, "main menu quit did not map to the quit command");

        state.selection = 0;
        const auto start = physics_sim::session_shell_activate(state, 6);
        REQUIRE(start.kind == SessionShellCommandKind::StartPlaying, "main menu continue did not start playing");
        REQUIRE(state.screen == SessionShellScreen::Playing, "main menu continue did not enter playing state");
    }

    {
        physics_sim::SessionShellState state;
        state.selection = 1;
        const auto scene_browser = physics_sim::session_shell_activate(state, 6);
        REQUIRE(scene_browser.kind == SessionShellCommandKind::OpenSceneBrowser, "scene browser entry was not mapped correctly");
        REQUIRE(state.screen == SessionShellScreen::SceneBrowser, "scene browser entry did not open the browser screen");
        REQUIRE(state.return_screen == SessionShellScreen::MainMenu, "scene browser did not remember the return screen");

        physics_sim::session_shell_wrap_selection(state, 7, -1);
        REQUIRE(state.selection == 6, "scene browser selection did not wrap to back");
        state.selection = 2;
        const auto scene_load = physics_sim::session_shell_activate(state, 6);
        REQUIRE(scene_load.kind == SessionShellCommandKind::LoadSceneAtIndex, "scene browser load did not map to a scene command");
        REQUIRE(scene_load.scene_index.has_value() && *scene_load.scene_index == 2, "scene browser load did not preserve the selected scene index");
        REQUIRE(state.screen == SessionShellScreen::Playing, "scene browser load did not return to playing");

        state.screen = SessionShellScreen::SceneBrowser;
        state.return_screen = SessionShellScreen::PauseMenu;
        state.selection = 6;
        const auto back = physics_sim::session_shell_activate(state, 6);
        REQUIRE(back.kind == SessionShellCommandKind::Back, "scene browser back was not mapped correctly");
        REQUIRE(state.screen == SessionShellScreen::PauseMenu, "scene browser back did not restore the return screen");
    }

    {
        physics_sim::SessionShellState state;
        state.screen = SessionShellScreen::SceneBrowser;
        state.return_screen = SessionShellScreen::MainMenu;
        state.selection = 3;

        physics_sim::session_shell_back(state);
        REQUIRE(state.screen == SessionShellScreen::MainMenu, "scene browser back did not restore the return screen");
        REQUIRE(state.selection == 0, "scene browser back did not reset the selection");
    }

    {
        physics_sim::SessionShellState state;
        state.screen = SessionShellScreen::PauseMenu;
        state.selection = 0;

        const auto resume = physics_sim::session_shell_activate(state, 6);
        REQUIRE(resume.kind == SessionShellCommandKind::Resume, "pause menu resume did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::Playing, "pause menu resume did not return to playing");

        state.screen = SessionShellScreen::PauseMenu;
        state.selection = 6;
        const auto settings = physics_sim::session_shell_activate(state, 6);
        REQUIRE(settings.kind == SessionShellCommandKind::OpenSettings, "pause menu settings did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::Settings, "pause menu settings did not open the settings screen");
        REQUIRE(state.return_screen == SessionShellScreen::PauseMenu, "pause menu settings did not remember the return screen");

        state.selection = 7;
        state.screen = SessionShellScreen::PauseMenu;
        const auto return_to_menu = physics_sim::session_shell_activate(state, 6);
        REQUIRE(return_to_menu.kind == SessionShellCommandKind::ReturnToMainMenu, "pause menu return-to-menu did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::MainMenu, "pause menu return-to-menu did not return to the main menu");
    }

    {
        physics_sim::SessionShellState state;
        state.screen = SessionShellScreen::Settings;
        state.return_screen = SessionShellScreen::MainMenu;

        REQUIRE(physics_sim::session_shell_settings_label(0) == std::string_view{"Toggle Help Overlay"}, "settings label 0 was incorrect");
        physics_sim::session_shell_wrap_selection(state, 3, 1);
        REQUIRE(state.selection == 1, "settings selection did not wrap forward");

        state.selection = 0;
        auto command = physics_sim::session_shell_activate(state, 6);
        REQUIRE(command.kind == SessionShellCommandKind::ToggleHelpOverlay, "settings toggle did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::Settings, "settings toggle should stay in the settings screen");

        state.selection = 1;
        command = physics_sim::session_shell_activate(state, 6);
        REQUIRE(command.kind == SessionShellCommandKind::CycleVisualMode, "settings visual mode did not map correctly");

        state.selection = 2;
        command = physics_sim::session_shell_activate(state, 6);
        REQUIRE(command.kind == SessionShellCommandKind::Back, "settings back did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::MainMenu, "settings back did not return to the main menu");
    }

    {
        physics_sim::SessionShellState state;
        state.screen = SessionShellScreen::About;
        state.return_screen = SessionShellScreen::PauseMenu;
        REQUIRE(physics_sim::session_shell_about_label(0) == std::string_view{"Back"}, "about label was incorrect");

        const auto command = physics_sim::session_shell_activate(state, 6);
        REQUIRE(command.kind == SessionShellCommandKind::Back, "about screen back did not map correctly");
        REQUIRE(state.screen == SessionShellScreen::PauseMenu, "about screen back did not return to the prior screen");
    }

    std::cout << "session shell tests passed" << std::endl;
    return 0;
}
