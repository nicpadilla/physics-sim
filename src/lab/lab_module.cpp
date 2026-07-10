#include <physics_sim/lab_application.hpp>
#include <physics_sim/mode_switch.hpp>

#include <physics_sim/simulation.hpp>

#include <SDL.h>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace physics_sim::lab
{
namespace
{
using Clock = std::chrono::steady_clock;

struct LabOptions
{
    std::optional<std::chrono::milliseconds> auto_exit{};
    std::optional<std::filesystem::path> capture_bundle{};
    int scenario = 1;
};

LabOptions parse_options(int argc, char *argv[])
{
    LabOptions options;
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];
        if (argument == "--auto-exit-ms" && index + 1 < argc)
        {
            options.auto_exit = std::chrono::milliseconds{std::max(0, std::atoi(argv[++index]))};
        }
        else if (argument == "--capture-bundle" && index + 1 < argc)
        {
            options.capture_bundle = std::filesystem::path{argv[++index]};
        }
        else if (argument == "--scenario" && index + 1 < argc)
        {
            options.scenario = std::clamp(std::atoi(argv[++index]), 0, 9);
        }
    }
    return options;
}

constexpr std::array<const char *, 10> scenario_names{
    "Closed box",     "U-container",  "Still pool", "Hydrostatic column", "Dam break", "Wall and corner impact",
    "Narrow channel", "Overcrowding", "Long run",   "Determinism"};

void add_border(Simulation &simulation, const SimulationConfig &config)
{
    for (std::size_t x = 0; x < config.grid_width; ++x)
    {
        simulation.apply(SetSolidCellCommand{x, 0, true});
        simulation.apply(SetSolidCellCommand{x, config.grid_height - 1, true});
    }
    for (std::size_t y = 0; y < config.grid_height; ++y)
    {
        simulation.apply(SetSolidCellCommand{0, y, true});
        simulation.apply(SetSolidCellCommand{config.grid_width - 1, y, true});
    }
}

void seed_cell(Simulation &simulation, const SimulationConfig &config, std::size_t x, std::size_t y, Vec2 velocity = {})
{
    constexpr std::array<std::array<float, 2>, 4> offsets{{{{0.3f, 0.3f}}, {{0.7f, 0.3f}}, {{0.3f, 0.7f}}, {{0.7f, 0.7f}}}};
    for (const auto &offset : offsets)
    {
        simulation.apply(
            SeedParticleCommand{{(static_cast<float>(x) + offset[0]) * config.cell_size, (static_cast<float>(y) + offset[1]) * config.cell_size}, velocity});
    }
}

void add_basin(Simulation &simulation, const SimulationConfig &config, float emission_rate, bool emitter)
{
    const std::size_t left = config.grid_width / 4;
    const std::size_t right = config.grid_width * 3 / 4;
    const std::size_t floor = config.grid_height * 4 / 5;
    for (std::size_t y = config.grid_height / 3; y <= floor; ++y)
    {
        simulation.apply(SetSolidCellCommand{left, y, true});
        simulation.apply(SetSolidCellCommand{right, y, true});
    }
    for (std::size_t x = left; x <= right; ++x)
    {
        simulation.apply(SetSolidCellCommand{x, floor, true});
    }
    if (emitter)
    {
        simulation.apply(AddEmitterCommand{
            SimulationEmitterKind::Directional,
            {static_cast<float>(config.grid_width) * config.cell_size * 0.5f, static_cast<float>(config.grid_height) * config.cell_size * 0.18f},
            {0.0f, 1.0f},
            5.0f,
            emission_rate,
            true});
    }
}

std::unique_ptr<Simulation> make_simulation(int scenario, FluidSolverProfile profile, float gravity, double timestep, float emission_rate)
{
    SimulationConfig config;
    config.solver_profile = profile;
    config.gravity_acceleration = gravity;
    config.fixed_timestep = timestep;
    auto simulation = std::make_unique<Simulation>(config);
    add_border(*simulation, config);
    if (scenario == 1 || scenario == 8 || scenario == 9)
    {
        add_basin(*simulation, config, emission_rate, true);
    }
    else if (scenario == 0)
    {
        simulation->apply(AddEmitterCommand{SimulationEmitterKind::Directional, {640.0f, 120.0f}, {0.0f, 1.0f}, 3.0f, emission_rate * 0.5f, true});
    }
    else if (scenario == 2 || scenario == 3)
    {
        const std::size_t left = scenario == 2 ? 25 : 35;
        const std::size_t right = scenario == 2 ? 54 : 44;
        for (std::size_t y = 20; y < 35; ++y)
        {
            for (std::size_t x = left; x <= right; ++x)
            {
                seed_cell(*simulation, config, x, y);
            }
        }
    }
    else if (scenario == 4)
    {
        for (std::size_t y = 12; y < 36; ++y)
        {
            for (std::size_t x = 8; x < 27; ++x)
            {
                seed_cell(*simulation, config, x, y);
            }
        }
    }
    else if (scenario == 5)
    {
        for (std::size_t y = 18; y < 28; ++y)
        {
            for (std::size_t x = 10; x < 20; ++x)
            {
                seed_cell(*simulation, config, x, y, {240.0f, 80.0f});
            }
        }
    }
    else if (scenario == 6)
    {
        for (std::size_t x = 1; x < config.grid_width - 1; ++x)
        {
            simulation->apply(SetSolidCellCommand{x, 18, x < 10 || x > 70});
            simulation->apply(SetSolidCellCommand{x, 27, true});
        }
        simulation->apply(AddEmitterCommand{SimulationEmitterKind::Directional, {96.0f, 360.0f}, {1.0f, 0.0f}, 6.0f, emission_rate, true});
    }
    else if (scenario == 7)
    {
        for (int index = 0; index < 80; ++index)
        {
            simulation->apply(SeedParticleCommand{{640.0f + static_cast<float>(index % 5), 360.0f + static_cast<float>(index / 5)}, {}});
        }
    }
    return simulation;
}

SDL_Texture *create_font_texture(SDL_Renderer *renderer)
{
    unsigned char *pixels = nullptr;
    int width = 0;
    int height = 0;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    if (texture == nullptr)
    {
        return nullptr;
    }
    SDL_UpdateTexture(texture, nullptr, pixels, width * 4);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    ImGui::GetIO().Fonts->SetTexID(static_cast<ImTextureID>(reinterpret_cast<std::uintptr_t>(texture)));
    return texture;
}

void render_imgui(SDL_Renderer *renderer, ImDrawData *draw_data)
{
    const ImVec2 display_position = draw_data->DisplayPos;
    const ImVec2 scale = draw_data->FramebufferScale;
    for (int list_index = 0; list_index < draw_data->CmdListsCount; ++list_index)
    {
        const ImDrawList *list = draw_data->CmdLists[list_index];
        for (const ImDrawCmd &command : list->CmdBuffer)
        {
            if (command.UserCallback != nullptr)
            {
                command.UserCallback(list, &command);
                continue;
            }
            SDL_Rect clip{
                static_cast<int>((command.ClipRect.x - display_position.x) * scale.x), static_cast<int>((command.ClipRect.y - display_position.y) * scale.y),
                static_cast<int>((command.ClipRect.z - command.ClipRect.x) * scale.x), static_cast<int>((command.ClipRect.w - command.ClipRect.y) * scale.y)};
            SDL_RenderSetClipRect(renderer, &clip);

            std::vector<SDL_Vertex> vertices(command.ElemCount);
            std::vector<int> indices(command.ElemCount);
            for (unsigned int element = 0; element < command.ElemCount; ++element)
            {
                const ImDrawIdx source_index = list->IdxBuffer[command.IdxOffset + element];
                const ImDrawVert &source = list->VtxBuffer[command.VtxOffset + source_index];
                const ImU32 color = source.col;
                vertices[element] = {{(source.pos.x - display_position.x) * scale.x, (source.pos.y - display_position.y) * scale.y},
                                     {static_cast<std::uint8_t>(color & 0xff), static_cast<std::uint8_t>((color >> 8) & 0xff),
                                      static_cast<std::uint8_t>((color >> 16) & 0xff), static_cast<std::uint8_t>((color >> 24) & 0xff)},
                                     {source.uv.x, source.uv.y}};
                indices[element] = static_cast<int>(element);
            }
            SDL_Texture *texture = reinterpret_cast<SDL_Texture *>(static_cast<std::uintptr_t>(command.GetTexID()));
            SDL_RenderGeometry(renderer, texture, vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()));
        }
    }
    SDL_RenderSetClipRect(renderer, nullptr);
}

void process_event(const SDL_Event &event, bool &running)
{
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    switch (event.type)
    {
    case SDL_QUIT:
        running = false;
        break;
    case SDL_MOUSEMOTION:
        io.AddMousePosEvent(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if (event.button.button >= SDL_BUTTON_LEFT && event.button.button <= SDL_BUTTON_X2)
        {
            io.AddMouseButtonEvent(event.button.button - 1, event.type == SDL_MOUSEBUTTONDOWN);
        }
        break;
    case SDL_MOUSEWHEEL:
        io.AddMouseWheelEvent(static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y));
        break;
    case SDL_TEXTINPUT:
        io.AddInputCharactersUTF8(event.text.text);
        break;
    default:
        break;
    }
}

bool write_capture_bundle(const std::filesystem::path &directory, const char *scenario, const Simulation &simulation, SDL_Renderer *renderer, int width,
                          int height)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error)
    {
        return false;
    }
    const SimulationMetrics metrics = simulation.metrics();
    const SimulationConfig &config = simulation.config();
    std::ofstream json{directory / "metrics.json", std::ios::trunc};
    json << "{\n  \"scenario\": \"" << scenario << "\",\n  \"tick\": " << metrics.tick << ",\n  \"state_digest\": \"" << simulation.state_digest()
         << "\",\n  \"fixed_timestep\": " << config.fixed_timestep << ",\n  \"gravity\": " << config.gravity_acceleration << ",\n  \"solver_profile\": \""
         << (config.solver_profile == FluidSolverProfile::Quality ? "quality" : "balanced") << "\",\n  \"active_particles\": " << metrics.active_particles
         << ",\n  \"active_cells\": " << metrics.active_cells << ",\n  \"pressure_residual\": " << metrics.pressure_relative_residual
         << ",\n  \"average_density_error\": " << metrics.average_density_error << ",\n  \"kinetic_energy\": " << metrics.kinetic_energy << "\n}\n";

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surface == nullptr)
    {
        return false;
    }
    const bool ok = SDL_RenderReadPixels(renderer, nullptr, surface->format->format, surface->pixels, surface->pitch) == 0 &&
                    SDL_SaveBMP(surface, (directory / "frame.bmp").string().c_str()) == 0;
    SDL_FreeSurface(surface);
    return ok && json.good();
}

} // namespace

int run_lab_application(int argc, char *argv[])
{
    const LabOptions options = parse_options(argc, argv);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        return 1;
    }
    SDL_Window *window =
        SDL_CreateWindow("Physics Sim Lab", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer *renderer = window == nullptr ? nullptr : SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr && window != nullptr)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (window == nullptr || renderer == nullptr)
    {
        SDL_Quit();
        return 2;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    SDL_Texture *font_texture = create_font_texture(renderer);
    if (font_texture == nullptr)
    {
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 3;
    }
    SDL_StartTextInput();

    FluidSolverProfile profile = FluidSolverProfile::Balanced;
    int scenario = options.scenario;
    float gravity = 9.8f;
    float timestep = 1.0f / 120.0f;
    float emission_rate = 90.0f;
    auto simulation = make_simulation(scenario, profile, gravity, timestep, emission_rate);
    auto balanced_comparison = make_simulation(scenario, FluidSolverProfile::Balanced, gravity, timestep, emission_rate);
    auto quality_comparison = make_simulation(scenario, FluidSolverProfile::Quality, gravity, timestep, emission_rate);
    bool running = true;
    int exit_code = 0;
    bool paused = false;
    bool identity_diverged = false;
    std::array<float, 240> particle_history{};
    std::array<float, 240> residual_history{};
    std::size_t history_cursor = 0;
    const auto start = Clock::now();
    auto previous = start;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0)
        {
            process_event(event, running);
        }
        const auto now = Clock::now();
        io.DeltaTime = std::max(1.0f / 1000.0f, std::chrono::duration<float>(now - previous).count());
        previous = now;
        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        io.DisplaySize = {static_cast<float>(width), static_cast<float>(height)};
        io.DisplayFramebufferScale = {1.0f, 1.0f};
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({590.0f, 270.0f}, ImGuiCond_Always);
        ImGui::Begin("Scenario & Run");
        if (ImGui::Combo("Scenario", &scenario,
                         "Closed box\0U-container\0Still pool\0Hydrostatic column\0Dam break\0Wall and corner impact\0Narrow channel\0Overcrowding\0Long "
                         "run\0Determinism\0"))
        {
            simulation = make_simulation(scenario, profile, gravity, timestep, emission_rate);
            balanced_comparison = make_simulation(scenario, FluidSolverProfile::Balanced, gravity, timestep, emission_rate);
            quality_comparison = make_simulation(scenario, FluidSolverProfile::Quality, gravity, timestep, emission_rate);
            identity_diverged = false;
        }
        ImGui::Text("Manifest: %s", scenario_names[static_cast<std::size_t>(scenario)]);
        int profile_index = profile == FluidSolverProfile::Quality ? 1 : 0;
        if (ImGui::Combo("Solver profile", &profile_index, "Balanced\0Quality\0"))
        {
            profile = profile_index == 0 ? FluidSolverProfile::Balanced : FluidSolverProfile::Quality;
            simulation = make_simulation(scenario, profile, gravity, timestep, emission_rate);
            identity_diverged = false;
        }
        if (ImGui::Button(paused ? "Run" : "Pause"))
        {
            paused = !paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Single step"))
        {
            simulation->step();
            paused = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Replay initial"))
        {
            simulation = make_simulation(scenario, profile, gravity, timestep, emission_rate);
            balanced_comparison = make_simulation(scenario, FluidSolverProfile::Balanced, gravity, timestep, emission_rate);
            quality_comparison = make_simulation(scenario, FluidSolverProfile::Quality, gravity, timestep, emission_rate);
            identity_diverged = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Sandbox"))
        {
            exit_code = switch_to_sandbox_exit_code;
            running = false;
        }
        bool parameters_changed = false;
        parameters_changed |= ImGui::SliderFloat("Gravity", &gravity, 0.0f, 30.0f, "%.2f");
        parameters_changed |= ImGui::SliderFloat("Timestep", &timestep, 1.0f / 240.0f, 1.0f / 30.0f, "%.5f");
        parameters_changed |= ImGui::SliderFloat("Emission", &emission_rate, 0.0f, 240.0f, "%.1f");
        if (parameters_changed)
        {
            simulation = make_simulation(scenario, profile, gravity, timestep, emission_rate);
            balanced_comparison = make_simulation(scenario, FluidSolverProfile::Balanced, gravity, timestep, emission_rate);
            quality_comparison = make_simulation(scenario, FluidSolverProfile::Quality, gravity, timestep, emission_rate);
            identity_diverged = true;
        }
        ImGui::Text("Run identity: %s", simulation->state_digest().c_str());
        ImGui::TextColored(identity_diverged ? ImVec4{1.0f, 0.55f, 0.25f, 1.0f} : ImVec4{0.35f, 0.9f, 0.5f, 1.0f}, "%s",
                           identity_diverged ? "Forked from initial configuration" : "Matches initial configuration");
        ImGui::End();

        if (!paused)
        {
            simulation->step();
            balanced_comparison->step();
            quality_comparison->step();
        }
        const SimulationMetrics metrics = simulation->metrics();
        particle_history[history_cursor % particle_history.size()] = static_cast<float>(metrics.active_particles);
        residual_history[history_cursor % residual_history.size()] = static_cast<float>(metrics.pressure_relative_residual);
        ++history_cursor;

        ImGui::SetNextWindowPos({10.0f, 290.0f}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({590.0f, 490.0f}, ImGuiCond_Always);
        ImGui::Begin("Metrics & Diagnostics");
        ImGui::Text("Tick: %llu", static_cast<unsigned long long>(metrics.tick));
        ImGui::Text("Particles: %zu | active cells: %zu", metrics.active_particles, metrics.active_cells);
        ImGui::Text("Mass: emitted %.3f removed %.3f outflow %.3f", metrics.total_emitted_mass, metrics.total_removed_mass, metrics.total_outflow_mass);
        ImGui::Text("Divergence avg %.6f max %.6f", metrics.average_divergence, metrics.max_divergence);
        ImGui::Text("Pressure residual %.7f (%zu iterations)", metrics.pressure_relative_residual, metrics.pressure_iterations);
        ImGui::Text("Density error avg %.5f max %.5f", metrics.average_density_error, metrics.max_density_error);
        ImGui::Text("Kinetic energy %.3f", metrics.kinetic_energy);
        ImGui::PlotLines("Particle history", particle_history.data(), static_cast<int>(particle_history.size()),
                         static_cast<int>(history_cursor % particle_history.size()), nullptr, 0.0f, FLT_MAX, {0.0f, 80.0f});
        ImGui::PlotLines("Pressure residual", residual_history.data(), static_cast<int>(residual_history.size()),
                         static_cast<int>(history_cursor % residual_history.size()), nullptr, 0.0f, 0.001f, {0.0f, 80.0f});
        ImGui::SeparatorText("Views");
        ImGui::TextUnformatted("Particles  Velocity  Pressure  Divergence  Density  Volume fraction  Solids");
        const SimulationMetrics balanced_metrics = balanced_comparison->metrics();
        const SimulationMetrics quality_metrics = quality_comparison->metrics();
        ImGui::SeparatorText("Balanced / Quality comparison");
        ImGui::Text("Particles: %zu / %zu", balanced_metrics.active_particles, quality_metrics.active_particles);
        ImGui::Text("Pressure residual: %.7f / %.7f", balanced_metrics.pressure_relative_residual, quality_metrics.pressure_relative_residual);
        ImGui::Text("Density error: %.5f / %.5f", balanced_metrics.average_density_error, quality_metrics.average_density_error);
        ImGui::End();

        ImGui::SetNextWindowPos({610.0f, 10.0f}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({650.0f, 770.0f}, ImGuiCond_Always);
        ImGui::Begin("Field View");
        static int field_view = 0;
        ImGui::Combo("Field", &field_view, "Particles\0Velocity\0Pressure\0Divergence\0Density\0Volume fraction\0Solids\0");
        const ImVec2 canvas_position = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_position, {canvas_position.x + canvas_size.x, canvas_position.y + canvas_size.y}, IM_COL32(8, 14, 24, 255));
        const SimulationSnapshot snapshot = simulation->snapshot();
        const float scale_x = canvas_size.x / (static_cast<float>(snapshot.grid_width) * snapshot.cell_size);
        const float scale_y = canvas_size.y / (static_cast<float>(snapshot.grid_height) * snapshot.cell_size);
        const float view_scale = std::max(0.01f, std::min(scale_x, scale_y));
        if (field_view == 0)
        {
            for (const FluidParticle &particle : snapshot.particles)
            {
                const ImVec2 point{canvas_position.x + particle.position.x * view_scale, canvas_position.y + particle.position.y * view_scale};
                draw_list->AddCircleFilled(point, std::max(1.0f, snapshot.cell_size * view_scale * 0.18f), IM_COL32(45, 184, 225, 220));
            }
        }
        else if (field_view == 1)
        {
            for (std::size_t y = 0; y < snapshot.grid_height; y += 2)
            {
                for (std::size_t x = 0; x < snapshot.grid_width; x += 2)
                {
                    const Vec2 velocity = snapshot.velocities[y * snapshot.grid_width + x];
                    const ImVec2 start_point{canvas_position.x + (x + 0.5f) * snapshot.cell_size * view_scale,
                                             canvas_position.y + (y + 0.5f) * snapshot.cell_size * view_scale};
                    const float arrow_scale = std::min(0.03f, snapshot.cell_size * view_scale / 2000.0f);
                    const ImVec2 end_point{start_point.x + velocity.x * arrow_scale, start_point.y + velocity.y * arrow_scale};
                    draw_list->AddLine(start_point, end_point, IM_COL32(90, 220, 255, 220), 1.0f);
                }
            }
        }
        else
        {
            float maximum = 1.0e-6f;
            const std::vector<float> *values = nullptr;
            if (field_view == 2)
            {
                values = &snapshot.pressures;
            }
            else if (field_view == 3)
            {
                values = &snapshot.divergences;
            }
            else if (field_view == 4)
            {
                values = &snapshot.densities;
            }
            else if (field_view == 5)
            {
                values = &snapshot.volume_fractions;
            }
            if (values != nullptr)
            {
                for (float value : *values)
                {
                    maximum = std::max(maximum, std::abs(value));
                }
            }
            for (std::size_t y = 0; y < snapshot.grid_height; ++y)
            {
                for (std::size_t x = 0; x < snapshot.grid_width; ++x)
                {
                    const std::size_t index = y * snapshot.grid_width + x;
                    const bool solid = snapshot.solid_cells[index] != 0;
                    const float value = values == nullptr ? 0.0f : std::abs((*values)[index]) / maximum;
                    if ((field_view >= 2 && field_view <= 5 && value <= 0.01f) || (field_view == 6 && !solid))
                    {
                        continue;
                    }
                    const ImU32 color = field_view == 6
                                            ? IM_COL32(90, 100, 120, 255)
                                            : IM_COL32(static_cast<int>(40.0f + value * 210.0f), static_cast<int>(80.0f + (1.0f - value) * 140.0f), 225, 220);
                    const ImVec2 minimum{canvas_position.x + x * snapshot.cell_size * view_scale, canvas_position.y + y * snapshot.cell_size * view_scale};
                    const ImVec2 cell_maximum{minimum.x + snapshot.cell_size * view_scale, minimum.y + snapshot.cell_size * view_scale};
                    draw_list->AddRectFilled(minimum, cell_maximum, color);
                }
            }
        }
        ImGui::Dummy(canvas_size);
        ImGui::End();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 10, 15, 25, 255);
        SDL_RenderClear(renderer);
        render_imgui(renderer, ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        if (options.capture_bundle && metrics.tick >= 10)
        {
            static_cast<void>(
                write_capture_bundle(*options.capture_bundle, scenario_names[static_cast<std::size_t>(scenario)], *simulation, renderer, width, height));
            running = false;
        }
        if (options.auto_exit && std::chrono::duration_cast<std::chrono::milliseconds>(now - start) >= *options.auto_exit)
        {
            running = false;
        }
    }

    SDL_StopTextInput();
    ImGui::GetIO().Fonts->SetTexID(ImTextureID{});
    SDL_DestroyTexture(font_texture);
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return exit_code;
}

} // namespace physics_sim::lab
