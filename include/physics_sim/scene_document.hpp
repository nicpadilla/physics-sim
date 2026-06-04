#pragma once

#include <physics_sim/math.hpp>
#include <physics_sim/water_simulation.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace physics_sim
{
struct SceneEmitter
{
    WaterEmitterKind kind = WaterEmitterKind::Directional;
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float speed = 6.0f;
    float emission_rate = 0.0f;
    bool enabled = true;
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
    std::vector<SceneCell> solid_cells{};
    std::vector<SceneEmitter> emitters{};
};

[[nodiscard]] inline SceneDocument capture_scene(const WaterSimulation2D& simulation)
{
    SceneDocument document;
    document.grid_width = simulation.grid().width();
    document.grid_height = simulation.grid().height();
    document.cell_size = simulation.grid().cell_size();

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

    return document;
}

inline void apply_scene(const SceneDocument& document, WaterSimulation2D& simulation)
{
    simulation.resize(document.grid_width, document.grid_height, document.cell_size);

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
}

[[nodiscard]] inline std::optional<SceneDocument> parse_scene_text(std::string_view text)
{
    std::string text_copy{text};
    std::istringstream stream{text_copy};
    SceneDocument document;

    bool saw_header = false;
    bool saw_grid = false;

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

        if (keyword == "physics-sim-scene")
        {
            int version = 0;
            if (!(line_stream >> version) || version != 1)
            {
                return std::nullopt;
            }
            saw_header = true;
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

        return std::nullopt;
    }

    if (!saw_header || !saw_grid)
    {
        return std::nullopt;
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

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
    {
        return false;
    }

    file << "physics-sim-scene 1\n";
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

    file.flush();
    return static_cast<bool>(file);
}

[[nodiscard]] inline bool save_scene(const std::filesystem::path& path, const WaterSimulation2D& simulation)
{
    return save_scene(path, capture_scene(simulation));
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
} // namespace physics_sim
