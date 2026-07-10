#pragma once

#include <physics_sim/action.hpp>
#include <physics_sim/solver_profile.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace physics_sim
{
inline constexpr int ReplayScriptVersion = 2;

struct ReplayEvent
{
    std::uint64_t tick = 0;
    std::string command{};
    std::vector<std::string> arguments{};
};

struct ReplayScript
{
    std::string scene_digest{};
    double fixed_timestep = 0.0;
    FluidSolverProfile solver_profile = FluidSolverProfile::Balanced;
    std::optional<std::string> expected_final_digest{};
    std::vector<ReplayEvent> events{};
};

[[nodiscard]] inline bool replay_identity_matches(
    const ReplayScript& script,
    std::string_view scene_digest,
    double fixed_timestep,
    FluidSolverProfile profile,
    double epsilon = 1.0e-12) noexcept
{
    return script.scene_digest == scene_digest
        && std::abs(script.fixed_timestep - fixed_timestep) <= epsilon
        && script.solver_profile == profile;
}

[[nodiscard]] inline std::string stable_replay_source_digest(std::string_view source)
{
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char value : source)
    {
        if (value == '\r')
        {
            continue;
        }
        hash ^= value;
        hash *= 1099511628211ULL;
    }

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
}

[[nodiscard]] inline std::optional<std::string> stable_replay_file_digest(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return stable_replay_source_digest(buffer.str());
}

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
    bool saw_scene_digest = false;
    bool saw_fixed_timestep = false;
    bool saw_solver_profile = false;

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

        if (keyword == "scene-digest")
        {
            if (!(line_stream >> script.scene_digest) || script.scene_digest.size() != 16)
            {
                return std::nullopt;
            }
            saw_scene_digest = true;
            continue;
        }

        if (keyword == "fixed-timestep")
        {
            if (!(line_stream >> script.fixed_timestep) || script.fixed_timestep <= 0.0)
            {
                return std::nullopt;
            }
            saw_fixed_timestep = true;
            continue;
        }

        if (keyword == "solver-profile")
        {
            std::string profile;
            if (!(line_stream >> profile))
            {
                return std::nullopt;
            }
            const auto parsed = parse_solver_profile_token(profile);
            if (!parsed.has_value())
            {
                return std::nullopt;
            }
            script.solver_profile = *parsed;
            saw_solver_profile = true;
            continue;
        }

        if (keyword == "expected-final-digest")
        {
            std::string digest;
            if (!(line_stream >> digest) || digest.size() != 16)
            {
                return std::nullopt;
            }
            script.expected_final_digest = std::move(digest);
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

    if (!saw_header || !saw_scene_digest || !saw_fixed_timestep || !saw_solver_profile || script.events.empty())
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
