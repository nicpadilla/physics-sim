#include <physics_sim/scene_document.hpp>
#include <physics_sim/water_simulation.hpp>

#include <array>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <utility>

namespace fs = std::filesystem;

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
    const fs::path repo_root{PHYSICS_SIM_SOURCE_DIR};
    {
        physics_sim::WaterSimulation2D simulation;
        physics_sim::SceneMetadata metadata;
        const fs::path path = repo_root / "scenes/starter_basin.pscene";
        REQUIRE(physics_sim::load_scene(path, simulation, &metadata), "starter basin scene failed to load");
        REQUIRE(metadata.title == "Starter Basin", "starter basin title mismatch");
        REQUIRE(simulation.emitters().empty(), "starter basin should not contain a persistent emitter");
        REQUIRE(simulation.grid().solid(20, 24), "starter basin should keep the left basin wall");
        REQUIRE(simulation.grid().solid(60, 24), "starter basin should keep the right basin wall");
        REQUIRE(simulation.grid().solid(40, 36), "starter basin should keep the basin floor");
    }

    const std::array<std::pair<std::string_view, std::string_view>, 6> scenes{
        std::make_pair(std::string_view{"scenes/demo_scene.pscene"}, std::string_view{"U-Container Demo"}),
        std::make_pair(std::string_view{"scenes/free_fall.pscene"}, std::string_view{"Free Fall"}),
        std::make_pair(std::string_view{"scenes/hose_wall_impact.pscene"}, std::string_view{"Hose Wall Impact"}),
        std::make_pair(std::string_view{"scenes/omni_spray.pscene"}, std::string_view{"Omni Spray"}),
        std::make_pair(std::string_view{"scenes/objective_fill.pscene"}, std::string_view{"Objective Fill"}),
        std::make_pair(std::string_view{"scenes/future_device.pscene"}, std::string_view{"Future Device Placeholder"}),
    };

    for (const auto& [relative_path, expected_title] : scenes)
    {
        physics_sim::WaterSimulation2D simulation;
        physics_sim::SceneMetadata metadata;
        const fs::path path = repo_root / fs::path{relative_path};
        REQUIRE(physics_sim::load_scene(path, simulation, &metadata), "gallery scene failed to load");
        REQUIRE(metadata.title == expected_title, "gallery scene title mismatch");
        REQUIRE(!metadata.notes.empty(), "gallery scene is missing purpose notes");
        REQUIRE(!metadata.description.empty(), "gallery scene is missing a description");
        REQUIRE(fs::exists(path.parent_path() / (path.stem().string() + ".thumb.bmp")), "gallery scene is missing its thumbnail sidecar");

        if (relative_path == std::string_view{"scenes/demo_scene.pscene"})
        {
            REQUIRE(!simulation.emitters().empty(), "hose-fed demo scene should keep its persistent emitter");
        }

        if (relative_path == std::string_view{"scenes/objective_fill.pscene"})
        {
            REQUIRE(!simulation.sensors().empty(), "objective gallery scene is missing its sensor");
            REQUIRE(simulation.metrics().objective_sensors == 1, "objective gallery scene did not expose an objective sensor");
            REQUIRE(!simulation.metrics().objective_completed, "objective gallery scene should start incomplete");
        }
    }

    return 0;
}
