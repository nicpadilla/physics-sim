#include <physics_sim/scene_document.hpp>
#include <physics_sim/water_simulation.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

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

bool nearly_equal(float lhs, float rhs, float epsilon = 0.0001f) noexcept
{
    return std::fabs(lhs - rhs) <= epsilon;
}
} // namespace

int main()
{
    namespace fs = std::filesystem;
    using physics_sim::Vec2;

    physics_sim::WaterSimulation2D original{12, 9, 1.5f};
    original.set_solid_cell(2, 3, true);
    original.set_solid_cell(3, 3, true);
    original.set_solid_cell(4, 3, true);

    physics_sim::WaterEmitter directional;
    directional.kind = physics_sim::WaterEmitterKind::Directional;
    directional.position = Vec2{4.5f, 2.0f};
    directional.direction = Vec2{0.0f, 1.0f};
    directional.speed = 7.5f;
    directional.emission_rate = 22.0f;
    directional.enabled = false;
    original.add_emitter(directional);

    physics_sim::WaterEmitter omni;
    omni.kind = physics_sim::WaterEmitterKind::Omni;
    omni.position = Vec2{6.0f, 5.0f};
    omni.direction = Vec2{1.0f, 0.0f};
    omni.speed = 4.0f;
    omni.emission_rate = 12.0f;
    original.add_emitter(omni);

    const auto snapshot = physics_sim::capture_scene(original);
    REQUIRE(snapshot.grid_width == 12, "capture_scene lost grid width");
    REQUIRE(snapshot.grid_height == 9, "capture_scene lost grid height");
    REQUIRE(nearly_equal(snapshot.cell_size, 1.5f), "capture_scene lost cell size");
    REQUIRE(snapshot.solid_cells.size() == 3, "capture_scene lost solid cells");
    REQUIRE(snapshot.emitters.size() == 2, "capture_scene lost emitters");

    const fs::path temp_path = fs::temp_directory_path() / "physics-sim-scene-roundtrip.pscene";
    REQUIRE(physics_sim::save_scene(temp_path, snapshot), "save_scene failed");
    REQUIRE(fs::exists(temp_path), "save_scene did not create a file");

    const auto loaded = physics_sim::load_scene(temp_path);
    REQUIRE(loaded.has_value(), "load_scene failed");
    REQUIRE(loaded->solid_cells.size() == 3, "load_scene lost solid cells");
    REQUIRE(loaded->emitters.size() == 2, "load_scene lost emitters");
    REQUIRE(loaded->grid_width == 12, "load_scene lost grid width");
    REQUIRE(loaded->grid_height == 9, "load_scene lost grid height");
    REQUIRE(nearly_equal(loaded->cell_size, 1.5f), "load_scene lost cell size");

    physics_sim::WaterSimulation2D restored;
    physics_sim::apply_scene(*loaded, restored);

    REQUIRE(restored.grid().width() == 12, "apply_scene lost grid width");
    REQUIRE(restored.grid().height() == 9, "apply_scene lost grid height");
    REQUIRE(nearly_equal(restored.grid().cell_size(), 1.5f), "apply_scene lost cell size");
    REQUIRE(restored.grid().solid(2, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.grid().solid(3, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.grid().solid(4, 3), "apply_scene lost a wall cell");
    REQUIRE(restored.emitters().size() == 2, "apply_scene lost emitters");
    REQUIRE(restored.emitters().front().kind == physics_sim::WaterEmitterKind::Directional, "apply_scene changed emitter kind");
    REQUIRE(nearly_equal(restored.emitters().front().position.x, 4.5f), "apply_scene lost emitter position");
    REQUIRE(nearly_equal(restored.emitters().front().speed, 7.5f), "apply_scene lost emitter speed");
    REQUIRE(restored.emitters().back().kind == physics_sim::WaterEmitterKind::Omni, "apply_scene changed omni emitter kind");
    REQUIRE(nearly_equal(restored.emitters().back().emission_rate, 12.0f), "apply_scene lost omni emission rate");

    fs::remove(temp_path);
    return 0;
}
