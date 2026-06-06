#pragma once

#include <physics_sim/scene_document.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace physics_sim
{
enum class PlayerRecoveryAction
{
    None,
    Retry,
    OpenDefaultScene,
    OpenAutosave,
    BrowseSaves,
    IgnoreAudio,
    ResetSettings,
    ReturnToMenu,
    Quit,
};

struct PlayerFeedback
{
    std::string status_message{};
    std::string detail{};
    PlayerRecoveryAction recovery = PlayerRecoveryAction::None;
};

[[nodiscard]] inline constexpr std::string_view player_recovery_action_name(PlayerRecoveryAction action) noexcept
{
    switch (action)
    {
    case PlayerRecoveryAction::None:
        return "none";
    case PlayerRecoveryAction::Retry:
        return "retry";
    case PlayerRecoveryAction::OpenDefaultScene:
        return "open-default-scene";
    case PlayerRecoveryAction::OpenAutosave:
        return "open-autosave";
    case PlayerRecoveryAction::BrowseSaves:
        return "browse-saves";
    case PlayerRecoveryAction::IgnoreAudio:
        return "ignore-audio";
    case PlayerRecoveryAction::ResetSettings:
        return "reset-settings";
    case PlayerRecoveryAction::ReturnToMenu:
        return "return-to-menu";
    case PlayerRecoveryAction::Quit:
        return "quit";
    }

    return "unknown";
}

namespace detail
{
[[nodiscard]] inline std::optional<std::string> read_first_content_line(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::string line;
    while (std::getline(file, line))
    {
        const auto is_not_space = [](unsigned char ch) { return !std::isspace(ch); };
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), is_not_space));
        line.erase(std::find_if(line.rbegin(), line.rend(), is_not_space).base(), line.end());
        if (!line.empty() && line.front() != '#')
        {
            return line;
        }
    }

    return std::nullopt;
}

[[nodiscard]] inline std::optional<int> parse_scene_header_version(std::string_view line)
{
    std::istringstream stream{std::string{line}};
    std::string keyword;
    int version = 0;
    if (!(stream >> keyword >> version) || keyword != "physics-sim-scene")
    {
        return std::nullopt;
    }

    return version;
}

[[nodiscard]] inline std::string path_detail(const std::filesystem::path& path, std::string_view reason)
{
    std::ostringstream stream;
    stream << reason << ": " << path.string();
    return stream.str();
}
} // namespace detail

[[nodiscard]] inline PlayerFeedback describe_scene_load_failure(
    const std::filesystem::path& path,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::BrowseSaves)
{
    if (!std::filesystem::exists(path))
    {
        return {
            "SCENE FILE MISSING",
            detail::path_detail(path, "scene file missing"),
            recovery,
        };
    }

    const auto first_content_line = detail::read_first_content_line(path);
    if (!first_content_line.has_value())
    {
        return {
            "SCENE FILE CORRUPT",
            detail::path_detail(path, "scene file empty or unreadable"),
            recovery,
        };
    }

    const auto version = detail::parse_scene_header_version(*first_content_line);
    if (!version.has_value())
    {
        return {
            "SCENE FILE CORRUPT",
            detail::path_detail(path, "scene header malformed"),
            recovery,
        };
    }

    if (*version < 1 || *version > SceneFormatVersion)
    {
        return {
            "SCENE VERSION NOT SUPPORTED",
            detail::path_detail(path, "scene version mismatch"),
            recovery,
        };
    }

    return {
        "SCENE FILE CORRUPT",
        detail::path_detail(path, "scene body malformed"),
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_scene_save_failure(
    const std::filesystem::path& path,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::Retry)
{
    return {
        "SAVE FAILED",
        detail::path_detail(path, "scene save failed"),
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_settings_failure(
    const std::filesystem::path& path,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::ResetSettings)
{
    return {
        "SETTINGS SAVE FAILED",
        detail::path_detail(path, "settings save failed"),
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_audio_failure(
    std::string_view reason,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::IgnoreAudio)
{
    return {
        "AUDIO UNAVAILABLE",
        std::string{"audio unavailable: "} + std::string{reason},
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_renderer_failure(
    std::string_view reason,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::Quit)
{
    return {
        "RENDERER UNAVAILABLE",
        std::string{"renderer unavailable: "} + std::string{reason},
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_replay_failure(
    std::string_view reason,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::ReturnToMenu)
{
    return {
        "REPLAY FAILED",
        std::string{"replay failure: "} + std::string{reason},
        recovery,
    };
}

[[nodiscard]] inline PlayerFeedback describe_package_content_failure(
    std::string_view reason,
    PlayerRecoveryAction recovery = PlayerRecoveryAction::Quit)
{
    return {
        "PACKAGE CONTENT MISSING",
        std::string{"package content missing: "} + std::string{reason},
        recovery,
    };
}
} // namespace physics_sim
