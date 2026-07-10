#include <physics_sim/scene_document.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

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
    const fs::path scene_path = repo_root / "scenes/tutorial_intro.pscene";
    const fs::path thumbnail_path = scene_path.parent_path() / (scene_path.stem().string() + ".thumb.bmp");

    physics_sim::WaterSimulation2D simulation;
    physics_sim::SceneMetadata metadata;
    REQUIRE(physics_sim::load_scene(scene_path, simulation, &metadata), "tutorial scene failed to load");
    REQUIRE(metadata.title == "Guided Tutorial", "tutorial scene title mismatch");
    REQUIRE(metadata.description == "First-run guided scene for learning the sandbox loop.", "tutorial scene description mismatch");
    REQUIRE(!metadata.notes.empty(), "tutorial scene is missing notes");
    REQUIRE(metadata.notes.front() == "Use this scene on first launch or with --tutorial-mode.", "tutorial scene note mismatch");
    REQUIRE(metadata.notes[1].find("pouring") != std::string::npos, "tutorial scene note did not describe the recovery loop");
    REQUIRE(metadata.notes[1].find("gallery") == std::string::npos, "tutorial scene note exposed deferred gallery breadth");
    REQUIRE(simulation.emitters().empty(), "tutorial scene should not contain a persistent water emitter");
    REQUIRE(fs::exists(thumbnail_path), "tutorial scene is missing its thumbnail sidecar");

    return 0;
}
