# Physics Sim

Physics Sim is a Windows C++20/SDL2 deterministic 2D water sandbox and laboratory.

Version `0.2.0-alpha.1` is the first recovery prerelease. One executable provides a focused custom-SDL sandbox and a Dear ImGui engineering laboratory over the same validated simulation core. It is an alpha rather than a finished game; `ROADMAP.md` and `PROGRESS.md` record the current evidence and remaining limitations.

## Recovery Product

- **Sandbox:** pour water, draw/erase walls, pause/step/resume, reset, undo/redo, and save/load.
- **Lab:** canonical scenarios, profiles, metrics, plots, field views, replay, capture, and comparison.

The recovered command/data contracts, canonical lab scenarios, basin rendering, regression suite, and player acceptance matrix are implemented. Pre-recovery sources and evidence are preserved by tag `pre-recovery-2026-07-10`.

## Current Limitations

- This is a plausible deterministic 2D CPU water model, not a 3D, GPU, multiphase, or research-grade simulator.
- The SDL application and some solver passes remain larger than ideal even though compiled dependency boundaries and the simulation facade are enforced.
- Full verification includes the 6000-tick solver run, visual/replay regressions, and packaging; Fast and Standard are the bounded developer loops.
- Scene v2, replay v2, and recovered-settings v1 intentionally reject pre-recovery formats.
- Secondary devices, objectives, progression, gallery breadth, and decorative polish will not be exposed in the first recovered release.
- The alpha has Windows-only build and package coverage.

## Requirements

- Windows
- Visual Studio 2022 Build Tools with MSVC and the Windows SDK
- PowerShell
- CMake 3.25 or newer (the wrappers can use the Visual Studio bundled CMake)
- vcpkg dependencies declared in `vcpkg.json`

## Configure, Build, Test, Run

```powershell
.\scripts\configure.ps1
.\scripts\build.ps1
.\scripts\test.ps1
.\scripts\run-smoke.ps1
.\build\windows-x64\Debug\physics-sim.exe --mode sandbox
.\build\windows-x64\Debug\physics-sim.exe --mode lab
```

Sandbox starts without engineering metrics. Add `--debug-overlay` only when diagnosing a run; laboratory diagnostics remain available through `--mode lab`.

Lab capture smoke:

```powershell
.\scripts\run-lab-smoke.ps1
```

Use `scripts/test.ps1 -Tier Fast|Standard|Full`. Fast is the sub-30-second developer loop, Standard adds solver and smoke coverage under 90 seconds, and Full adds visuals, benchmarks, and release checks under eight minutes.

Current full local bundle:

```powershell
.\scripts\verify-all.ps1
```

Additional current checks:

```powershell
.\scripts\verify-fluid-quality-suite.ps1
.\scripts\verify-replay-suite.ps1
.\scripts\measure-water-solver.ps1 -Profile All
.\scripts\check-tracking.ps1
.\scripts\check-dependencies.ps1
.\scripts\capture-recovery-contact-sheet.ps1 -Tick 2400
```

Create the recovery prerelease package:

```powershell
.\scripts\package-release.ps1
```

## Sandbox Controls

- Hold left mouse in pointer-water mode to pour.
- `0`: pointer water; `1`: draw wall; `2`: erase wall.
- `Space`: pause/resume; `S`: single step while paused; `R`: clear fluid.
- `Ctrl+Z` / `Ctrl+Y`: undo/redo.
- `F5`: save; `F9`: load autosave; `F10`: retry current scene.
- Middle-drag: pan; mouse wheel: zoom; `H`: help; `V`: visual mode; `L`: laboratory.
- `Esc`: pause/menu navigation.

Secondary tool shortcuts `3` through `9` and gallery navigation are disabled in the recovery sandbox. Their underlying source remains available for later feature-by-feature restoration.

## Evidence And Tracking

- `ROADMAP.md`: recovery product contract.
- `PROGRESS.md`: current implementation, automation, and human-acceptance status.
- `ISSUES.md`: implementation-ready recovery queue.
- `docs/TRACKING.md`: lifecycle and evidence rules.
- `docs/adr/README.md`: architecture decisions.
- `docs/recovery-architecture.md`: active compiled boundaries and simulation facade.
- `docs/history/pre-recovery-snapshot.md`: immutable legacy snapshot reference.

A matching image hash proves reproducibility only. Recovered visual acceptance requires semantic checks plus a named, dated review. See `docs/release-notes-0.2.0-alpha.1.md` for the prerelease scope and limitations.
