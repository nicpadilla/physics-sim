#define SDL_MAIN_HANDLED

#include <physics_sim/save_browser.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

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

    const fs::path temp_directory = fs::temp_directory_path() / "physics-sim-save-browser-tests";
    fs::create_directories(temp_directory);

    const auto save_name = physics_sim::sanitize_save_name("U-Container Demo!");
    REQUIRE(save_name == "u-container-demo", "sanitize_save_name did not produce a stable lower-case slug");

    physics_sim::SceneMetadata metadata;
    metadata.title = "U-Container Demo";
    const fs::path current_scene_path = fs::path{"scenes"} / "demo_scene.pscene";
    const fs::path named_path = physics_sim::named_save_path(temp_directory, current_scene_path, metadata);
    REQUIRE(named_path.filename() == "u-container-demo.pscene", "named_save_path did not derive the expected filename");

    physics_sim::WaterSimulation2D simulation{4, 4, 1.0f};
    simulation.set_solid_cell(1, 1, true);
    simulation.set_solid_cell(2, 1, true);

    const fs::path autosave_path = temp_directory / "autosave.pscene";
    const fs::path other_save_path = temp_directory / "bonus-run.pscene";

    physics_sim::SceneMetadata autosave_metadata;
    autosave_metadata.title = "Autosave Snapshot";
    REQUIRE(physics_sim::save_scene(autosave_path, simulation, autosave_metadata), "failed to seed the autosave file");

    physics_sim::SceneMetadata other_metadata;
    other_metadata.title = "Bonus Run";
    REQUIRE(physics_sim::save_scene(other_save_path, simulation, other_metadata), "failed to seed the secondary save");

    const auto entries = physics_sim::build_save_browser_entries(temp_directory, autosave_path);
    REQUIRE(!entries.empty(), "save browser did not produce any entries");
    REQUIRE(entries.front().kind == physics_sim::SaveBrowserEntryKind::Autosave, "autosave should sort to the top of the browser");
    REQUIRE(entries.front().label.find("Autosave Snapshot") != std::string::npos, "autosave label did not use scene metadata");
    REQUIRE(entries.size() >= 2, "save browser did not include the secondary save");

    bool found_secondary_save = false;
    for (const auto& entry : entries)
    {
        if (entry.path == other_save_path)
        {
            found_secondary_save = true;
            REQUIRE(entry.label.find("Bonus Run") != std::string::npos, "secondary save label did not use scene metadata");
        }
    }
    REQUIRE(found_secondary_save, "save browser did not include the secondary save file");

    fs::remove_all(temp_directory);

    std::cout << "save browser tests passed" << std::endl;
    return 0;
}
