# Progress

Live status for the roadmap markers in `ROADMAP.md`.

Allowed statuses: `Missing`, `Partial`, `Done`, `Verified`, `Blocked`.

`Verified` requires a named command, test, scene, regression check, or documented manual check. Code-only evidence should use `Done` or `Partial`.

## Stage 1: Bootstrap And Windowing

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R1.01 | `scripts/build.ps1` succeeds on a clean checkout. | Verified | `.\scripts\build.ps1` completed successfully on 2026-06-04. | None. |
| R1.02 | `scripts/test.ps1` succeeds on a clean checkout. | Verified | `.\scripts\test.ps1` passed 6/6 tests on 2026-06-04. | None. |
| R1.03 | `scripts/run-smoke.ps1` launches the app, opens a window, and exits with code `0`. | Verified | `.\scripts\run-smoke.ps1` completed successfully on 2026-06-04. | None. |
| R1.04 | The app handles window resize, mouse movement, keyboard input, and close events without crashing. | Verified | `docs\manual-verification-checklist.md` documents repeatable resize, mouse movement, keyboard input, and close checks. | None. |
| R1.05 | The build is reproducible from repository docs and wrappers, without copying DLLs by hand. | Verified | `.\scripts\build.ps1` drives CMake/vcpkg and produced `build\windows-x64\Debug\physics-sim.exe`. | None. |
| R1.06 | No repo-local step depends on a machine-specific absolute path outside the documented helper scripts. | Verified | `.\scripts\check-tracking.ps1` enforces the absolute-path scan and passed on 2026-06-04. | None. |
| R1.07 | README or equivalent docs show exact commands needed to configure, build, test, and run. | Verified | Manual check on 2026-06-04 confirmed `README.md` documents configure, build, run, smoke, tests, `verify-demo-scene`, `verify-demo-scene-density`, and `check-tracking`. | None. |

## Stage 2: Fixed Timestep Core

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R2.01 | The main loop uses a fixed simulation step rather than advancing physics directly by render delta time. | Verified | `physics_sim_tests` covers `FixedStepDriver` accumulation and clamping, and `src\main.cpp` advances simulation through fixed steps only. | None. |
| R2.02 | Time accumulation, stepping, and clamping are handled in one place. | Verified | `physics_sim_tests` covers `FixedStepDriver` accumulation and clamping. | None. |
| R2.03 | Pause, single-step, and reset are available from input. | Verified | `physics_sim_tests` covers action mapping for Space, `S`, and `R`. | Add manual runtime check if UI behavior changes. |
| R2.04 | Input is translated into actions/commands before it reaches simulation code. | Verified | `physics_sim_tests` covers `action_from_keycode`, and `src\main.cpp` routes SDL input through `physics_sim::Action` before simulation code runs. | None. |
| R2.05 | Reset returns the simulation to its initial state without restarting the executable. | Verified | `physics_sim_tests` and `physics_sim_editor_tests` cover `FixedStepDriver::reset`, `SimulationState::reset`, and `SceneController::reset_scene`. | None. |
| R2.06 | Core helpers for math, indexing, and time accumulation have unit tests. | Verified | `physics_sim_tests` covers math, indexing, fixed timestep, simulation state, and grid basics. | None. |
| R2.07 | A paused simulation can still render and accept UI input correctly. | Verified | `docs\manual-verification-checklist.md` covers the paused render/input path and `physics_sim_editor_tests` covers pause-adjacent editor behavior. | None. |

## Stage 3: 2D Water Solver

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R3.01 | Water is represented by a real fluid simulation state. | Verified | `physics_sim_water_tests` creates `WaterSimulation2D`, particles, emitters, grid fields, and metrics. | None. |
| R3.02 | The solver uses a 2D hybrid particle-grid approach suitable for water behavior. | Verified | `physics_sim_water_tests` exercises `WaterSimulation2D` with particles, MAC-grid velocity, pressure, divergence, and solid data. | None. |
| R3.03 | Gravity, advection, pressure projection, and solid boundary handling are implemented. | Verified | `physics_sim_water_tests` covers emission/gravity, axis-separated wall-slide collision, solid collision, divergence thresholds, and stress stability. | None. |
| R3.04 | Particle-to-grid and grid-to-particle transfers are implemented and tested. | Verified | `physics_sim_water_tests` now includes focused helper tests for scatter normalization and grid-to-particle sampling. | None. |
| R3.05 | A configurable water fixture can emit fluid with position, direction, emission rate, and initial speed. | Verified | `physics_sim_editor_tests` verifies directional fixture position, direction, speed, and rate, and `physics_sim_water_tests` verifies demo-scale emitter water moves visibly downstream. | None. |
| R3.06 | Water collides with walls and accumulates in containers instead of passing through them. | Verified | `physics_sim_water_tests` includes closed-box and U-container retention/mass checks with zero leak tolerance, and `physics_sim_fluid_quality_tests --scenario hose-wall-pooling` verifies wall-impact pooling with no penetration or outflow. | None. |
| R3.07 | The solver produces only finite values during long runs. | Verified | `physics_sim_water_tests` runs 6000 steps and checks finite pressure, divergence, velocities, particles, and metrics. | None. |

## Stage 4: Walls And Fixtures

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R4.01 | The user can draw walls with the mouse. | Verified | `docs\manual-verification-checklist.md` documents the mouse-drag wall-drawing check and `physics_sim_editor_tests` covers the controller path. | None. |
| R4.02 | The user can erase or modify walls without restarting the app. | Verified | `physics_sim_editor_tests` verifies erase strokes remove target cells only. | None. |
| R4.03 | Wall edits affect the simulation immediately or on the next fixed step. | Verified | `physics_sim_editor_tests` verifies wall strokes mutate the solid grid used by the next solver step. | None. |
| R4.04 | Wall geometry is stored as scene data, not only transient runtime state. | Verified | `physics_sim_scene_persistence_tests` verifies solid cells survive capture, save, load, and apply. | None. |
| R4.05 | Fixtures are data-driven objects rather than one-off hard-coded behaviors. | Verified | `physics_sim_editor_tests` and `physics_sim_scene_persistence_tests` verify configurable directional and omni emitters as scene data. | None. |
| R4.06 | A directional hose fixture exists and emits water in a configurable direction. | Verified | `physics_sim_editor_tests` verifies directional fixture creation and configured direction. | None. |
| R4.07 | A 360-degree emitter exists and sprays water radially. | Verified | `physics_sim_water_tests` verifies omni emission creates leftward and rightward particles. | None. |
| R4.08 | A U-shaped wall with a hose above it retains water in the container instead of leaking through. | Verified | `physics_sim_water_tests` includes a U-container fill test with zero leak and mass-drift tolerance, and `physics_sim_fluid_quality_tests --scenario hose-wall-pooling` verifies hose-fed wall/floor pooling. | None. |

## Stage 5: Visualization And Editing

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R5.01 | Water and wall geometry are visually clear in motion. | Verified | `physics_sim_demo_regression` and `physics_sim_demo_regression_density` compare deterministic clean-frame and density-view captures of the demo scene after the PSIM-0042 flow fix. | None. |
| R5.02 | At least one useful debug visualization mode exists, such as particle view, density view, or surface view. | Verified | `physics_sim_ui_tests` verifies the visual-mode cycle helpers, and `src\main.cpp` renders surface, mixed, density, and particle views without changing simulation state. | None. |
| R5.03 | Overlay diagnostics show FPS, fixed timestep info, particle count, and at least one solver health metric. | Verified | `physics_sim_ui_tests` verifies overlay text generation for FPS, step, tick, particles, divergence, visual mode, and selected-fixture state. | None. |
| R5.04 | The user can zoom and pan the view. | Verified | `physics_sim_editor_tests` verifies viewport pan and zoom anchor behavior. | None. |
| R5.05 | The user can select, place, and remove fixtures from the scene. | Verified | `physics_sim_editor_tests` verifies fixture placement, selection, deletion, and scene-history rollback. | None. |
| R5.06 | The user can reset the scene without relaunching the app. | Verified | `docs\manual-verification-checklist.md` documents the reset behavior and `physics_sim_editor_tests` covers `SceneController::reset_scene`. | None. |
| R5.07 | Scene save and load work for walls and fixtures. | Verified | `physics_sim_scene_persistence_tests` verifies walls and two emitter kinds survive save/load/apply. | None. |
| R5.08 | Demo scenes can be loaded consistently and reproduce the same starting state. | Verified | `physics_sim_demo_regression` loads the committed demo scene and matches the golden frame. | None. |
| R5.09 | Regression capture exists for at least one golden scene. | Verified | `physics_sim_demo_regression`, `physics_sim_demo_regression_density`, and `physics_sim_demo_regression_surface` compare deterministic captures with committed clean, density, and surface baselines. | None. |

## Stage 6: Complete Testable Product

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R6.01 | A fresh checkout can be configured, built, tested, and run from documented commands on this machine. | Verified | `.\scripts\build.ps1`, `.\scripts\test.ps1`, and `.\scripts\run-smoke.ps1` completed successfully on 2026-06-04. | None. |
| R6.02 | The app can create or load a scene, emit water, interact with walls, and show expected contained-water behavior. | Verified | `physics_sim_demo_regression`, `physics_sim_demo_regression_density`, and `physics_sim_water_tests` cover scene load, visible emission flow, wall interaction, and contained-water behavior. | None. |
| R6.03 | Automated tests pass. | Verified | `.\scripts\test.ps1` passed 6/6 tests on 2026-06-04. | None. |
| R6.04 | Smoke tests pass. | Verified | `.\scripts\run-smoke.ps1` completed successfully on 2026-06-04. | None. |
| R6.05 | A repeatable end-to-end demo scene exists and can be used as a regression check. | Verified | `physics_sim_demo_regression` uses `scenes\demo_scene.pscene` and `regression\demo_scene_golden.bmp`. | None. |
| R6.06 | The core workflow is complete: launch app, draw or load walls, place a fixture, observe water behavior, reset, repeat. | Verified | `docs\manual-verification-checklist.md` documents the full launch/edit/simulate/reset loop, and `physics_sim_editor_tests` covers the controller behavior. | None. |
| R6.07 | There are no known blockers preventing the v1 experience from being used as a real interactive testbed. | Verified | `docs\manual-verification-checklist.md`, `physics_sim_demo_regression`, and `physics_sim_demo_regression_density` cover the remaining V1 workflow and acceptance paths. | None. |

## Stage 7: Editor UX And Fixture Authoring

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R7.01 | Users can select and delete individual fixtures without clearing the scene. | Verified | `physics_sim_editor_tests` verifies fixture selection, deletion, and fixture-history rollback without clearing walls or other emitters. | None. |
| R7.02 | A selected fixture inspector shows kind, position, direction, speed, emission rate, and enabled state. | Verified | `physics_sim_ui_tests` verifies overlay lines for the selected fixture kind, position, direction, speed, emission rate, and enabled state. | None. |
| R7.03 | Users can move and rotate selected fixtures while seeing immediate visual feedback. | Verified | `physics_sim_editor_tests` verifies selected-fixture movement and rotation, and the renderer highlights the selected emitter. | None. |
| R7.04 | Undo and redo work for wall painting, wall erasing, fixture placement, fixture deletion, and fixture edits. | Verified | `physics_sim_editor_tests` covers wall stroke undo/redo, fixture deletion undo/redo, movement undo/redo, and parameter-edit undo/redo. | None. |
| R7.05 | An in-app help or controls reference is available without leaving the simulation. | Verified | `docs\manual-verification-checklist.md` documents the `H` help-overlay check, and `src\main.cpp` toggles the in-app help overlay. | None. |
| R7.06 | The overlay or equivalent UI shows active tool, selected fixture state, and editable parameter values. | Verified | `physics_sim_ui_tests` verifies the overlay text for active tool, visual mode, selected fixture state, and editable defaults. | None. |
| R7.07 | Editor workflows have repeatable automated or manual verification evidence. | Verified | `physics_sim_editor_tests` and `physics_sim_ui_tests` now cover the core editor workflows and overlay state. | None. |

## Stage 8: Solver Correctness, Performance, And Determinism

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R8.01 | Particle-to-grid and grid-to-particle transfer behavior has focused tests beyond indirect integration coverage. | Verified | `physics_sim_water_tests` includes direct helper tests for scatter normalization and grid-to-particle sampling. | None. |
| R8.02 | Leak and retention acceptance thresholds are documented and tested for closed and U-shaped containers. | Verified | `physics_sim_water_tests` documents closed-box and U-container leak thresholds with zero escape tolerance, and `physics_sim_fluid_quality_tests --scenario hose-wall-pooling` verifies zero wall penetration and zero outflow. | None. |
| R8.03 | Inflow, outflow, particle count, or mass behavior is measured against explicit tolerances. | Verified | `physics_sim_water_tests` checks exact particle-count conservation against emitted inflow in the container fill tests. | None. |
| R8.04 | Particle lifecycle and out-of-domain cleanup are defined, tested, and visible in metrics. | Verified | `docs\water-lifecycle.md` documents the policy, `physics_sim_water_tests` covers out-of-domain cleanup and lifecycle metrics, and `WaterSimulation2D::metrics()` now includes active, emitted, removed, and outflow counts. | None. |
| R8.05 | Simulation replay and frame capture can be made deterministic for regression scenarios. | Verified | `--replay-file` loads deterministic replay scripts, `physics_sim_replay_script_tests` covers parsing and validation, and `scripts\verify-replay-suite.ps1` exercises replay-driven regression captures. | None. |
| R8.06 | Solver performance has a documented budget and a repeatable profiling or timing check. | Verified | `docs\performance-budget.md` documents the small stress and demo-grid budgets, and `.\scripts\measure-water-solver.ps1 -Tier All` passed on 2026-06-06 with live `small-container-stress` at `2.89s`, live `demo-grid-flow` at `0.22s`, offline `small-container-stress` at `15.67s`, and offline `demo-grid-flow` at `3.19s`. | None. |
| R8.07 | Long-run stress verification has a named command, duration, and stability threshold. | Verified | `physics_sim_water_tests` includes a 6000-step finite-value stress run with explicit thresholds for pressure, divergence, velocity, and particle finiteness. | None. |

## Stage 9: Sandbox Devices And Interactive Systems

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R9.01 | Scene data supports device types beyond walls and water emitters. | Verified | `SceneDocument` now stores gates, sensors, drains, pumps, and valves in addition to walls and water emitters, and `physics_sim_scene_persistence_tests` verifies the round-trip. | None. |
| R9.02 | Drains or sinks remove water in a controlled, inspectable way. | Verified | `physics_sim_device_runtime_tests` verifies a drain removes particles inside its region and increments removal metrics, and `physics_sim_scene_persistence_tests` covers drain save/load round-tripping. | None. |
| R9.03 | Pumps and valves can route or regulate water flow. | Verified | `physics_sim_device_runtime_tests` verifies pump acceleration and valve open/closed behavior, and `physics_sim_scene_persistence_tests` covers pump and valve save/load round-tripping. | None. |
| R9.04 | Gates or doors can be opened and closed by user input. | Verified | `physics_sim_editor_tests` and `physics_sim_device_runtime_tests` cover gate placement, collision, open/close toggling, and persistence. | None. |
| R9.05 | Sensors or triggers can react to water or device state. | Verified | `physics_sim_editor_tests` and `physics_sim_device_runtime_tests` cover sensor placement, water detection, active state, and objective metrics. | None. |
| R9.06 | A device palette or equivalent tool workflow lets users choose and place devices predictably. | Verified | `Tab`/`Shift+Tab` cycle the current tool palette, invalid emitter placement is rejected and previewed in red, and `physics_sim_tests` plus `physics_sim_editor_tests` cover the cycle order and placement rules. | None. |
| R9.07 | Devices persist through scene save/load and have regression or acceptance coverage. | Verified | `physics_sim_scene_persistence_tests` covers gate, sensor, drain, pump, and valve save/load/apply round-trips, and `physics_sim_device_runtime_tests` plus `physics_sim_scene_gallery_tests` cover the runtime acceptance path. | None. |

## Stage 10: Scene Library, Sharing, And Regression Coverage

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R10.01 | The scene format is versioned with a documented compatibility policy. | Verified | `docs\scene-format.md` documents version `1` support and rejection of unknown or malformed versions, and `physics_sim_scene_persistence_tests` covers the policy. | None. |
| R10.02 | Scenes can store metadata such as title, description, author, tags, and notes. | Verified | `SceneDocument` now carries `SceneMetadata`, `physics_sim_scene_persistence_tests` verifies metadata round-trips through save/load, and `docs\scene-metadata.md` documents the fields. | None. |
| R10.03 | Scenes can store or generate thumbnails for browsing and regression evidence. | Verified | `docs\scene-metadata.md` documents the sibling-BMP thumbnail policy, and the committed demo scene has a matching `scenes\demo_scene.thumb.bmp` sidecar. | None. |
| R10.04 | Users can browse and load multiple scenes without manually editing file paths. | Verified | `PgUp`/`PgDn` now cycle the curated scene gallery, the window title shows the current scene title, and `physics_sim_scene_gallery_tests` verifies the curated scenes load consistently. | None. |
| R10.05 | A scripted replay regression suite can exercise more than one scene or interaction path. | Verified | `scripts\verify-replay-suite.ps1` runs the `demo-add-directional` and `demo-add-omni` replay cases with distinct golden frames, and `scripts\verify-all.ps1` includes the suite in the local verification bundle. | None. |
| R10.06 | A demo-scene gallery covers the core sandbox behaviors. | Verified | The repository now includes curated scenes for free fall, U-container, hose-wall impact, omni spray, and a future-device placeholder, and `physics_sim_scene_gallery_tests` verifies that each scene and its thumbnail sidecar load. | None. |
| R10.07 | Scene import, invalid-scene handling, and save/load failures are tested or manually documented. | Verified | `physics_sim_scene_persistence_tests` covers invalid scene text, unsupported versions, malformed headers, and missing-file load failure. | None. |

## Stage 11: Sandbox Game Loop And Player Experience

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R11.01 | Creative mode has a coherent start, edit, simulate, reset, and continue flow. | Verified | The app starts on the default U-container scene, shows the current scene title and mode/tool state in-app, and the README/help overlay now document reset, retry, and gallery navigation for the creative loop. | None. |
| R11.02 | Optional challenge or objective mode can define goals without replacing creative play. | Verified | `scenes\objective_fill.pscene` defines a sensor-driven objective, `physics_sim_scene_gallery_tests` verifies the scene loads with the objective sensor, and `physics_sim_ui_tests` shows the objective status in the HUD and title. | None. |
| R11.03 | Session controls support reset, retry, pause, resume, and clear-scene workflows cleanly. | Verified | `src\main.cpp` now splits `R` into fluid-only reset and `F10` into retrying the current scene path, while `README.md` and the in-app help overlay document the distinct controls; `.\scripts\build.ps1`, `.\scripts\test.ps1`, and `.\scripts\run-smoke.ps1` passed. | None. |
| R11.04 | Water visualization moves beyond square particles where practical while preserving debug clarity. | Verified | `src\main.cpp` now draws blended circular particles in mixed mode while keeping density/debug modes separate, and `.\scripts\test.ps1` passes both `physics_sim_demo_regression` and `physics_sim_demo_regression_density` after the regression scripts force the repo-root working directory. | None. |
| R11.05 | User feedback such as sound, animation, or visual affordances reinforces important actions. | Verified | Save, load, reset, delete, and tool changes now flash a short `MSG` line in the debug overlay, and the behavior is documented in `README.md` and `docs\diagnostics.md`; `.\scripts\build.ps1`, `.\scripts\test.ps1`, and `.\scripts\run-smoke.ps1` passed. | None. |
| R11.06 | The app communicates scene state, tool state, and simulation state without relying only on the window title. | Verified | `physics_sim_ui_tests` verifies overlay lines for scene metrics, visual mode, active tool, and selected-fixture state. | None. |
| R11.07 | The default demo experience teaches the core sandbox loop through interaction rather than external explanation. | Verified | The default U-container scene now loads with the current scene title visible, the HUD/help overlay documents the core controls, and the curated gallery provides clear interaction examples in-app. | None. |

## Stage 12: Packaging, Diagnostics, And Project Automation

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R12.01 | A release package can be produced from a documented command. | Verified | `scripts\package-release.ps1` created `dist\physics-sim-release`, and the packaged `physics-sim.exe` launched from the package root with `--auto-exit-ms 1500` and exit code `0`. | None. |
| R12.02 | App logging or crash diagnostics capture enough context to debug runtime failures. | Verified | `docs\diagnostics.md` documents the log path and failure check, and `physics-sim.exe --log-file build\windows-x64\physics-sim-diagnostics.log --scene-path scenes\does-not-exist.pscene --auto-exit-ms 1500` wrote startup, renderer, load-failure, fallback, and shutdown log lines. | None. |
| R12.03 | User settings can persist preferences such as window size, overlay mode, and visual mode. | Verified | `docs\user-settings.md` documents settings format version `3`, `physics_sim_user_settings_tests` covers save/load and invalid/default handling for surface visual mode and solver profile, and startup logs include persisted visual/profile values. | None. |
| R12.04 | A one-command local verification bundle runs tracking, build, tests, smoke, and relevant regression checks. | Verified | `.\scripts\verify-all.ps1` passed on 2026-06-06 and ran tracking validation, build, tests, smoke, replay regression, and fluid-quality regression. | None. |
| R12.05 | The tracking validator checks epic structure, duplicate issue headings, invalid dependencies, and missing roadmap links. | Verified | `.\scripts\check-tracking.ps1` validates roadmap/progress/issue links, epic structure, duplicate issue headings, required fields, and dependency references. | None. |
| R12.06 | Architecture decision records or equivalent notes exist for major technical choices. | Verified | `docs\adr\README.md` and the initial solver, scene-format, and tracking ADRs are present, and `AGENTS.md` points agents to them. | None. |
| R12.07 | Backlog pruning, reprioritization, and implementation handoff conventions are documented. | Verified | `docs\TRACKING.md` now includes the canonical issue template, implementation handoff checklist, and backlog review cadence, and `AGENTS.md` references them. | None. |

## Stage 13: Physics Accuracy And Particle Interaction

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R13.01 | Fluid physics model, units, invariants, and equations are documented. | Verified | `docs\physics-accuracy-contract.md` and ADR 0006 define the model, units, invariants, and live/offline tiers; `.\scripts\check-tracking.ps1` and `.\scripts\test.ps1` passed on 2026-06-05. | None. |
| R13.02 | Particles carry physical mass, volume, density, and interaction diagnostics. | Verified | `FluidParticle` carries mass, volume, density, neighbor count, and APIC affine state; `classify_fluid_cells` computes deterministic volume fractions from particle volume. `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05. | None. |
| R13.03 | Particle-grid transfer conserves mass and momentum and supports APIC/FLIP. | Verified | `physics_sim_water_tests` covers APIC affine velocity helpers, runtime APIC affine update in the quality profile, mass-weighted scatter momentum conservation, and PIC/FLIP blending; `.\scripts\test.ps1` passed on 2026-06-06. | None. |
| R13.04 | Pressure projection enforces incompressibility with measured residuals. | Verified | Matrix-free PCG projection reports iterations, residuals, convergence, visible cells, pressure-active cells, RHS norm, solution norm, `dt`, and rest density; `physics_sim_water_tests`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\measure-water-solver.ps1 -Tier All` passed on 2026-06-06. | None. |
| R13.05 | Dense particle clusters redistribute through density constraints without mass loss. | Verified | `physics_sim_water_tests` and `physics_sim_fluid_quality_tests` verify the bounded density-correction primitive and the overcrowding scenario reduce density error while preserving mass, volume, and deterministic center-of-mass behavior. | PSIM-0086 tracks remaining high live-profile density-error behavior in stress scenarios. |
| R13.06 | World boundaries implement defined free-slip/no-slip material interaction. | Verified | `docs\boundary-interaction.md` defines swept-sample no-penetration, default free-slip, and damped tangential behavior; `physics_sim_water_tests`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-06. | Scene-authored wall materials remain future work. |
| R13.07 | Viscosity, surface tension, and free-surface behavior are modeled and tested. | Verified | `physics_sim_water_tests` and `physics_sim_fluid_quality_tests` cover bounded viscosity and surface-tension behavior, still-pool jitter, dam-break flow, and hose-wall stability; `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\verify-all.ps1` passed on 2026-06-05. | None. |
| R13.08 | Particle resampling and rendering preserve physical volume. | Verified | `physics_sim_water_tests`, `physics_sim_fluid_quality_tests`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\verify-demo-scene-surface.ps1`, `.\scripts\verify-replay-suite.ps1`, and `.\scripts\verify-all.ps1` pass with deterministic resampling and volume-fraction rendering baselines. | None. |
| R13.09 | Live and offline physics-quality budgets pass deterministically. | Verified | `physics_sim_solver_benchmark`, `.\scripts\measure-water-solver.ps1 -Tier All`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\verify-all.ps1` passed on 2026-06-06 with live and offline budgets inside `docs\performance-budget.md`. | PSIM-0086 tracks remaining high live-profile density-error and kinetic-energy quality concerns. |
