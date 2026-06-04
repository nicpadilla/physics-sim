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
- A small automated math test target

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
- `R` resets the demo scene.
- `Delete` clears the scene to a blank canvas.
- `F5` saves the current scene to `scenes/autosave.pscene`.
- `F9` loads `scenes/autosave.pscene` if it exists, otherwise the demo scene.
- `1` selects wall paint mode.
- `2` selects wall erase mode.
- `3` selects the directional hose fixture.
- `4` selects the omni emitter fixture.
- Left-drag draws or erases walls, depending on the active tool.
- Left-click places the selected emitter fixture.
- `W`, `A`, `S`, `D` or the arrow keys change the hose direction.
- `Q` and `E` rotate the hose direction.
- Middle-drag pans the camera.
- Mouse wheel zooms in and out.
- `Esc` quits the app.

Automated smoke test:

```powershell
.\scripts\run-smoke.ps1
```

Tests:

```powershell
.\scripts\test.ps1
```

Golden-scene regression check:

```powershell
.\scripts\verify-demo-scene.ps1
```

The regression check launches the demo scene, captures a deterministic frame, and compares it against `regression\demo_scene_golden.bmp`.

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the stage goals and completion markers.
