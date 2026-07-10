#define SDL_MAIN_HANDLED

#include <physics_sim/player_feedback.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace
{
[[noreturn]] void fail(const char* message, const char* file, int line)
{
    std::fprintf(stderr, "FAIL %s:%d: %s\n", file, line, message);
    std::fflush(stderr);
    std::exit(1);
}

#define REQUIRE(condition, message) \
    do \
    { \
        if (!(condition)) \
        { \
            fail((message), __FILE__, __LINE__); \
        } \
    } while (false)
} // namespace

int main()
{
    namespace fs = std::filesystem;

    const fs::path temp_directory = fs::temp_directory_path() / "physics-sim-player-feedback-tests";
    fs::create_directories(temp_directory);

    const fs::path missing_scene = temp_directory / "missing.pscene";
    fs::remove(missing_scene);
    const auto missing_feedback = physics_sim::describe_scene_load_failure(missing_scene);
    REQUIRE(missing_feedback.recovery == physics_sim::PlayerRecoveryAction::BrowseSaves, "missing scene should offer the save browser as recovery");
    REQUIRE(missing_feedback.status_message.find("MISSING") != std::string::npos, "missing scene message was not readable");

    const fs::path unsupported_scene = temp_directory / "unsupported.pscene";
    {
        std::ofstream file(unsupported_scene, std::ios::trunc);
        file << "physics-sim-scene 1\n";
        file << "grid 2 2 1\n";
    }
    const auto unsupported_feedback = physics_sim::describe_scene_load_failure(unsupported_scene);
    REQUIRE(unsupported_feedback.recovery == physics_sim::PlayerRecoveryAction::BrowseSaves, "unsupported version should offer the save browser as recovery");
    REQUIRE(unsupported_feedback.status_message.find("VERSION") != std::string::npos, "unsupported version message was not readable");

    const fs::path malformed_scene = temp_directory / "malformed.pscene";
    {
        std::ofstream file(malformed_scene, std::ios::trunc);
        file << "this is not a physics sim scene\n";
    }
    const auto malformed_feedback = physics_sim::describe_scene_load_failure(malformed_scene);
    REQUIRE(malformed_feedback.recovery == physics_sim::PlayerRecoveryAction::BrowseSaves, "malformed scene should offer the save browser as recovery");
    REQUIRE(malformed_feedback.status_message.find("CORRUPT") != std::string::npos, "malformed scene message was not readable");

    const auto audio_feedback = physics_sim::describe_audio_failure("SDL_OpenAudioDevice failed");
    REQUIRE(audio_feedback.recovery == physics_sim::PlayerRecoveryAction::IgnoreAudio, "audio failure should be recoverable by ignoring audio");
    REQUIRE(audio_feedback.status_message.find("AUDIO") != std::string::npos, "audio failure message was not readable");

    const auto renderer_feedback = physics_sim::describe_renderer_failure("SDL_CreateRenderer failed");
    REQUIRE(renderer_feedback.recovery == physics_sim::PlayerRecoveryAction::Quit, "renderer failure should be fatal");
    REQUIRE(renderer_feedback.status_message.find("RENDERER") != std::string::npos, "renderer failure message was not readable");

    const auto settings_feedback = physics_sim::describe_settings_failure(temp_directory / "settings.txt");
    REQUIRE(settings_feedback.recovery == physics_sim::PlayerRecoveryAction::ResetSettings, "settings failure should suggest resetting preferences");
    REQUIRE(settings_feedback.status_message.find("SETTINGS") != std::string::npos, "settings failure message was not readable");

    const auto replay_feedback = physics_sim::describe_replay_failure("bad token");
    REQUIRE(replay_feedback.recovery == physics_sim::PlayerRecoveryAction::ReturnToMenu, "replay failure should return the player to the menu");
    REQUIRE(replay_feedback.status_message.find("REPLAY") != std::string::npos, "replay failure message was not readable");

    const auto package_feedback = physics_sim::describe_package_content_failure("demo scene missing");
    REQUIRE(package_feedback.recovery == physics_sim::PlayerRecoveryAction::Quit, "package-content failure should be fatal");
    REQUIRE(package_feedback.status_message.find("PACKAGE") != std::string::npos, "package-content failure message was not readable");

    fs::remove_all(temp_directory);

    std::cout << "player feedback tests passed" << std::endl;
    return 0;
}
