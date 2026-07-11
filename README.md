# Physics Sim

Physics Sim is a Windows C++20/SDL2 deterministic 2D water sandbox and laboratory.

Version `0.2.0-alpha.2` is the complete-water-sandbox recovery prerelease. One executable provides a custom-SDL creative sandbox and a Dear ImGui engineering laboratory over the same validated simulation core. It is an alpha rather than a finished game; `ROADMAP.md` and `PROGRESS.md` record current evidence and limitations.

## Recovery Product

- **Sandbox:** pour water; draw/erase walls; place and control emitters, gates, sensors, drains, pumps, and valves; complete challenges; browse curated scenes; pause/step/resume; reset; undo/redo; and save/load.
- **Lab:** canonical scenarios, profiles, metrics, plots, field views, replay, capture, and comparison.

The recovered command/data contracts, canonical lab scenarios, basin rendering, regression suite, and player acceptance matrix are implemented. Pre-recovery sources and evidence are preserved by tag `pre-recovery-2026-07-10`.

## Current Limitations

- This is a plausible deterministic 2D CPU water model, not a 3D, GPU, multiphase, or research-grade simulator.
- The SDL application and some solver passes remain larger than ideal even though compiled dependency boundaries and the simulation facade are enforced.
- Full verification includes the 6000-tick solver run, visual/replay regressions, and packaging; Fast and Standard are the bounded developer loops.
- Scene v2, replay v2, and recovered-settings v1 intentionally reject pre-recovery formats.
- Challenge progression is scene-local rather than a persistent campaign, and the curated gallery intentionally contains eight reviewed scenes rather than legacy breadth.
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

Use `scripts/test.ps1 -Tier Fast|Standard|Full`. Fast is the sub-30-second developer loop, Standard adds solver and smoke coverage under 90 seconds, and local Full adds visuals, benchmarks, and release checks under eight minutes. Hosted CI allows ten minutes for the same Full test set because shared Windows runners are slower; it does not omit or relax test thresholds.

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
.\scripts\verify-challenges.ps1
.\scripts\verify-gallery.ps1
.\scripts\verify-audio.ps1
```

Create the recovery prerelease package:

```powershell
.\scripts\package-release.ps1
```

## Sandbox Controls

- Hold left mouse in pointer-water mode to pour.
- `0`: pointer water; `1`: draw wall; `2`: erase wall; `A`: expand/collapse Advanced Tools.
- Advanced shortcuts `3` through `9`: directional emitter, omni emitter, gate, sensor, drain, pump, and valve.
- `Space`: pause/resume; `S`: single step while paused; `R`: clear fluid.
- `Ctrl+Z` / `Ctrl+Y`: undo/redo.
- `F5`: save; `F9`: load autosave; `F10`: retry current scene.
- Middle-drag: pan; mouse wheel: zoom; `H`: help; `V`: visual mode; `L`: laboratory.
- `PageUp` / `PageDown`: previous/next curated scene; `Esc`: pause/menu navigation.
- Scene Gallery is available from the main and pause menus. Use left/right or the pointer wheel to filter Learn, Sandbox, and Challenges.

## Evidence And Tracking

- `ROADMAP.md`: recovery product contract.
- `PROGRESS.md`: current implementation, automation, and human-acceptance status.
- `ISSUES.md`: implementation-ready recovery queue.
- `docs/TRACKING.md`: lifecycle and evidence rules.
- `docs/adr/README.md`: architecture decisions.
- `docs/recovery-architecture.md`: active compiled boundaries and simulation facade.
- `docs/history/pre-recovery-snapshot.md`: immutable legacy snapshot reference.

A matching image hash proves reproducibility only. Visual and usability acceptance requires semantic checks plus a named, dated review. See `docs/release-notes-0.2.0-alpha.2.md` for this prerelease's scope and limitations.
