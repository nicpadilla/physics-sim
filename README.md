# Physics Sim

Physics Sim is a Windows C++20/SDL2 deterministic 2D water-sandbox project under active recovery.

The current executable builds, runs, and has broad legacy editor/game functionality, but it is **not a finished release**. The recovery is narrowing the player experience to a polished basin sandbox while creating a separate Dear ImGui laboratory client over the same validated simulation core. See `ROADMAP.md` and `PROGRESS.md` for current evidence rather than relying on the historical all-green tracker.

## Recovery Product

- **Sandbox:** pour water, draw/erase walls, pause/step/resume, reset, undo/redo, and save/load.
- **Lab:** canonical scenarios, profiles, metrics, plots, field views, replay, capture, and comparison.

Lab mode and the recovered command/data contracts are not implemented yet. The active recovery queue begins at PSIM-0089 in `ISSUES.md`. Pre-recovery sources and evidence are preserved by tag `pre-recovery-2026-07-10`.

## Current Limitations

- The committed surface regression is deterministic but does not meet the recovered visual acceptance bar.
- The executable entry point is now thin, but the legacy SDL application remains concentrated in `src/app/application.cpp` and solver orchestration remains in a large header pending staged extraction.
- The current test command includes long solver and visual cases and takes several minutes.
- Scene, replay, save, settings, and golden formats will intentionally break during recovery.
- Secondary devices, objectives, progression, gallery breadth, and decorative polish will not be exposed in the first recovered release.
- No Git remote, CI run, or current distributable establishes release readiness yet.

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
.\build\windows-x64\Debug\physics-sim.exe
```

The current `scripts/test.ps1` runs all 26 legacy CTest targets. PSIM-0098 will add `-Tier Fast`, `-Tier Standard`, and `-Tier Full`; do not use those parameters until that issue lands.

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
```

Create a legacy package (not a recovered release):

```powershell
.\scripts\package-release.ps1
```

## Current Legacy Controls

These controls describe the executable before the sandbox-scope recovery:

- Hold left mouse in pointer-water mode to pour.
- `0`: pointer water; `1`: draw wall; `2`: erase wall.
- `Space`: pause/resume; `S`: single step while paused; `R`: clear fluid.
- `Ctrl+Z` / `Ctrl+Y`: undo/redo.
- `F5`: save; `F9`: load autosave; `F10`: retry current scene.
- Middle-drag: pan; mouse wheel: zoom; `H`: legacy help; `V`: visual mode.
- `Esc`: pause/menu navigation.

Legacy secondary tool shortcuts `3` through `9`, gallery navigation, objectives, and session-shell behavior remain present until PSIM-0096 removes them from the recovered release UI.

## Evidence And Tracking

- `ROADMAP.md`: recovery product contract.
- `PROGRESS.md`: current implementation, automation, and human-acceptance status.
- `ISSUES.md`: implementation-ready recovery queue.
- `docs/TRACKING.md`: lifecycle and evidence rules.
- `docs/adr/README.md`: architecture decisions.
- `docs/recovery-architecture.md`: active compiled boundaries and simulation facade.
- `docs/history/pre-recovery-snapshot.md`: immutable legacy snapshot reference.

A matching image hash proves reproducibility only. Recovered visual acceptance requires semantic checks plus a named, dated human review.
