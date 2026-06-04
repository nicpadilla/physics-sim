#define SDL_MAIN_HANDLED

#include <SDL.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <system_error>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <physics_sim/action.hpp>
#include <physics_sim/debug_overlay.hpp>
#include <physics_sim/fixed_timestep.hpp>
#include <physics_sim/math.hpp>
#include <physics_sim/scene_document.hpp>
#include <physics_sim/scene_controller.hpp>
#include <physics_sim/scene_viewport.hpp>
#include <physics_sim/simulation_state.hpp>
#include <physics_sim/water_simulation.hpp>

namespace
{
using Clock = std::chrono::steady_clock;
namespace fs = std::filesystem;

struct RuntimeOptions
{
    std::optional<std::chrono::milliseconds> autoExitAfter;
    std::optional<fs::path> dumpFramePath;
    std::optional<std::chrono::milliseconds> dumpFrameAfter;
    std::optional<std::uint64_t> dumpFrameAfterTicks;
};

void show_error(const char* title, const std::string& message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message.c_str(), nullptr);
}

std::optional<std::chrono::milliseconds> parse_milliseconds(std::string_view value)
{
    int parsed = 0;
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto [ptr, ec] = std::from_chars(begin, end, parsed);
    if (ec != std::errc{} || ptr != end || parsed < 0)
    {
        return std::nullopt;
    }

    return std::chrono::milliseconds(parsed);
}

RuntimeOptions parse_command_line(int argc, char* argv[])
{
    RuntimeOptions options;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--auto-exit-ms" && i + 1 < argc)
        {
            options.autoExitAfter = parse_milliseconds(argv[++i]);
        }
        else if (arg.starts_with("--auto-exit-ms="))
        {
            options.autoExitAfter = parse_milliseconds(arg.substr(std::string_view("--auto-exit-ms=").size()));
        }
        else if (arg == "--dump-frame" && i + 1 < argc)
        {
            options.dumpFramePath = fs::path{argv[++i]};
        }
        else if (arg.starts_with("--dump-frame="))
        {
            options.dumpFramePath = fs::path{arg.substr(std::string_view("--dump-frame=").size())};
        }
        else if (arg == "--dump-frame-after-ms" && i + 1 < argc)
        {
            options.dumpFrameAfter = parse_milliseconds(argv[++i]);
        }
        else if (arg.starts_with("--dump-frame-after-ms="))
        {
            options.dumpFrameAfter = parse_milliseconds(arg.substr(std::string_view("--dump-frame-after-ms=").size()));
        }
        else if (arg == "--dump-frame-after-ticks" && i + 1 < argc)
        {
            std::uint64_t parsed = 0;
            const auto value = std::string_view{argv[++i]};
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (ec == std::errc{} && ptr == value.data() + value.size())
            {
                options.dumpFrameAfterTicks = parsed;
            }
        }
        else if (arg.starts_with("--dump-frame-after-ticks="))
        {
            std::uint64_t parsed = 0;
            const std::string_view value = arg.substr(std::string_view("--dump-frame-after-ticks=").size());
            const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (ec == std::errc{} && ptr == value.data() + value.size())
            {
                options.dumpFrameAfterTicks = parsed;
            }
        }
    }

    return options;
}

const char* tool_name(physics_sim::SceneTool tool) noexcept
{
    switch (tool)
    {
    case physics_sim::SceneTool::PaintWall:
        return "wall";
    case physics_sim::SceneTool::EraseWall:
        return "erase";
    case physics_sim::SceneTool::DirectionalEmitter:
        return "hose";
    case physics_sim::SceneTool::OmniEmitter:
        return "omni";
    }

    return "unknown";
}

void draw_grid(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, 38, 48, 62, 255);

    for (std::size_t x = 0; x <= grid.width(); ++x)
    {
        const physics_sim::Vec2 top = viewport.world_to_window({static_cast<float>(x) * cell_size, 0.0f});
        const physics_sim::Vec2 bottom = viewport.world_to_window({static_cast<float>(x) * cell_size, static_cast<float>(grid.height()) * cell_size});
        SDL_RenderDrawLineF(renderer, top.x, top.y, bottom.x, bottom.y);
    }

    for (std::size_t y = 0; y <= grid.height(); ++y)
    {
        const physics_sim::Vec2 left = viewport.world_to_window({0.0f, static_cast<float>(y) * cell_size});
        const physics_sim::Vec2 right = viewport.world_to_window({static_cast<float>(grid.width()) * cell_size, static_cast<float>(y) * cell_size});
        SDL_RenderDrawLineF(renderer, left.x, left.y, right.x, right.y);
    }
}

SDL_FRect world_rect(
    const physics_sim::SceneViewport& viewport,
    physics_sim::Vec2 world_top_left,
    physics_sim::Vec2 world_size)
{
    const physics_sim::Vec2 top_left = viewport.world_to_window(world_top_left);
    const physics_sim::Vec2 bottom_right = viewport.world_to_window(world_top_left + world_size);
    SDL_FRect rect{};
    rect.x = std::min(top_left.x, bottom_right.x);
    rect.y = std::min(top_left.y, bottom_right.y);
    rect.w = std::abs(bottom_right.x - top_left.x);
    rect.h = std::abs(bottom_right.y - top_left.y);
    return rect;
}

void draw_walls(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    const float cell_size = grid.cell_size();

    SDL_SetRenderDrawColor(renderer, 54, 58, 67, 255);
    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            if (!grid.solid(x, y))
            {
                continue;
            }

            const SDL_FRect rect = world_rect(
                viewport,
                {static_cast<float>(x) * cell_size, static_cast<float>(y) * cell_size},
                {cell_size, cell_size});
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}

void draw_particles(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const float cell_size = simulation.grid().cell_size();
    const float particle_size = std::max(8.0f, cell_size * 0.6f);

    SDL_SetRenderDrawColor(renderer, 78, 214, 255, 255);
    for (const auto& particle : simulation.particles())
    {
        const SDL_FRect rect = world_rect(
            viewport,
            {particle.position.x - particle_size * 0.5f, particle.position.y - particle_size * 0.5f},
            {particle_size, particle_size});
        SDL_RenderFillRectF(renderer, &rect);
    }
}

void draw_fluid_density(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const auto& grid = simulation.grid();
    if (grid.width() == 0 || grid.height() == 0)
    {
        return;
    }

    std::vector<std::size_t> counts(grid.cell_count(), 0);
    const float inv_cell_size = 1.0f / grid.cell_size();
    for (const auto& particle : simulation.particles())
    {
        const int cell_x = static_cast<int>(std::floor(particle.position.x * inv_cell_size));
        const int cell_y = static_cast<int>(std::floor(particle.position.y * inv_cell_size));
        if (cell_x < 0 || cell_y < 0 || static_cast<std::size_t>(cell_x) >= grid.width() || static_cast<std::size_t>(cell_y) >= grid.height())
        {
            continue;
        }

        if (grid.solid(static_cast<std::size_t>(cell_x), static_cast<std::size_t>(cell_y)))
        {
            continue;
        }

        const std::size_t index = grid.cell_index(static_cast<std::size_t>(cell_x), static_cast<std::size_t>(cell_y));
        ++counts[index];
    }

    for (std::size_t y = 0; y < grid.height(); ++y)
    {
        for (std::size_t x = 0; x < grid.width(); ++x)
        {
            const std::size_t index = grid.cell_index(x, y);
            const std::size_t count = counts[index];
            if (count == 0)
            {
                continue;
            }

            const std::uint8_t alpha = static_cast<std::uint8_t>(std::min<std::size_t>(220, 50 + count * 18));
            SDL_SetRenderDrawColor(renderer, 46, 180, 255, alpha);
            const SDL_FRect rect = world_rect(
                viewport,
                {static_cast<float>(x) * grid.cell_size(), static_cast<float>(y) * grid.cell_size()},
                {grid.cell_size(), grid.cell_size()});
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}

void draw_emitters(SDL_Renderer* renderer, const physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const float cell_size = simulation.grid().cell_size();
    const float icon_size = cell_size * 0.4f;
    const float line_length = cell_size * 0.9f;

    for (const auto& emitter : simulation.emitters())
    {
        const SDL_FRect icon = world_rect(
            viewport,
            {emitter.position.x - icon_size * 0.5f, emitter.position.y - icon_size * 0.5f},
            {icon_size, icon_size});

        if (emitter.kind == physics_sim::WaterEmitterKind::Directional)
        {
            SDL_SetRenderDrawColor(renderer, 255, 214, 87, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 126, 87, 255);
        }
        SDL_RenderFillRectF(renderer, &icon);

        const physics_sim::Vec2 direction = physics_sim::normalize(emitter.direction);
        const physics_sim::Vec2 line_end_world = emitter.position + (direction * line_length);
        const physics_sim::Vec2 line_start = viewport.world_to_window(emitter.position);
        const physics_sim::Vec2 line_end = viewport.world_to_window(line_end_world);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLineF(renderer, line_start.x, line_start.y, line_end.x, line_end.y);
    }
}

void draw_crosshair(SDL_Renderer* renderer, int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 122, 226, 255, 255);
    SDL_RenderDrawLine(renderer, x - 12, y, x + 12, y);
    SDL_RenderDrawLine(renderer, x, y - 12, x, y + 12);
    SDL_Rect dot{ x - 2, y - 2, 4, 4 };
    SDL_RenderFillRect(renderer, &dot);
}

void draw_tool_preview(
    SDL_Renderer* renderer,
    const physics_sim::SceneViewport& viewport,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::Vec2& mouse_world)
{
    const float cell_size = simulation.grid().cell_size();
    const auto tool = controller.tool();
    if (tool == physics_sim::SceneTool::PaintWall || tool == physics_sim::SceneTool::EraseWall)
    {
        const physics_sim::Vec2 cell_top_left{
            std::floor(mouse_world.x / cell_size) * cell_size,
            std::floor(mouse_world.y / cell_size) * cell_size,
        };
        const SDL_FRect rect = world_rect(viewport, cell_top_left, {cell_size, cell_size});
        SDL_SetRenderDrawColor(renderer, tool == physics_sim::SceneTool::PaintWall ? 96 : 255, 196, 255, 120);
        SDL_RenderDrawRectF(renderer, &rect);
    }
    else
    {
        const float icon_size = cell_size * 0.4f;
        const SDL_FRect rect = world_rect(
            viewport,
            {mouse_world.x - icon_size * 0.5f, mouse_world.y - icon_size * 0.5f},
            {icon_size, icon_size});
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 160);
        SDL_RenderDrawRectF(renderer, &rect);
    }
}

void sync_viewport_to_simulation(physics_sim::SceneViewport& viewport, const physics_sim::WaterSimulation2D& simulation)
{
    const float world_width = static_cast<float>(simulation.grid().width()) * simulation.grid().cell_size();
    const float world_height = static_cast<float>(simulation.grid().height()) * simulation.grid().cell_size();
    viewport.set_world_size({world_width, world_height});
}

void paint_wall_line(
    physics_sim::SceneController& controller,
    physics_sim::Vec2 start,
    physics_sim::Vec2 end)
{
    controller.begin_stroke(start);
    controller.drag_stroke(end);
    controller.end_stroke(end);
}

void load_demo_scene(physics_sim::SceneController& controller)
{
    const auto previous_tool = controller.tool();

    controller.reset_scene();

    controller.set_tool(physics_sim::SceneTool::PaintWall);
    paint_wall_line(controller, {320.0f, 576.0f}, {960.0f, 576.0f});
    paint_wall_line(controller, {320.0f, 384.0f}, {320.0f, 576.0f});
    paint_wall_line(controller, {960.0f, 384.0f}, {960.0f, 576.0f});

    controller.set_emitter_direction({0.0f, 1.0f});
    controller.set_emitter_speed(8.0f);
    controller.set_emission_rate(42.0f);
    controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
    controller.place_fixture({640.0f, 160.0f});

    controller.set_tool(previous_tool);
}

bool load_scene_from_file(
    const fs::path& path,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    physics_sim::SimulationState& state,
    physics_sim::FixedStepDriver& driver)
{
    if (!physics_sim::load_scene(path, simulation))
    {
        return false;
    }

    sync_viewport_to_simulation(viewport, simulation);
    state.reset();
    driver.reset();
    return true;
}

bool save_frame(SDL_Renderer* renderer, const fs::path& path, int width, int height)
{
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surface == nullptr)
    {
        return false;
    }

    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    const int read_result = SDL_RenderReadPixels(renderer, nullptr, surface->format->format, surface->pixels, surface->pitch);
    if (read_result != 0)
    {
        SDL_FreeSurface(surface);
        return false;
    }

    const std::string path_utf8 = path.string();
    const int save_result = SDL_SaveBMP(surface, path_utf8.c_str());
    SDL_FreeSurface(surface);
    return save_result == 0;
}

bool restore_demo_scene(
    const fs::path& path,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    physics_sim::SimulationState& state,
    physics_sim::FixedStepDriver& driver,
    physics_sim::SceneController& controller)
{
    if (load_scene_from_file(path, simulation, viewport, state, driver))
    {
        return true;
    }

    load_demo_scene(controller);
    sync_viewport_to_simulation(viewport, simulation);
    state.reset();
    driver.reset();
    static_cast<void>(physics_sim::save_scene(path, simulation));
    return false;
}

std::string build_window_title(
    const physics_sim::SimulationState& state,
    const physics_sim::FixedStepDriver& driver,
    const physics_sim::FixedStepDriver::AdvanceResult& last_update,
    double average_fps,
    const physics_sim::WaterSimulation2D& simulation,
    const physics_sim::SceneController& controller,
    const physics_sim::SceneViewport& viewport)
{
    std::ostringstream title;
    title.setf(std::ios::fixed);
    title.precision(2);
    title << "Physics Sim"
          << " | " << (driver.paused() ? "paused" : "running")
          << " | tool=" << tool_name(controller.tool())
          << " | ticks=" << state.tick_count
          << " | sim=" << state.simulated_time.count() << "s"
          << " | fps=" << average_fps
          << " | step=" << driver.fixed_step().count() << "s"
          << " | particles=" << simulation.particles().size()
          << " | emitters=" << simulation.emitters().size()
          << " | emitted=" << simulation.metrics().total_emitted
          << " | div=" << simulation.metrics().average_divergence_after_projection
          << "/" << simulation.metrics().max_divergence_after_projection
          << " | zoom=" << viewport.scale();

    if (last_update.steps > 0)
    {
        title << " | last=" << last_update.steps;
    }

    if (last_update.clamped)
    {
        title << " | clamped";
    }

    return title.str();
}

void apply_action(
    physics_sim::Action action,
    physics_sim::FixedStepDriver& driver,
    physics_sim::SimulationState& state,
    physics_sim::SceneController& controller,
    physics_sim::WaterSimulation2D& simulation,
    physics_sim::SceneViewport& viewport,
    const fs::path& demo_scene_path)
{
    switch (action)
    {
    case physics_sim::Action::Quit:
        break;
    case physics_sim::Action::TogglePause:
        driver.toggle_pause();
        break;
    case physics_sim::Action::StepOnce:
        driver.request_step();
        break;
    case physics_sim::Action::Reset:
        restore_demo_scene(demo_scene_path, simulation, viewport, state, driver, controller);
        break;
    }
}

void set_direction_from_keycode(physics_sim::SceneController& controller, SDL_Keycode keycode)
{
    switch (keycode)
    {
    case SDLK_UP:
    case SDLK_w:
        controller.set_emitter_direction({0.0f, -1.0f});
        break;
    case SDLK_DOWN:
    case SDLK_s:
        controller.set_emitter_direction({0.0f, 1.0f});
        break;
    case SDLK_LEFT:
    case SDLK_a:
        controller.set_emitter_direction({-1.0f, 0.0f});
        break;
    case SDLK_RIGHT:
    case SDLK_d:
        controller.set_emitter_direction({1.0f, 0.0f});
        break;
    case SDLK_q:
        controller.rotate_emitter_direction(-0.2617994f);
        break;
    case SDLK_e:
        controller.rotate_emitter_direction(0.2617994f);
        break;
    default:
        break;
    }
}
} // namespace

int main(int argc, char* argv[])
{
    const RuntimeOptions options = parse_command_line(argc, argv);
    SDL_SetMainReady();
    const fs::path demoScenePath{"scenes/demo_scene.pscene"};
    const fs::path autosaveScenePath{"scenes/autosave.pscene"};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        show_error("SDL_Init failed", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Physics Sim",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        show_error("SDL_CreateWindow failed", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    if (renderer == nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer == nullptr)
    {
        show_error("SDL_CreateRenderer failed", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool running = true;
    int windowWidth = 1280;
    int windowHeight = 720;
    physics_sim::Vec2 mouseScreen{windowWidth * 0.5f, windowHeight * 0.5f};
    physics_sim::FixedStepDriver stepDriver;
    physics_sim::SimulationState simulationState;
    physics_sim::WaterSimulation2D simulation{80, 45, 16.0f};
    physics_sim::SceneController controller{simulation};
    physics_sim::SceneViewport viewport;
    viewport.set_world_size({1280.0f, 720.0f});
    viewport.set_window_size(windowWidth, windowHeight);
    static_cast<void>(restore_demo_scene(demoScenePath, simulation, viewport, simulationState, stepDriver, controller));

    bool panning = false;
    bool painting = false;
    bool frameDumped = false;
    const bool captureByTickCount = options.dumpFrameAfterTicks.has_value();
    const bool cleanCaptureFrame = captureByTickCount;

    const auto appStart = Clock::now();
    auto lastFrameTime = appStart;
    std::uint64_t frameCount = 0;
    physics_sim::FixedStepDriver::AdvanceResult lastUpdate;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat != 0)
                {
                    break;
                }

                if (const auto action = physics_sim::action_from_keycode(event.key.keysym.sym))
                {
                    if (*action == physics_sim::Action::Quit)
                    {
                        running = false;
                    }
                    else
                    {
                        apply_action(*action, stepDriver, simulationState, controller, simulation, viewport, demoScenePath);
                    }
                    break;
                }

                switch (event.key.keysym.sym)
                {
                case SDLK_F5:
                    static_cast<void>(physics_sim::save_scene(autosaveScenePath, simulation));
                    break;
                case SDLK_F9:
                    if (!load_scene_from_file(autosaveScenePath, simulation, viewport, simulationState, stepDriver))
                    {
                        static_cast<void>(restore_demo_scene(demoScenePath, simulation, viewport, simulationState, stepDriver, controller));
                    }
                    break;
                case SDLK_1:
                    controller.set_tool(physics_sim::SceneTool::PaintWall);
                    break;
                case SDLK_2:
                    controller.set_tool(physics_sim::SceneTool::EraseWall);
                    break;
                case SDLK_3:
                    controller.set_tool(physics_sim::SceneTool::DirectionalEmitter);
                    break;
                case SDLK_4:
                    controller.set_tool(physics_sim::SceneTool::OmniEmitter);
                    break;
                case SDLK_DELETE:
                    controller.reset_scene();
                    simulationState.reset();
                    stepDriver.reset();
                    break;
                default:
                    set_direction_from_keycode(controller, event.key.keysym.sym);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseScreen.x = static_cast<float>(event.button.x);
                mouseScreen.y = static_cast<float>(event.button.y);

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    const physics_sim::Vec2 world = viewport.window_to_world(mouseScreen);
                    if (controller.tool() == physics_sim::SceneTool::PaintWall || controller.tool() == physics_sim::SceneTool::EraseWall)
                    {
                        controller.begin_stroke(world);
                        painting = true;
                    }
                    else
                    {
                        controller.place_fixture(world);
                    }
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                mouseScreen.x = static_cast<float>(event.button.x);
                mouseScreen.y = static_cast<float>(event.button.y);

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (painting)
                    {
                        controller.end_stroke(viewport.window_to_world(mouseScreen));
                        painting = false;
                    }
                }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                {
                    panning = false;
                }
                break;
            case SDL_MOUSEMOTION:
            {
                const physics_sim::Vec2 previousScreen = mouseScreen;
                mouseScreen.x = static_cast<float>(event.motion.x);
                mouseScreen.y = static_cast<float>(event.motion.y);

                if (panning)
                {
                    viewport.pan_pixels({static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)});
                }

                if (painting)
                {
                    controller.drag_stroke(viewport.window_to_world(mouseScreen));
                }

                (void)previousScreen;
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                float wheel = static_cast<float>(event.wheel.y);
                if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                {
                    wheel = -wheel;
                }

                if (wheel != 0.0f)
                {
                    const float factor = wheel > 0.0f ? 1.12f : 1.0f / 1.12f;
                    viewport.zoom_at(std::pow(factor, std::abs(wheel)), mouseScreen);
                }
                break;
            }
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                    viewport.set_window_size(windowWidth, windowHeight);
                }
                break;
            default:
                break;
            }
        }

        if (captureByTickCount && simulationState.tick_count < *options.dumpFrameAfterTicks)
        {
            stepDriver.request_step();
        }

        const auto now = Clock::now();
        const auto frameDelta = std::chrono::duration_cast<physics_sim::FixedStepDriver::duration>(now - lastFrameTime);
        lastFrameTime = now;
        const auto realElapsed = std::chrono::duration<double>(now - appStart);

        if (options.autoExitAfter && std::chrono::duration_cast<std::chrono::milliseconds>(realElapsed) >= *options.autoExitAfter)
        {
            running = false;
        }

        ++frameCount;

        lastUpdate = stepDriver.advance(frameDelta, [&](physics_sim::FixedStepDriver::duration step)
        {
            simulationState.advance(step);
            simulation.step(step.count());
        });

        SDL_SetRenderDrawColor(renderer, 14, 18, 28, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer, viewport, simulation);
        draw_fluid_density(renderer, viewport, simulation);
        draw_walls(renderer, viewport, simulation);
        draw_particles(renderer, viewport, simulation);
        draw_emitters(renderer, viewport, simulation);
        draw_tool_preview(renderer, viewport, simulation, controller, viewport.window_to_world(mouseScreen));
        draw_crosshair(renderer, static_cast<int>(mouseScreen.x), static_cast<int>(mouseScreen.y));
        const double averageFps = realElapsed.count() > 0.0 ? static_cast<double>(frameCount) / realElapsed.count() : 0.0;
        if (!cleanCaptureFrame)
        {
            physics_sim::DebugOverlayMetrics overlayMetrics;
            overlayMetrics.fps = averageFps;
            overlayMetrics.driver = &stepDriver;
            overlayMetrics.state = &simulationState;
            overlayMetrics.simulation = &simulation;
            overlayMetrics.controller = &controller;
            draw_debug_overlay(renderer, overlayMetrics, viewport.scale());
        }

        if (!frameDumped && options.dumpFramePath.has_value())
        {
            const bool ready_to_dump = captureByTickCount
                ? simulationState.tick_count >= *options.dumpFrameAfterTicks
                : !options.dumpFrameAfter.has_value() || realElapsed >= *options.dumpFrameAfter;
            if (ready_to_dump)
            {
                static_cast<void>(save_frame(renderer, *options.dumpFramePath, windowWidth, windowHeight));
                frameDumped = true;
                if (captureByTickCount)
                {
                    running = false;
                }
            }
        }
        SDL_SetWindowTitle(
            window,
            build_window_title(simulationState, stepDriver, lastUpdate, averageFps, simulation, controller, viewport).c_str());

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#if defined(_WIN32)
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main(__argc, __argv);
}
#endif
