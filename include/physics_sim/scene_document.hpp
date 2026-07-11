#pragma once

#include <physics_sim/math.hpp>
#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace physics_sim
{
struct SceneChallenge
{
    std::string title{};
    std::size_t required_objective_sensors = 1;
    std::uint64_t hold_ticks = 1;
    std::optional<double> maximum_emitted_mass{};
    std::optional<double> maximum_outflow_mass{};
};

struct SceneMetadata
{
    std::string title{};
    std::string description{};
    std::string author{};
    std::vector<std::string> tags{};
    std::vector<std::string> notes{};
    std::optional<SceneChallenge> challenge{};
};

struct SceneEmitter
{
    WaterEmitterKind kind = WaterEmitterKind::Directional;
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float speed = 6.0f;
    float emission_rate = 0.0f;
    bool enabled = true;
};

struct SceneGate
{
    std::size_t x = 0;
    std::size_t y = 0;
    bool open = false;
};

struct SceneSensor
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
    bool active = false;
    bool objective = false;
    std::string label{};
};

struct SceneDrain
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
};

struct ScenePump
{
    std::size_t x = 0;
    std::size_t y = 0;
    std::size_t width = 1;
    std::size_t height = 1;
    bool enabled = true;
    Vec2 direction{0.0f, 1.0f};
    float strength = 8.0f;
};

struct SceneValve
{
    std::size_t x = 0;
    std::size_t y = 0;
    bool open = false;
};

struct SceneCell
{
    std::size_t x = 0;
    std::size_t y = 0;
};

struct SceneDocument
{
    std::size_t grid_width = 0;
    std::size_t grid_height = 0;
    float cell_size = 1.0f;
    std::optional<FluidSolverProfile> solver_profile{};
    SceneMetadata metadata{};
    std::vector<SceneCell> solid_cells{};
    std::vector<SceneEmitter> emitters{};
    std::vector<SceneGate> gates{};
    std::vector<SceneSensor> sensors{};
    std::vector<SceneDrain> drains{};
    std::vector<ScenePump> pumps{};
    std::vector<SceneValve> valves{};
};

// Recovery scene v2 is intentionally incompatible with the pre-recovery v1 format.
inline constexpr int SceneFormatVersion = 2;

[[nodiscard]] inline SceneDocument capture_scene(const WaterSimulation2D& simulation, SceneMetadata metadata = {})
{
    SceneDocument document;
    document.grid_width = simulation.grid().width();
    document.grid_height = simulation.grid().height();
    document.cell_size = simulation.grid().cell_size();
    document.solver_profile = simulation.solver_settings().profile;
    document.metadata = std::move(metadata);

    for (std::size_t y = 0; y < simulation.grid().height(); ++y)
    {
        for (std::size_t x = 0; x < simulation.grid().width(); ++x)
        {
            if (simulation.grid().solid(x, y))
            {
                document.solid_cells.push_back({x, y});
            }
        }
    }

    document.emitters.reserve(simulation.emitters().size());
    for (const auto& emitter : simulation.emitters())
    {
        document.emitters.push_back(SceneEmitter{
            emitter.kind,
            emitter.position,
            emitter.direction,
            emitter.speed,
            emitter.emission_rate,
            emitter.enabled,
        });
    }

    document.gates.reserve(simulation.gates().size());
    for (const auto& gate : simulation.gates())
    {
        document.gates.push_back(SceneGate{
            gate.x,
            gate.y,
            gate.open,
        });
    }

    document.sensors.reserve(simulation.sensors().size());
    for (const auto& sensor : simulation.sensors())
    {
        document.sensors.push_back(SceneSensor{
            sensor.x,
            sensor.y,
            sensor.width,
            sensor.height,
            sensor.enabled,
            sensor.active,
            sensor.objective,
            sensor.label,
        });
    }

    document.drains.reserve(simulation.drains().size());
    for (const auto& drain : simulation.drains())
    {
        document.drains.push_back(SceneDrain{
            drain.x,
            drain.y,
            drain.width,
            drain.height,
            drain.enabled,
        });
    }

    document.pumps.reserve(simulation.pumps().size());
    for (const auto& pump : simulation.pumps())
    {
        document.pumps.push_back(ScenePump{
            pump.x,
            pump.y,
            pump.width,
            pump.height,
            pump.enabled,
            pump.direction,
            pump.strength,
        });
    }

    document.valves.reserve(simulation.valves().size());
    for (const auto& valve : simulation.valves())
    {
        document.valves.push_back(SceneValve{
            valve.x,
            valve.y,
            valve.open,
        });
    }

    return document;
}

[[nodiscard]] inline FluidSolverProfile effective_scene_solver_profile(
    const SceneDocument& document,
    FluidSolverProfile fallback_profile = FluidSolverProfile::Balanced,
    std::optional<FluidSolverProfile> forced_profile = std::nullopt) noexcept
{
    if (forced_profile.has_value())
    {
        return *forced_profile;
    }

    if (document.solver_profile.has_value())
    {
        return *document.solver_profile;
    }

    return fallback_profile;
}

inline void apply_scene(
    const SceneDocument& document,
    WaterSimulation2D& simulation,
    FluidSolverProfile fallback_profile = FluidSolverProfile::Balanced,
    std::optional<FluidSolverProfile> forced_profile = std::nullopt)
{
    const FluidSolverProfile profile = effective_scene_solver_profile(document, fallback_profile, forced_profile);
    simulation.resize(document.grid_width, document.grid_height, document.cell_size);
    simulation.set_solver_settings(WaterSimulation2D::solver_settings_for_profile(profile));

    for (const auto& cell : document.solid_cells)
    {
        simulation.set_solid_cell(cell.x, cell.y, true);
    }

    for (const auto& emitter : document.emitters)
    {
        WaterEmitter runtime_emitter;
        runtime_emitter.kind = emitter.kind;
        runtime_emitter.position = emitter.position;
        runtime_emitter.direction = emitter.direction;
        runtime_emitter.speed = emitter.speed;
        runtime_emitter.emission_rate = emitter.emission_rate;
        runtime_emitter.enabled = emitter.enabled;
        simulation.add_emitter(runtime_emitter);
    }

    for (const auto& gate : document.gates)
    {
        simulation.add_gate(WaterGate{gate.x, gate.y, gate.open});
    }

    for (const auto& sensor : document.sensors)
    {
        simulation.add_sensor(WaterSensor{
            sensor.x,
            sensor.y,
            sensor.width,
            sensor.height,
            sensor.enabled,
            sensor.active,
            sensor.objective,
            sensor.label,
        });
    }

    for (const auto& drain : document.drains)
    {
        simulation.add_drain(WaterDrain{
            drain.x,
            drain.y,
            drain.width,
            drain.height,
            drain.enabled,
        });
    }

    for (const auto& pump : document.pumps)
    {
        simulation.add_pump(WaterPump{
            pump.x,
            pump.y,
            pump.width,
            pump.height,
            pump.enabled,
            pump.direction,
            pump.strength,
        });
    }

    for (const auto& valve : document.valves)
    {
        simulation.add_valve(WaterValve{
            valve.x,
            valve.y,
            valve.open,
        });
    }
}

[[nodiscard]] inline std::optional<SceneDocument> parse_scene_text(std::string_view text)
{
    std::string text_copy{text};
    std::istringstream stream{text_copy};
    SceneDocument document;

    bool saw_header = false;
    bool saw_grid = false;
    int file_version = 0;

    auto trim = [](std::string& line)
    {
        const auto is_not_space = [](unsigned char ch) { return !std::isspace(ch); };
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), is_not_space));
        line.erase(std::find_if(line.rbegin(), line.rend(), is_not_space).base(), line.end());
    };

    const auto read_value = [&trim](std::istringstream& line_stream) -> std::string
    {
        std::string value;
        std::getline(line_stream, value);
        trim(value);
        return value;
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

        if (keyword == "physics-sim-scene")
        {
            int version = 0;
            if (saw_header || !(line_stream >> version) || version != SceneFormatVersion)
            {
                return std::nullopt;
            }
            file_version = version;
            saw_header = true;
            continue;
        }

        if (keyword == "solver-profile" || keyword == "solver_profile")
        {
            std::string token;
            if (!(line_stream >> token))
            {
                return std::nullopt;
            }

            const auto profile = parse_solver_profile_token(token);
            if (!profile.has_value())
            {
                return std::nullopt;
            }

            document.solver_profile = *profile;
            continue;
        }

        if (keyword == "title")
        {
            document.metadata.title = read_value(line_stream);
            continue;
        }

        if (keyword == "description")
        {
            document.metadata.description = read_value(line_stream);
            continue;
        }

        if (keyword == "author")
        {
            document.metadata.author = read_value(line_stream);
            continue;
        }

        if (keyword == "tag")
        {
            const std::string value = read_value(line_stream);
            if (value.empty())
            {
                return std::nullopt;
            }

            document.metadata.tags.push_back(value);
            continue;
        }

        if (keyword == "note")
        {
            const std::string value = read_value(line_stream);
            if (value.empty())
            {
                return std::nullopt;
            }

            document.metadata.notes.push_back(value);
            continue;
        }

        if (keyword == "grid")
        {
            if (!(line_stream >> document.grid_width >> document.grid_height >> document.cell_size))
            {
                return std::nullopt;
            }
            if (document.grid_width == 0 || document.grid_height == 0 || document.cell_size <= 0.0f)
            {
                return std::nullopt;
            }
            saw_grid = true;
            continue;
        }

        if (keyword == "wall")
        {
            SceneCell cell{};
            if (!(line_stream >> cell.x >> cell.y))
            {
                return std::nullopt;
            }
            document.solid_cells.push_back(cell);
            continue;
        }

        if (keyword == "emitter")
        {
            std::string kind_token;
            SceneEmitter emitter;
            int enabled_value = 1;

            if (!(line_stream >> kind_token >> emitter.position.x >> emitter.position.y >> emitter.direction.x >> emitter.direction.y >> emitter.speed >> emitter.emission_rate >> enabled_value))
            {
                return std::nullopt;
            }

            if (kind_token == "directional")
            {
                emitter.kind = WaterEmitterKind::Directional;
            }
            else if (kind_token == "omni")
            {
                emitter.kind = WaterEmitterKind::Omni;
            }
            else
            {
                return std::nullopt;
            }

            emitter.enabled = enabled_value != 0;
            document.emitters.push_back(emitter);
            continue;
        }

        if (keyword == "gate")
        {
            SceneGate gate;
            int open_value = 0;
            if (!(line_stream >> gate.x >> gate.y >> open_value))
            {
                return std::nullopt;
            }

            gate.open = open_value != 0;
            document.gates.push_back(gate);
            continue;
        }

        if (keyword == "sensor")
        {
            SceneSensor sensor;
            int enabled_value = 1;
            int active_value = 0;
            int objective_value = 0;

            if (!(line_stream >> sensor.x >> sensor.y >> sensor.width >> sensor.height >> enabled_value >> active_value >> objective_value))
            {
                return std::nullopt;
            }
            if (sensor.width == 0 || sensor.height == 0)
            {
                return std::nullopt;
            }

            sensor.enabled = enabled_value != 0;
            sensor.active = active_value != 0;
            sensor.objective = objective_value != 0;
            sensor.label = read_value(line_stream);
            document.sensors.push_back(sensor);
            continue;
        }

        if (keyword == "challenge")
        {
            SceneChallenge challenge;
            double maximum_emitted = -1.0;
            double maximum_outflow = -1.0;
            if (!(line_stream >> challenge.required_objective_sensors >> challenge.hold_ticks >> maximum_emitted >> maximum_outflow))
            {
                return std::nullopt;
            }
            challenge.title = read_value(line_stream);
            if (challenge.required_objective_sensors == 0 || challenge.hold_ticks == 0 || challenge.title.empty()
                || !std::isfinite(maximum_emitted) || !std::isfinite(maximum_outflow)
                || maximum_emitted < -1.0 || maximum_outflow < -1.0)
            {
                return std::nullopt;
            }
            if (maximum_emitted >= 0.0) challenge.maximum_emitted_mass = maximum_emitted;
            if (maximum_outflow >= 0.0) challenge.maximum_outflow_mass = maximum_outflow;
            document.metadata.challenge = std::move(challenge);
            continue;
        }

        if (keyword == "drain")
        {
            SceneDrain drain;
            int enabled_value = 1;
            if (!(line_stream >> drain.x >> drain.y >> drain.width >> drain.height >> enabled_value))
            {
                return std::nullopt;
            }

            if (drain.width == 0 || drain.height == 0)
            {
                return std::nullopt;
            }

            drain.enabled = enabled_value != 0;
            document.drains.push_back(drain);
            continue;
        }

        if (keyword == "pump")
        {
            ScenePump pump;
            int enabled_value = 1;
            if (!(line_stream >> pump.x >> pump.y >> pump.width >> pump.height >> enabled_value >> pump.direction.x >> pump.direction.y >> pump.strength))
            {
                return std::nullopt;
            }

            if (pump.width == 0 || pump.height == 0 || pump.strength < 0.0f)
            {
                return std::nullopt;
            }

            pump.enabled = enabled_value != 0;
            document.pumps.push_back(pump);
            continue;
        }

        if (keyword == "valve")
        {
            SceneValve valve;
            int open_value = 0;
            if (!(line_stream >> valve.x >> valve.y >> open_value))
            {
                return std::nullopt;
            }

            valve.open = open_value != 0;
            document.valves.push_back(valve);
            continue;
        }

        return std::nullopt;
    }

    if (!saw_header || !saw_grid || file_version != SceneFormatVersion || !document.solver_profile.has_value())
    {
        return std::nullopt;
    }

    const auto region_valid = [&](std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        return x < document.grid_width && y < document.grid_height
            && width > 0 && height > 0
            && width <= document.grid_width - x && height <= document.grid_height - y;
    };
    for (const auto& cell : document.solid_cells) if (!region_valid(cell.x, cell.y, 1, 1)) return std::nullopt;
    for (const auto& gate : document.gates) if (!region_valid(gate.x, gate.y, 1, 1)) return std::nullopt;
    for (const auto& sensor : document.sensors) if (!region_valid(sensor.x, sensor.y, sensor.width, sensor.height)) return std::nullopt;
    for (const auto& drain : document.drains) if (!region_valid(drain.x, drain.y, drain.width, drain.height)) return std::nullopt;
    for (const auto& pump : document.pumps)
    {
        if (!region_valid(pump.x, pump.y, pump.width, pump.height) || !std::isfinite(pump.strength)
            || !std::isfinite(pump.direction.x) || !std::isfinite(pump.direction.y)) return std::nullopt;
    }
    for (const auto& valve : document.valves) if (!region_valid(valve.x, valve.y, 1, 1)) return std::nullopt;
    const float world_width = static_cast<float>(document.grid_width) * document.cell_size;
    const float world_height = static_cast<float>(document.grid_height) * document.cell_size;
    for (const auto& emitter : document.emitters)
    {
        if (!std::isfinite(emitter.position.x) || !std::isfinite(emitter.position.y)
            || !std::isfinite(emitter.direction.x) || !std::isfinite(emitter.direction.y)
            || !std::isfinite(emitter.speed) || !std::isfinite(emitter.emission_rate)
            || emitter.position.x < 0.0f || emitter.position.y < 0.0f
            || emitter.position.x >= world_width || emitter.position.y >= world_height
            || emitter.speed < 0.0f || emitter.emission_rate < 0.0f) return std::nullopt;
    }
    if (document.metadata.challenge.has_value())
    {
        const auto objective_count = static_cast<std::size_t>(std::count_if(
            document.sensors.begin(), document.sensors.end(), [](const SceneSensor& sensor) { return sensor.objective && sensor.enabled; }));
        if (objective_count < document.metadata.challenge->required_objective_sensors) return std::nullopt;
    }

    return document;
}

[[nodiscard]] inline std::optional<SceneDocument> load_scene(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parse_scene_text(buffer.str());
}

[[nodiscard]] inline bool save_scene(const std::filesystem::path& path, const SceneDocument& document)
{
    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    std::filesystem::path temporary_path = path;
    temporary_path += ".tmp";
    std::filesystem::path backup_path = path;
    backup_path += ".bak";

    std::error_code cleanup_error;
    std::filesystem::remove(temporary_path, cleanup_error);
    std::ofstream file(temporary_path, std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }

    file << "physics-sim-scene " << SceneFormatVersion << "\n";
    if (document.solver_profile.has_value())
    {
        file << "solver-profile " << solver_profile_name(*document.solver_profile) << "\n";
    }
    if (!document.metadata.title.empty())
    {
        file << "title " << document.metadata.title << "\n";
    }
    if (!document.metadata.description.empty())
    {
        file << "description " << document.metadata.description << "\n";
    }
    if (!document.metadata.author.empty())
    {
        file << "author " << document.metadata.author << "\n";
    }
    for (const auto& tag : document.metadata.tags)
    {
        file << "tag " << tag << "\n";
    }
    for (const auto& note : document.metadata.notes)
    {
        file << "note " << note << "\n";
    }
    if (document.metadata.challenge.has_value())
    {
        const auto& challenge = *document.metadata.challenge;
        file << "challenge " << challenge.required_objective_sensors << ' ' << challenge.hold_ticks << ' '
             << challenge.maximum_emitted_mass.value_or(-1.0) << ' '
             << challenge.maximum_outflow_mass.value_or(-1.0) << ' ' << challenge.title << "\n";
    }
    file << "grid " << document.grid_width << ' ' << document.grid_height << ' ' << document.cell_size << "\n";
    for (const auto& cell : document.solid_cells)
    {
        file << "wall " << cell.x << ' ' << cell.y << "\n";
    }
    for (const auto& emitter : document.emitters)
    {
        file << "emitter "
             << (emitter.kind == WaterEmitterKind::Directional ? "directional" : "omni") << ' '
             << emitter.position.x << ' '
             << emitter.position.y << ' '
             << emitter.direction.x << ' '
             << emitter.direction.y << ' '
             << emitter.speed << ' '
             << emitter.emission_rate << ' '
             << (emitter.enabled ? 1 : 0) << "\n";
    }
    for (const auto& gate : document.gates)
    {
        file << "gate "
             << gate.x << ' '
             << gate.y << ' '
             << (gate.open ? 1 : 0) << "\n";
    }
    for (const auto& sensor : document.sensors)
    {
        file << "sensor "
             << sensor.x << ' '
             << sensor.y << ' '
             << sensor.width << ' '
             << sensor.height << ' '
             << (sensor.enabled ? 1 : 0) << ' '
             << (sensor.active ? 1 : 0) << ' '
             << (sensor.objective ? 1 : 0);
        if (!sensor.label.empty())
        {
            file << ' ' << sensor.label;
        }
        file << "\n";
    }
    for (const auto& drain : document.drains)
    {
        file << "drain "
             << drain.x << ' '
             << drain.y << ' '
             << drain.width << ' '
             << drain.height << ' '
             << (drain.enabled ? 1 : 0) << "\n";
    }
    for (const auto& pump : document.pumps)
    {
        file << "pump "
             << pump.x << ' '
             << pump.y << ' '
             << pump.width << ' '
             << pump.height << ' '
             << (pump.enabled ? 1 : 0) << ' '
             << pump.direction.x << ' '
             << pump.direction.y << ' '
             << pump.strength << "\n";
    }
    for (const auto& valve : document.valves)
    {
        file << "valve "
             << valve.x << ' '
             << valve.y << ' '
             << (valve.open ? 1 : 0) << "\n";
    }

    file.flush();
    const bool write_succeeded = static_cast<bool>(file);
    file.close();
    if (!write_succeeded || !load_scene(temporary_path).has_value())
    {
        std::filesystem::remove(temporary_path, cleanup_error);
        return false;
    }

    std::error_code ec;
    const bool had_existing_file = std::filesystem::exists(path, ec) && !ec;
    if (had_existing_file)
    {
        std::filesystem::remove(backup_path, ec);
        ec.clear();
        std::filesystem::rename(path, backup_path, ec);
        if (ec)
        {
            std::filesystem::remove(temporary_path, cleanup_error);
            return false;
        }
    }

    ec.clear();
    std::filesystem::rename(temporary_path, path, ec);
    if (ec)
    {
        if (had_existing_file)
        {
            std::error_code restore_error;
            std::filesystem::rename(backup_path, path, restore_error);
        }
        std::filesystem::remove(temporary_path, cleanup_error);
        return false;
    }

    return true;
}

[[nodiscard]] inline bool save_scene(const std::filesystem::path& path, const WaterSimulation2D& simulation)
{
    return save_scene(path, capture_scene(simulation));
}

[[nodiscard]] inline bool save_scene(
    const std::filesystem::path& path,
    const WaterSimulation2D& simulation,
    SceneMetadata metadata)
{
    return save_scene(path, capture_scene(simulation, std::move(metadata)));
}

[[nodiscard]] inline bool load_scene(const std::filesystem::path& path, WaterSimulation2D& simulation)
{
    const auto document = load_scene(path);
    if (!document.has_value())
    {
        return false;
    }

    apply_scene(*document, simulation);
    return true;
}

[[nodiscard]] inline bool load_scene(
    const std::filesystem::path& path,
    WaterSimulation2D& simulation,
    SceneMetadata* metadata_out)
{
    const auto document = load_scene(path);
    if (!document.has_value())
    {
        return false;
    }

    if (metadata_out != nullptr)
    {
        *metadata_out = document->metadata;
    }

    apply_scene(*document, simulation);
    return true;
}

[[nodiscard]] inline bool load_scene(
    const std::filesystem::path& path,
    WaterSimulation2D& simulation,
    SceneMetadata* metadata_out,
    FluidSolverProfile fallback_profile,
    std::optional<FluidSolverProfile> forced_profile = std::nullopt)
{
    const auto document = load_scene(path);
    if (!document.has_value())
    {
        return false;
    }

    if (metadata_out != nullptr)
    {
        *metadata_out = document->metadata;
    }

    apply_scene(*document, simulation, fallback_profile, forced_profile);
    return true;
}
} // namespace physics_sim
