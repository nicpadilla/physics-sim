#include <physics_sim/scene_document.hpp>
#include <physics_sim/gallery_manifest.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>

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

    physics_sim::GalleryManifest manifest;
    std::string manifest_error;
    REQUIRE(physics_sim::load_gallery_manifest(repo_root / "gallery/gallery.manifest", manifest, &manifest_error), "curated gallery manifest failed to load");
    REQUIRE(manifest.version == 1, "gallery manifest version mismatch");
    REQUIRE(manifest.entries.size() >= 8, "gallery must contain at least eight reviewed entries");
    std::set<physics_sim::GalleryCategory> categories;
    int previous_order = -1;
    for (const auto& entry : manifest.entries)
    {
        REQUIRE(entry.sort_order > previous_order, "gallery entries must have unique ascending sort orders");
        previous_order = entry.sort_order;
        categories.insert(entry.category);
        REQUIRE(!entry.required_features.empty(), "gallery entry is missing required features");
        physics_sim::WaterSimulation2D simulation;
        physics_sim::SceneMetadata metadata;
        const fs::path path = repo_root / entry.scene_path;
        REQUIRE(physics_sim::load_scene(path, simulation, &metadata), "gallery scene failed to load");
        REQUIRE(!metadata.title.empty(), "gallery scene title is missing");
        REQUIRE(!metadata.notes.empty(), "gallery scene is missing purpose notes");
        REQUIRE(!metadata.description.empty(), "gallery scene is missing a description");
        REQUIRE(fs::exists(repo_root / entry.thumbnail_path), "gallery scene is missing its curated thumbnail");
    }
    REQUIRE(categories.size() == 3, "gallery must contain Learn, Sandbox, and Challenges categories");

    const fs::path malformed_path = fs::temp_directory_path() / "physics-sim-duplicate-gallery.manifest";
    {
        std::ofstream malformed(malformed_path);
        malformed << "physics-sim-gallery\t1\n"
                  << "entry\tduplicate\tlearn\t1\tscenes/a.pscene\tgallery/a.bmp\tpour\tA\tDescription\n"
                  << "entry\tduplicate\tsandbox\t2\tscenes/b.pscene\tgallery/b.bmp\twalls\tB\tDescription\n";
    }
    physics_sim::GalleryManifest rejected;
    REQUIRE(!physics_sim::load_gallery_manifest(malformed_path, rejected, &manifest_error), "duplicate gallery ids must be rejected");
    std::error_code ec;
    fs::remove(malformed_path, ec);

    return 0;
}
