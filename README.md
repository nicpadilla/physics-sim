# Physics Sim

Windows C++ 2D physics simulator scaffold focused on water first.

## What is here now

- CMake + MSVC build setup
- SDL2 window bootstrap
- A small shared math header for future simulation code
- A running app that opens a window, renders water, draws walls, and shows cursor feedback
- Mouse-driven wall painting, erasing, emitter placement, zoom, and pan
- A repeatable demo scene with a directional hose feeding a U-shaped container
- Scene save/load support through a plain-text scene format
- Curated scene gallery with metadata notes and browser shortcuts
- A small automated math test target
- Scene format policy notes in [docs/scene-format.md](docs/scene-format.md)
- Scene metadata and thumbnail policy notes in [docs/scene-metadata.md](docs/scene-metadata.md)
- Diagnostics notes in [docs/diagnostics.md](docs/diagnostics.md)
- User settings notes in [docs/user-settings.md](docs/user-settings.md)
- Water lifecycle notes in [docs/water-lifecycle.md](docs/water-lifecycle.md)
- Future fluid-realism notes in [docs/fluid-realism-roadmap.md](docs/fluid-realism-roadmap.md)
- Physics accuracy contract in [docs/physics-accuracy-contract.md](docs/physics-accuracy-contract.md)
- Boundary interaction notes in [docs/boundary-interaction.md](docs/boundary-interaction.md)
- Future finished-game polish notes in [docs/finished-game-polish-roadmap.md](docs/finished-game-polish-roadmap.md)
- Visual style note in [docs/visual-style.md](docs/visual-style.md)
- Replay regression notes in [docs/replay-regression.md](docs/replay-regression.md)
- Architecture decision records in [docs/adr/README.md](docs/adr/README.md)

## Build

The repository is configured for the Visual Studio 2022 Build Tools already installed on this machine.
Use the PowerShell wrappers under `scripts/` if `cmake` is not already on PATH.

Configure:

```powershell
.\scripts\configure.ps1
```

Build:

```powershell
.\scripts\build.ps1
```

Run:

```powershell
.\build\windows-x64\Debug\physics-sim.exe
```

Controls:

- `Space` toggles pause and resume.
- `S` steps one fixed simulation tick while paused.
- `R` clears fluid only.
- `F10` retries the current scene.
- `Delete` removes the selected fixture or device, or clears the scene if nothing is selected.
- `F5` saves the current scene to `scenes/autosave.pscene`.
- `F9` loads `scenes/autosave.pscene` if it exists, otherwise the demo scene.
- Important actions flash a short `MSG` line in the debug overlay.
- `PgUp` and `PgDn` browse the curated scene gallery.
- `Tab` cycles through the wall, emitter, gate, sensor, drain, pump, and valve tools.
- `1` selects wall paint mode.
- `2` selects wall erase mode.
- `3` selects the directional hose fixture.
- `4` selects the omni emitter fixture.
- `5` selects the gate tool.
- `6` selects the sensor tool.
- `7` selects the drain tool.
- `8` selects the pump tool.
- `9` selects the valve tool.
- `V` cycles the visual/debug mode.
- `H` toggles the in-app help overlay.
- `Esc` opens the pause menu while playing, backs out of menu screens, and returns to the sandbox from the main menu.
- `Enter` and mouse clicks activate menu items.
- The app opens to the main menu unless `--skip-session-shell` is passed.
- `Ctrl+Z` undoes the last wall or fixture edit.
- `Ctrl+Y` redoes the last undone edit.
- Left-drag draws or erases walls, depending on the active tool.
- Left-click on empty space places the selected emitter, gate, or sensor tool.
- Left-click on an existing emitter, gate, or sensor selects it instead of placing a new one.
- Right-click clears the current fixture selection.
- `W`, `A`, `S`, `D` or the arrow keys change the hose direction.
- `Ctrl+W`, `Ctrl+A`, `Ctrl+S`, and `Ctrl+D`, or the arrow keys with `Ctrl`, move the selected fixture by one grid cell.
- `Q` and `E` rotate the hose direction or the selected fixture.
- `[` and `]` adjust emitter speed in grid cells per second.
- `-` and `=` adjust emitter emission rate.
- `T` toggles the selected emitter, gate, sensor, or valve state.
- The drain tool removes water in a 3x3 region.
- The pump tool pushes water in the current direction.
- The valve tool places open or closed flow regulators.
- Middle-drag pans the camera.
- Mouse wheel zooms in and out.
- `Space` also resumes from the pause menu.

Automated smoke test:

```powershell
.\scripts\run-smoke.ps1
```

Solver performance budget:

```powershell
.\scripts\measure-water-solver.ps1
```

This runs the repeatable solver benchmark and prints the measured timing and particle count.

One-command verification bundle:

```powershell
.\scripts\verify-all.ps1
```

This runs tracking validation, build, tests, smoke, and the replay regression suite.

Release package:

```powershell
.\scripts\package-release.ps1
```

This creates a runnable folder in `dist\physics-sim-release` and writes a contents manifest.

Diagnostics:

- `physics-sim.log` is the default log file in the current working directory.
- Use `--log-file <path>` to override the log location.
- Use `--scene-path <path>` to load a specific startup scene and capture scene-load failures in the log.
- Use `--replay-file <path>` to load a deterministic replay script.
- Use `--skip-session-shell` to bypass the menu shell for smoke, replay, and capture flows.

Settings:

- Use `--settings-file <path>` to override the user-settings file location.
- Settings persist window size, help overlay visibility, and visual mode.
- `--window-size <width>x<height>`, `--show-help`, and `--visual-mode <mixed|density|particles>` are handy for seeding a local persistence check.
- See [docs/scene-gallery.md](docs/scene-gallery.md) for the curated scene list and gallery workflow.
- The gallery includes an objective scene in [scenes/objective_fill.pscene](scenes/objective_fill.pscene) that uses a sensor target.

Tests:

```powershell
.\scripts\test.ps1
```

Golden-scene regression check:

```powershell
.\scripts\verify-demo-scene.ps1
```

The regression check launches the demo scene, captures a deterministic frame, and compares it against `regression\demo_scene_golden.bmp`.

Later density regression check:

```powershell
.\scripts\verify-demo-scene-density.ps1
```

This captures a later density-view frame and compares it against `regression\demo_scene_density_golden.bmp`.

Replay regression suite:

```powershell
.\scripts\verify-replay-suite.ps1
```

This runs deterministic replay cases defined in `regression\replay_suite.psd1`.

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the stage goals and completion markers.
