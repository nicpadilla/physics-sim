#pragma once

#include <physics_sim/action.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace physics_sim
{
inline constexpr int ReplayScriptVersion = 1;

struct ReplayEvent
{
    std::uint64_t tick = 0;
    std::string command{};
    std::vector<std::string> arguments{};
};

struct ReplayScript
{
    std::vector<ReplayEvent> events{};
};

[[nodiscard]] inline const char* action_name(Action action) noexcept
{
    switch (action)
    {
    case Action::Quit:
        return "quit";
    case Action::TogglePause:
        return "toggle-pause";
    case Action::StepOnce:
        return "step-once";
    case Action::Reset:
        return "reset";
    }

    return "unknown";
}

[[nodiscard]] inline std::optional<Action> action_from_replay_token(std::string_view token) noexcept
{
    if (token == "quit")
    {
        return Action::Quit;
    }

    if (token == "toggle-pause")
    {
        return Action::TogglePause;
    }

    if (token == "step-once")
    {
        return Action::StepOnce;
    }

    if (token == "reset")
    {
        return Action::Reset;
    }

    return std::nullopt;
}

[[nodiscard]] inline std::optional<ReplayScript> parse_replay_script_text(std::string_view text)
{
    std::string text_copy{text};
    std::istringstream stream{text_copy};
    ReplayScript script;

    bool saw_header = false;

    auto trim = [](std::string& line)
    {
        const auto is_not_space = [](unsigned char ch) { return !std::isspace(ch); };
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), is_not_space));
        line.erase(std::find_if(line.rbegin(), line.rend(), is_not_space).base(), line.end());
    };

    std::string line;
    while (std::getline(stream, line))
    {
        trim(line);
        if (line.empty() || line.front() == '#')
        {
            continue;
        }

        std::istringstream line_stream(line);
        std::string keyword;
        line_stream >> keyword;
        if (!line_stream)
        {
            return std::nullopt;
        }

        if (keyword == "physics-sim-replay")
        {
            int version = 0;
            if (!(line_stream >> version) || version != ReplayScriptVersion)
            {
                return std::nullopt;
            }

            saw_header = true;
            continue;
        }

        if (keyword == "tick")
        {
            std::uint64_t tick = 0;
            std::string command;
            if (!(line_stream >> tick >> command))
            {
                return std::nullopt;
            }

            std::vector<std::string> arguments;
            std::string token;
            while (line_stream >> token)
            {
                arguments.push_back(token);
            }

            if (command == "action")
            {
                if (arguments.size() != 1 || !action_from_replay_token(arguments.front()).has_value())
                {
                    return std::nullopt;
                }
            }
            else if (command == "tool")
            {
                if (arguments.size() != 1)
                {
                    return std::nullopt;
                }
            }
            else if (command == "direction")
            {
                if (arguments.size() != 2)
                {
                    return std::nullopt;
                }
            }
            else if (command == "speed" || command == "rate")
            {
                if (arguments.size() != 1)
                {
                    return std::nullopt;
                }
            }
            else if (command == "place")
            {
                if (arguments.size() != 2)
                {
                    return std::nullopt;
                }
            }
            else
            {
                return std::nullopt;
            }

            script.events.push_back(ReplayEvent{tick, std::move(command), std::move(arguments)});
            continue;
        }

        return std::nullopt;
    }

    if (!saw_header || script.events.empty())
    {
        return std::nullopt;
    }

    std::stable_sort(script.events.begin(), script.events.end(), [](const ReplayEvent& lhs, const ReplayEvent& rhs)
    {
        return lhs.tick < rhs.tick;
    });

    return script;
}

[[nodiscard]] inline std::optional<ReplayScript> load_replay_script(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse_replay_script_text(buffer.str());
}
} // namespace physics_sim
