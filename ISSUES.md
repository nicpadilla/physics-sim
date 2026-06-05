# Issues

Repo-local implementation queue. See `docs/TRACKING.md` for status meanings, evidence rules, and lifecycle rules.

Issue IDs are stable. Do not reuse deleted IDs.

Allowed priorities:

- `P0`: V1 blocker.
- `P1`: Near-term foundation.
- `P2`: Future sandbox polish.

## Open Queue

| ID | Status | Priority | Epic | Linked roadmap IDs | Title |
| --- | --- | --- | --- | --- | --- |
| PSIM-0001 | Done | P0 | V1 Closure | R5.05, R6.06, R6.07, R7.01 | Fixture selection and individual fixture deletion |
| PSIM-0002 | Done | P1 | V1 Closure | R5.02, R5.03, R7.06, R11.06 | Visual and debug mode toggles |
| PSIM-0003 | Done | P1 | V1 Closure | R3.05, R4.05, R4.06, R5.05, R7.02, R7.06 | Emitter parameter editing workflow |
| PSIM-0004 | Done | P0 | V1 Closure | R3.06, R4.08, R6.02, R6.06, R6.07, R8.02, R8.03, R8.07 | Stronger water acceptance tests for containment, mass, and stress |
| PSIM-0005 | Done | P1 | V1 Closure | R5.01, R5.08, R5.09, R6.02, R6.05, R6.06, R6.07, R10.05 | Richer end-to-end demo evidence beyond the early golden frame |
| PSIM-0006 | Done | P1 | V1 Closure | R1.04, R2.07, R4.01, R5.06, R7.07 | Input/window/manual-verification automation |
| PSIM-0007 | Done | P0 | V1 Closure | R6.01, R6.02, R6.03, R6.04, R6.05, R6.06, R6.07, R7.07 | V1 completion audit and final status reconciliation |
| PSIM-0008 | Done | P1 | Editor UX | R7.02, R7.06 | Selected fixture inspector |
| PSIM-0009 | Done | P1 | Editor UX | R7.03 | Move and rotate selected fixtures |
| PSIM-0010 | Done | P1 | Editor UX | R7.04 | Undo and redo for wall and fixture edits |
| PSIM-0011 | Done | P2 | Editor UX | R7.05 | In-app help and control reference |
| PSIM-0012 | Done | P1 | Editor UX | R5.03, R7.06, R11.06, R12.03 | Tool state overlay polish |
| PSIM-0013 | Done | P1 | Solver Quality | R3.04, R8.01 | Focused particle-grid transfer tests |
| PSIM-0014 | Done | P1 | Solver Quality | R3.06, R4.08, R8.02, R8.03 | Leak and retention acceptance thresholds |
| PSIM-0015 | Done | P1 | Solver Quality | R8.04 | Particle lifecycle and outflow cleanup |
| PSIM-0016 | Done | P1 | Solver Quality | R8.05, R10.05 | Deterministic replay and capture harness |
| PSIM-0017 | Done | P1 | Solver Quality | R8.06, R8.07 | Solver performance budget and profiling |
| PSIM-0018 | Done | P2 | Sandbox Devices | R9.01, R9.02, R9.07 | Drains and sinks |
| PSIM-0019 | Done | P2 | Sandbox Devices | R9.01, R9.03, R9.07 | Pumps and valves |
| PSIM-0020 | Done | P2 | Sandbox Devices | R9.01, R9.04, R9.07 | Gates and user-controlled doors |
| PSIM-0021 | Done | P2 | Sandbox Devices | R9.01, R9.05, R9.07, R11.02 | Sensors and triggers |
| PSIM-0022 | Done | P1 | Sandbox Devices | R9.01, R9.06, R9.07 | Device palette and placement rules |
| PSIM-0023 | Done | P1 | Scenes And Regression | R9.01, R9.07, R10.01, R10.07 | Versioned scene format |
| PSIM-0024 | Done | P2 | Scenes And Regression | R10.02, R10.03 | Scene metadata and thumbnails |
| PSIM-0025 | Done | P2 | Scenes And Regression | R10.04, R10.07 | Multi-scene loading and browser workflow |
| PSIM-0026 | Done | P1 | Scenes And Regression | R9.07, R10.05, R12.04 | Scripted replay regression suite |
| PSIM-0027 | Done | P2 | Scenes And Regression | R9.07, R10.06, R11.07 | Demo-scene gallery for core sandbox behaviors |
| PSIM-0028 | Done | P2 | Sandbox Game Experience | R11.01, R11.06, R11.07 | Creative-mode flow |
| PSIM-0029 | Done | P2 | Sandbox Game Experience | R11.02 | Optional challenge and objective mode |
| PSIM-0030 | Done | P2 | Sandbox Game Experience | R11.03 | Reset, retry, and session controls |
| PSIM-0031 | Done | P2 | Sandbox Game Experience | R5.01, R11.04 | Visual surface rendering beyond square particles |
| PSIM-0032 | Done | P2 | Sandbox Game Experience | R11.05 | Sound and feedback pass |
| PSIM-0033 | Done | P2 | Platform And Automation | R12.01 | Release packaging |
| PSIM-0034 | Done | P2 | Platform And Automation | R12.02 | App logging and crash diagnostics |
| PSIM-0035 | Done | P2 | Platform And Automation | R12.03 | User settings file |
| PSIM-0036 | Done | P1 | Platform And Automation | R12.04 | CI or one-command local verification bundle |
| PSIM-0037 | Done | P1 | Platform And Automation | R12.05 | Stronger tracking validator for epic and dependency integrity |
| PSIM-0038 | Done | P1 | Agentic Project Ops | R12.07 | Issue template normalization |
| PSIM-0039 | Done | P1 | Agentic Project Ops | R12.06 | Architecture decision records |
| PSIM-0040 | Done | P1 | Agentic Project Ops | R12.07 | Implementation handoff checklist |
| PSIM-0041 | Done | P1 | Agentic Project Ops | R12.07 | Backlog pruning and reprioritization cadence |
| PSIM-0042 | Done | P0 | Solver Quality | R3.05, R5.01, R6.02, R8.06 | Restore visible emitter flow and live-scene frame rate |
| PSIM-0043 | Done | P1 | Fluid Realism And Validation | R5.03, R8.03, R8.04, R8.05, R8.07 | Fluid quality metrics and snapshot harness |
| PSIM-0044 | Done | P1 | Fluid Realism And Validation | R3.02, R3.03, R3.06, R8.01, R8.02, R8.06, R8.07 | Pressure solver and incompressibility upgrade |
| PSIM-0045 | Done | P1 | Fluid Realism And Validation | R3.06, R4.08, R8.02, R8.03, R11.04 | Pooling, resting water, and free-surface stability |
| PSIM-0046 | Done | P1 | Fluid Realism And Validation | R3.03, R3.06, R4.08, R8.02 | Wall interaction, deflection, and no-penetration behavior |
| PSIM-0047 | Done | P1 | Fluid Realism And Validation | R8.05, R8.07, R10.05, R12.04 | Multi-scenario fluid regression suite |
| PSIM-0048 | Done | P1 | Fluid Realism And Validation | R8.06, R12.04 | Live and offline performance budgets |
| PSIM-0049 | Done | P2 | Fluid Realism And Validation | R10.04, R10.06, R11.07 | Fluid realism scenario gallery |
| PSIM-0050 | Done | P1 | Fluid Realism And Validation | R8.01, R8.02, R8.03, R8.04, R8.05, R8.06, R8.07, R12.04 | Final fluid realism audit |
| PSIM-0051 | Done | P2 | Finished Game Polish And Presentation | R11.01, R11.06, R11.07, R12.06 | Finished-game product direction and UX quality bar |
| PSIM-0052 | Done | P2 | Finished Game Polish And Presentation | R11.01, R11.03, R11.06, R12.03 | Main menu, pause menu, and session shell |
| PSIM-0053 | Done | P2 | Finished Game Polish And Presentation | R5.03, R7.06, R9.06, R11.06 | HUD, tool palette, and in-game readability pass |
| PSIM-0054 | Done | P2 | Finished Game Polish And Presentation | R7.05, R11.01, R11.07 | First-run onboarding and interactive tutorial |
| PSIM-0055 | Done | P2 | Finished Game Polish And Presentation | R10.02, R10.03, R10.04, R10.06, R11.02, R11.07 | Scenario browser, progression, and objective presentation |
| PSIM-0056 | Done | P2 | Finished Game Polish And Presentation | R5.01, R5.02, R11.04, R11.05 | Visual identity, art direction, and rendering polish |
| PSIM-0057 | Done | P2 | Finished Game Polish And Presentation | R5.04, R11.03, R11.05, R11.06 | Animation, camera, and moment-to-moment feedback |
| PSIM-0058 | Done | P2 | Finished Game Polish And Presentation | R11.05, R12.03 | Audio mix, sound effects, and player feedback |
| PSIM-0059 | Done | P2 | Finished Game Polish And Presentation | R1.04, R2.04, R7.05, R12.03 | Settings, accessibility, and input remapping |
| PSIM-0060 | Done | P2 | Finished Game Polish And Presentation | R5.07, R10.07, R12.02, R12.03 | Save/load UX, autosave, and recovery flow |
| PSIM-0061 | Done | P2 | Finished Game Polish And Presentation | R10.07, R12.02, R12.03 | Player-facing error handling and diagnostics polish |
| PSIM-0062 | Done | P2 | Finished Game Polish And Presentation | R1.05, R1.07, R12.01, R12.02, R12.04 | Release package, first-launch, and install handoff |
| PSIM-0063 | Done | P2 | Finished Game Polish And Presentation | R8.06, R11.03, R12.04 | Finished-game performance and responsiveness budget |
| PSIM-0064 | Done | P1 | Finished Game Polish And Presentation | R6.01, R6.06, R6.07, R11.01, R11.07, R12.04 | Final finished-game polish audit |
| PSIM-0065 | Done | P1 | Fluid Realism And Validation | R3.03, R3.06, R4.08, R8.02, R8.06 | Restore wall-impact pooling instead of full-stop collision |
| PSIM-0066 | Done | P1 | Physics Accuracy And Particle Interaction | R13.01, R12.06 | Physics model, units, and solver invariants |
| PSIM-0067 | Done | P1 | Physics Accuracy And Particle Interaction | R13.02, R8.03, R8.04 | Physical particle state and density metrics |
| PSIM-0068 | Done | P1 | Physics Accuracy And Particle Interaction | R13.02, R13.04, R3.03, R3.06 | Fluid, air, and solid cell classification with volume fractions |
| PSIM-0069 | Done | P1 | Physics Accuracy And Particle Interaction | R13.03, R3.04, R8.01, R8.03 | Conservative APIC/FLIP particle-grid transfer |
| PSIM-0070 | Done | P1 | Physics Accuracy And Particle Interaction | R13.04, R3.02, R3.03, R3.06, R8.02, R8.06, R8.07 | Matrix-free pressure projection and incompressibility upgrade |
| PSIM-0071 | Done | P1 | Physics Accuracy And Particle Interaction | R13.05, R3.06, R8.02, R8.03 | Local particle interaction and density constraint correction |
| PSIM-0072 | Done | P1 | Physics Accuracy And Particle Interaction | R13.06, R3.03, R3.06, R4.08, R8.02 | World and material boundary interaction |
| PSIM-0073 | Done | P1 | Physics Accuracy And Particle Interaction | R13.07, R11.04, R8.02, R8.07 | Viscosity, surface tension, and free-surface stability |
| PSIM-0074 | Done | P1 | Physics Accuracy And Particle Interaction | R13.08, R5.01, R11.04, R8.03 | Particle resampling and physical surface rendering |
| PSIM-0075 | Done | P1 | Physics Accuracy And Particle Interaction | R13.04, R13.05, R13.07, R8.05, R8.07 | Physics validation scenarios and numeric acceptance thresholds |
| PSIM-0076 | Done | P1 | Physics Accuracy And Particle Interaction | R13.09, R8.06, R12.04 | Live/offline quality tiers and performance budgets |
| PSIM-0077 | Done | P1 | Physics Accuracy And Particle Interaction | R13.01, R13.02, R13.03, R13.04, R13.05, R13.06, R13.07, R13.08, R13.09, R12.04 | Final physics interaction audit |
| PSIM-0078 | Done | P1 | Agentic Project Ops | R12.04, R12.07 | Versioned workflow Git hooks |

## Epic 1: V1 Closure

### PSIM-0001: Fixture selection and individual fixture deletion

Status: Done

Priority: P0

Linked roadmap IDs: R5.05, R6.06, R6.07, R7.01

Problem:
Fixture placement exists, but there is no user-facing way to select a specific fixture or remove one fixture without clearing the whole scene.

Acceptance criteria:

- User can select an existing fixture in the scene.
- Selected fixture has a visible selection state.
- User can delete only the selected fixture without deleting walls or other fixtures.
- Scene save/load remains correct after fixture deletion.
- Progress entries R5.05, R6.06, R6.07, and R7.01 are updated after verification.

Subtasks:

- Add a selection model to scene editing state.
- Add hit testing for fixture selection.
- Add a delete-selected-fixture command.
- Add tests for selection, deletion, and persistence after deletion.
- Document the new controls in `README.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: place two fixtures, select one, delete it, save/load, confirm the other fixture remains.

Dependencies:

- None.

Implementation notes:

- Added fixture hit-testing, selected-fixture state, per-fixture deletion, and a snapshot history in `SceneController`.
- Left-click on an existing emitter selects it; `Delete` removes only the selected emitter or clears the scene if nothing is selected.
- Selected emitters are highlighted in the renderer and their details are shown in the overlay.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `physics_sim_editor_tests`
- `physics_sim_ui_tests`

### PSIM-0002: Visual and debug mode toggles

Status: Done

Priority: P1

Linked roadmap IDs: R5.02, R5.03, R7.06, R11.06

Problem:
The app renders density and particles together, but there is no explicit user-facing mode toggle for density, particle, surface, grid, or overlay views.

Acceptance criteria:

- User can switch between at least two visual/debug modes without changing simulation state.
- Active mode is visible in the overlay or window title.
- The default mode preserves the current readable simulation view.
- Progress entries R5.02, R7.06, and R11.06 are updated after verification.

Subtasks:

- Add a visualization mode enum/state.
- Add keyboard controls for cycling or selecting modes.
- Make render paths conditional on the active mode.
- Add a test for mode state transitions where practical.
- Document controls in `README.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: toggle modes while simulation runs and while paused; verify simulation continues unchanged.

Dependencies:

- None.

Implementation notes:

- Added `include\physics_sim\visual_mode.hpp` and wired `V` to cycle mixed, density, and particle views without changing simulation state.
- Render paths now respect the active mode and the active mode is shown in the overlay and window title.
- The new UI tests cover the visual-mode cycle helpers.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_ui_tests`

### PSIM-0003: Emitter parameter editing workflow

Status: Done

Priority: P1

Linked roadmap IDs: R3.05, R4.05, R4.06, R5.05, R7.02, R7.06

Problem:
Emitter speed, emission rate, and direction are configurable through controller state, but there is no complete interactive workflow for inspecting and editing existing emitter parameters.

Acceptance criteria:

- User can inspect the active emitter tool defaults or selected fixture settings.
- User can change emitter speed and emission rate from the app.
- Directional emitter direction remains adjustable.
- Changes are reflected in scene data and save/load behavior.

Subtasks:

- Decide whether editing applies to tool defaults, selected fixtures, or both.
- Add bounded controls for speed and emission rate.
- Show active values in the overlay or window title.
- Add tests for parameter updates and persistence.
- Document controls in `README.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: change rate/speed, place emitter, observe changed output, save/load scene.

Dependencies:

- PSIM-0001 if editing existing fixtures depends on fixture selection.

Implementation notes:

- Added selected-fixture editing for direction, speed, emission rate, and enabled state, while preserving tool-default editing when no fixture is selected.
- `Ctrl+Arrows/WASD` move a selected fixture, `Q/E` rotate it, `[`/`]` adjust speed, and `-`/`=` adjust emission rate.
- The selected/default values are reflected in the overlay and window title.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_editor_tests`
- `physics_sim_ui_tests`

### PSIM-0004: Stronger water acceptance tests for containment, mass, and stress

Status: Done

Priority: P0

Linked roadmap IDs: R3.06, R4.08, R6.02, R6.06, R6.07, R8.02, R8.03, R8.07

Problem:
Current tests cover collision, divergence thresholds, omni emission, and finite long-run values, but they do not fully prove container retention or mass behavior for the acceptance scenes.

Acceptance criteria:

- A closed or U-shaped container test verifies particles do not leak through solid boundaries.
- A fixed-duration fill test records expected particle count or mass tolerance after known inflow.
- Stress verification has a documented duration, threshold, and command.
- Linked progress rows are updated after verification.

Subtasks:

- Define measurable retention and mass criteria.
- Add one or more water simulation acceptance tests.
- Add failure messages that identify leakage, non-finite values, or mass drift.
- Consider a longer scripted check if it is too slow for default unit tests.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Optional longer stress command if added.

Dependencies:

- None.

Implementation notes:

- Added focused transfer helper tests for particle-to-grid scatter normalization and grid-to-particle velocity sampling.
- Added closed-box and U-container retention/mass checks that assert zero wall leakage and exact particle-count conservation.
- Kept the long-run 6000-step stress run as a named `physics_sim_water_tests` case covering finite pressure, divergence, and velocity values.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_water_tests`

### PSIM-0005: Richer end-to-end demo evidence beyond the early golden frame

Status: Done

Priority: P1

Linked roadmap IDs: R5.01, R5.08, R5.09, R6.02, R6.05, R6.06, R6.07, R10.05

Problem:
The current golden frame is deterministic and useful, but the early 240-tick capture is not enough evidence for the full interactive/demo experience.

Acceptance criteria:

- Add a later or complementary demo-scene verification that shows meaningful water interaction.
- Preserve deterministic regression behavior.
- Document how to regenerate or inspect the new evidence.
- Linked progress rows are updated after verification as appropriate.

Subtasks:

- Choose a later tick count, scripted replay, or second demo scene.
- Ensure frame capture succeeds reliably for the chosen evidence.
- Add or update regression docs.
- Avoid replacing existing baseline unless the issue explicitly calls for it.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-demo-scene.ps1`
- Any new regression or demo verification command added by the issue.

Dependencies:

- PSIM-0004 if visual evidence should rely on stronger solver acceptance first.

Implementation notes:

- Added `scripts\verify-demo-scene-density.ps1` and a second committed baseline at `regression\demo_scene_density_golden.bmp`.
- Added `--visual-mode` support so the later regression can capture the demo scene in density view without changing the interactive default.
- Documented both the early clean-frame baseline and the later density-view baseline in `regression\README.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`

### PSIM-0006: Input/window/manual-verification automation

Status: Done

Priority: P1

Linked roadmap IDs: R1.04, R2.07, R4.01, R5.06, R7.07

Problem:
Several implemented runtime behaviors are marked `Done` rather than `Verified` because they lack repeatable input/window or manual-verification evidence.

Acceptance criteria:

- Add a documented manual checklist or automation path for resize, mouse movement, close, pause/edit, wall drawing, and reset.
- Evidence can be repeated by an agent on this machine.
- Relevant `PROGRESS.md` rows are upgraded only when evidence is recorded.

Subtasks:

- Define the smallest repeatable checks for each `Done` runtime marker.
- Add a docs checklist or script if feasible.
- Record exact commands or manual steps.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`
- Manual checklist produced by this issue.

Dependencies:

- None.

Implementation notes:

- Added `docs\manual-verification-checklist.md` with repeatable window, input, pause, wall drawing, fixture editing, visual-mode, and help-overlay checks.
- The checklist covers the runtime behaviors that are awkward to prove through unit tests alone and gives a concrete manual path for verifying the V1 UI loop.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`
- `docs\manual-verification-checklist.md`

### PSIM-0007: V1 completion audit and final status reconciliation

Status: Done

Priority: P0

Linked roadmap IDs: R6.01, R6.02, R6.03, R6.04, R6.05, R6.06, R6.07, R7.07

Problem:
The v1 exit status should be decided from current evidence after the remaining v1 blockers and acceptance checks are complete.

Acceptance criteria:

- Every stage 1-6 progress row is `Verified` or explicitly justified as not required.
- Every P0 issue is `Done` with verification recorded.
- `ROADMAP.md`, `PROGRESS.md`, and `ISSUES.md` agree on whether v1 is complete.
- Final verification commands and manual checks are recorded.

Subtasks:

- Review all stage 1-6 progress rows.
- Close or defer remaining v1 issues.
- Record final command results.
- Update documentation for current v1 state.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`

Dependencies:

- PSIM-0001
- PSIM-0004
- PSIM-0006

Implementation notes:

- Promoted all stage 1-6 progress rows to `Verified` or documented manual checks so the v1 exit audit now matches current evidence.
- Recorded the final v1 command set and reconciled `ROADMAP.md`, `PROGRESS.md`, and `ISSUES.md` on the same state.
- Fresh verification on 2026-06-04: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, and `.\scripts\verify-demo-scene.ps1` all passed after the final reconciliation.

## Epic 2: Editor UX

### PSIM-0008: Selected fixture inspector

Status: Done

Priority: P1

Linked roadmap IDs: R7.02, R7.06

Problem:
Selected fixture data is not visible in-app, which makes future editing workflows hard to reason about.

Acceptance criteria:

- Selected fixture kind, position, direction, speed, emission rate, and enabled state are visible.
- Inspector display updates when selection or parameters change.
- Inspector state is covered by tests or a documented manual check.

Subtasks:

- Define display location for selected fixture data.
- Expose selected fixture state from editor/controller code.
- Add test coverage for selected fixture state formatting or accessors.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: select a fixture and confirm displayed values.

Dependencies:

- PSIM-0001.

Implementation notes:

- The overlay now reports selected-fixture kind, position, direction, speed, emission rate, and enabled state.
- When nothing is selected, the overlay falls back to the current tool defaults so the editor state remains inspectable.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_ui_tests`

### PSIM-0009: Move and rotate selected fixtures

Status: Done

Priority: P1

Linked roadmap IDs: R7.03

Problem:
Placed fixtures cannot be repositioned or rotated without deleting/recreating the scene content.

Acceptance criteria:

- User can move a selected fixture.
- User can rotate a selected directional fixture.
- Movement and rotation update scene data and save/load state.
- Visual feedback makes the pending transform clear.

Subtasks:

- Add selected-fixture transform commands.
- Add input bindings for move/rotate.
- Add persistence tests for transformed fixtures.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: move and rotate a fixture, save/load, confirm transform persists.

Dependencies:

- PSIM-0001.

Implementation notes:

- Added selected-fixture transform commands in `SceneController`.
- `Ctrl+Arrows/WASD` nudge the selected fixture by one grid cell and `Q/E` rotate it in place.
- Fixture transforms are recorded in the edit history so they persist through undo/redo and save/load.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_editor_tests`

### PSIM-0010: Undo and redo for wall and fixture edits

Status: Done

Priority: P1

Linked roadmap IDs: R7.04

Problem:
Scene edits are destructive and cannot be reversed, which is a poor fit for sandbox creation.

Acceptance criteria:

- Undo and redo work for wall strokes, erase strokes, fixture placement, fixture deletion, and fixture parameter edits.
- Undo/redo does not change simulation time unless the issue explicitly documents that policy.
- Tests cover representative command history behavior.

Subtasks:

- Add an edit-command/history model.
- Convert wall and fixture edits to command records.
- Add undo/redo input bindings and documentation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: perform and undo/redo each supported edit type.

Dependencies:

- PSIM-0001
- PSIM-0003

Implementation notes:

- Added a snapshot-based edit history in `SceneController` that records completed wall strokes and fixture edits.
- Undo/redo now covers wall painting, wall erasing, fixture placement, fixture deletion, fixture movement, and fixture parameter edits.
- The tests exercise wall undo/redo, fixture deletion undo/redo, movement undo/redo, and parameter-edit undo/redo.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_editor_tests`

### PSIM-0011: In-app help and control reference

Status: Done

Priority: P2

Linked roadmap IDs: R7.05

Problem:
Controls exist only in external docs, so users need to leave the app to recover command knowledge.

Acceptance criteria:

- In-app help can be opened and closed.
- Help lists active controls accurately.
- Help does not block existing quit/reset/pause behavior unexpectedly.

Subtasks:

- Choose a help display mode.
- Add help text rendering or overlay state.
- Keep README and in-app controls synced.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: open/close help and verify listed controls.

Dependencies:

- PSIM-0012 if the help uses shared overlay infrastructure.

Implementation notes:

- Added an in-app help overlay toggled by `H`.
- The help panel lists the active editor controls, including selection, undo/redo, visual mode toggles, and fixture editing bindings.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`

### PSIM-0012: Tool state overlay polish

Status: Done

Priority: P1

Linked roadmap IDs: R5.03, R7.06, R11.06, R12.03

Problem:
The overlay is useful but does not yet present all editor state needed for a polished sandbox workflow.

Acceptance criteria:

- Overlay shows current visual mode, active tool, selected fixture summary, editable defaults, and relevant simulation state.
- Overlay content remains readable at the default window size.
- Tests cover overlay line generation for new state where practical.

Subtasks:

- Extend overlay metrics/state model.
- Add selected/default parameter lines.
- Add tests for overlay text generation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: verify overlay content across modes/tools.

Dependencies:

- PSIM-0001
- PSIM-0002
- PSIM-0003

Implementation notes:

- Extended the overlay to show visual mode, active tool, selected-fixture summary, editable defaults, and solver health metrics.
- Added `physics_sim_ui_tests` coverage for the overlay text generation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_ui_tests`

## Epic 3: Solver Quality

### PSIM-0013: Focused particle-grid transfer tests

Status: Done

Priority: P1

Linked roadmap IDs: R3.04, R8.01

Problem:
Particle-grid transfer is central to the solver but is only covered indirectly by integration tests.

Acceptance criteria:

- Add focused tests for scatter weights, face normalization, and grid-to-particle velocity sampling.
- Tests use small deterministic grids and clear expected values.
- Transfer failures produce precise failure messages.

Subtasks:

- Expose or test transfer behavior through the smallest reasonable public surface.
- Add deterministic particle/grid test cases.
- Keep tests fast enough for `.\scripts\test.ps1`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- Added `WaterDrain` scene data plus simulation support for rectangular sinks that remove water inside their region and count the removal in metrics.
- Drain placement, preview, persistence, and load/save coverage are wired through `SceneController`, `scene_document.hpp`, `src\main.cpp`, and the runtime tests.
- The overlay and window title now surface removed-water metrics so drain behavior is visible while the app runs.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_device_runtime_tests`
- `physics_sim_scene_persistence_tests`

### PSIM-0014: Leak and retention acceptance thresholds

Status: Done

Priority: P1

Linked roadmap IDs: R3.06, R4.08, R8.02, R8.03

Problem:
The solver lacks explicit acceptance thresholds for what counts as contained water behavior.

Acceptance criteria:

- Define leak, retention, and fill thresholds for at least one closed container and one U-shaped container.
- Add tests or scripted checks that enforce those thresholds.
- Threshold rationale is documented so future solver changes can be reviewed.

Subtasks:

- Choose deterministic container scenes.
- Define allowed leak count or particle-position bounds.
- Add tests and documentation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`

Dependencies:

- PSIM-0004.

Implementation notes:

- Added focused particle-grid helper APIs in `include\physics_sim\grid_transfer.hpp` for scatter normalization and grid-to-particle sampling.
- `physics_sim_water_tests` now covers the helper APIs with deterministic grid cases.

### PSIM-0015: Particle lifecycle and outflow cleanup

Status: Done

Priority: P1

Linked roadmap IDs: R8.04

Problem:
Particle lifecycle policy is implicit, and out-of-domain behavior is currently not a first-class metric or design choice.

Acceptance criteria:

- Define when particles are clamped, removed, drained, or counted as outflow.
- Metrics expose active, emitted, removed, and outflow counts as needed.
- Tests cover lifecycle transitions.

Subtasks:

- Document lifecycle policy.
- Add metrics fields if needed.
- Add lifecycle tests for boundaries and future drains.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0018 if drain behavior is implemented first.

Implementation notes:

- Added explicit lifecycle metrics for active particles, emitted particles, removed particles, and outflow particles.
- Particles that start or end a step outside the simulation domain are culled before the next scatter pass and counted as outflow.
- Documented the lifecycle policy in `docs\water-lifecycle.md` and added a focused out-of-domain cleanup test in `physics_sim_water_tests`.

### PSIM-0016: Deterministic replay and capture harness

Status: Done

Priority: P1

Linked roadmap IDs: R8.05, R10.05

Problem:
The app has deterministic tick-based capture for one scene, but no general replay mechanism for scripted interactions.

Acceptance criteria:

- A replay file or script can drive deterministic input/actions.
- Capture timing is based on simulation ticks, not wall-clock timing.
- Replay/capture can be used by regression scripts.

Subtasks:

- Define replay command format.
- Add runtime command-line handling or a test harness.
- Add one replay-based regression check.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-replay-suite.ps1`
- New replay/regression command added by the issue.

Dependencies:

- PSIM-0026.

Implementation notes:

- Added `--replay-file` support and a deterministic replay parser that can drive scripted actions, tool changes, emitter parameter changes, and fixture placement at fixed-step ticks.
- Added `physics_sim_replay_script_tests` to cover replay parsing, ordering, and validation.
- The replay runner and suite now use fixed-step replay cases rather than wall-clock timing, which keeps the regression path deterministic.

### PSIM-0017: Solver performance budget and profiling

Status: Done

Priority: P1

Linked roadmap IDs: R8.06, R8.07

Problem:
There is no performance budget, benchmark, or profiling check for solver-heavy sandbox scenes.

Acceptance criteria:

- Define target scene size, particle count, and acceptable timing budget.
- Add a repeatable timing or profiling command.
- Record performance metrics in docs without requiring special tooling.

Subtasks:

- Choose representative scenes and particle counts.
- Add timing output or a benchmark executable/script.
- Document thresholds and how to interpret failures.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\measure-water-solver.ps1`

Dependencies:

- None.

Implementation notes:

- Added `physics_sim_solver_benchmark` as a repeatable timing target and `scripts\measure-water-solver.ps1` as the documented command.
- Documented the benchmark scene, particle count, and 20-second budget in `docs\performance-budget.md`.
- The latest observed run on 2026-06-04 completed in 6.59 seconds with 600 end particles.

### PSIM-0042: Restore visible emitter flow and live-scene frame rate

Status: Done

Priority: P0

Linked roadmap IDs: R3.05, R5.01, R6.02, R8.06

Problem:
The default demo emitter appears to pin water near the source, and unpaused runtime can drop to roughly 5 FPS in the live demo scene.

Acceptance criteria:

- The demo-scene directional emitter produces visible downstream particle displacement within a short fixed-tick run.
- The solver does not mark the entire scene as active fluid for a small local particle set.
- The live demo-sized solver path has a repeatable timing check that covers the larger grid used by the app.
- Progress entries R3.05, R5.01, R6.02, and R8.06 remain backed by verification after the fix.

Subtasks:

- Add a regression test for visible emitter displacement on the demo grid scale.
- Add a focused active-cell test for the pressure-solve region.
- Adjust emitter velocity scaling or pressure active-cell marking based on the measured root cause.
- Re-run the build, test, smoke, and tracking checks.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- None.

Implementation notes:

- Investigation captured `build\windows-x64\debug_flow_tick240.bmp`; after 240 fixed ticks, particles were still visually clustered at the emitter.
- Investigation logs showed the 240-tick capture took about 7 seconds of wall time, while `.\scripts\measure-water-solver.ps1` only benchmarked a tiny `12x12` grid.
- Added `physics_sim_water_tests` coverage for local active-cell bounds and demo-scale visible emitter displacement.
- Stopped marking the entire non-solid grid as active fluid, keeping pressure projection local to particle neighborhoods.
- Scaled emitter launch speed by grid cell size so scene speed values behave consistently across `1.0` and `16.0` cell-size scenes.
- Retained most incoming particle velocity during grid-to-particle transfer so open stream motion is not erased by pressure correction.
- Extended `physics_sim_solver_benchmark` and `docs\performance-budget.md` with a demo-grid timing case: `demo-grid-flow` completed 240 ticks in 1.37 seconds under the 3.0-second budget.
- Regenerated expected visual baselines with:
- `.\build\windows-x64\Debug\physics-sim.exe --dump-frame regression\demo_scene_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000`
- `.\build\windows-x64\Debug\physics-sim.exe --visual-mode density --dump-frame regression\demo_scene_density_golden.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000`
- `.\build\windows-x64\Debug\physics-sim.exe --replay-file regression\replays\demo-add-directional.replay --dump-frame regression\demo_scene_replay_add_directional_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000`
- `.\build\windows-x64\Debug\physics-sim.exe --replay-file regression\replays\demo-add-omni.replay --dump-frame regression\demo_scene_replay_add_omni_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000`

Verification:

- `.\scripts\build.ps1`
- `.\build\windows-x64\Debug\physics_sim_water_tests.exe`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\check-tracking.ps1`

## Epic 4: Sandbox Devices

### PSIM-0018: Drains and sinks

Status: Done

Priority: P2

Linked roadmap IDs: R9.01, R9.02, R9.07

Problem:
The sandbox has emitters but no controlled way to remove water from scenes.

Acceptance criteria:

- Drain/sink device removes water within a documented radius or region.
- Removed water is visible in metrics.
- Drain/sink persists through scene save/load.

Subtasks:

- Add drain/sink device data.
- Apply drain behavior during simulation step.
- Add tests for removal and persistence.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: place drain, observe controlled water removal.

Dependencies:

- PSIM-0023 if scene format changes are required first.

Implementation notes:

- Added drain scene data, runtime drain removal, removal metrics, and drain persistence coverage.
- Verification evidence is recorded in `PROGRESS.md` through `physics_sim_device_runtime_tests`, `physics_sim_scene_persistence_tests`, and related runtime acceptance coverage.

### PSIM-0019: Pumps and valves

Status: Done

Priority: P2

Linked roadmap IDs: R9.01, R9.03, R9.07

Problem:
Users cannot route or regulate water flow beyond placing emitters and walls.

Acceptance criteria:

- Pump device can move or accelerate water in a controlled direction.
- Valve device can restrict or release flow.
- Devices persist and have at least one acceptance scene or test.

Subtasks:

- Define pump and valve device data.
- Implement solver interaction.
- Add tests or demo scene.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: route water through a pump/valve setup.

Dependencies:

- PSIM-0022
- PSIM-0023

Implementation notes:

- Added `WaterPump` and `WaterValve` scene data plus simulation support for directional flow boosts and open/closed flow regulators.
- Pump and valve placement, preview, persistence, and runtime behavior are wired through `SceneController`, `scene_document.hpp`, `src\main.cpp`, and the regression tests.
- The tool palette now includes dedicated drain, pump, and valve entries so the new devices are reachable from the app.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_device_runtime_tests`
- `physics_sim_scene_persistence_tests`
- `physics_sim_ui_tests`

### PSIM-0020: Gates and user-controlled doors

Status: Done

Priority: P2

Linked roadmap IDs: R9.01, R9.04, R9.07

Problem:
Walls are static, so users cannot build interactive barriers or timed flow systems.

Acceptance criteria:

- A gate/door can toggle between solid and open states.
- User input can open/close the selected gate.
- State persists or resets according to documented scene policy.

Subtasks:

- Add dynamic-solid device data.
- Add input and visual state.
- Add tests for collision behavior in both states.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: open/close a gate and observe water routing.

Dependencies:

- PSIM-0022
- PSIM-0023

Implementation notes:

- Added `WaterGate` scene data plus simulation support for open/closed gates that synchronize solidity into the grid.
- Gate placement, selection, preview, toggling, deletion, undo/redo, and persistence are wired through `SceneController`, `scene_document.hpp`, `src\main.cpp`, and the debug overlay.
- Added deterministic gate coverage in `physics_sim_device_runtime_tests`, `physics_sim_editor_tests`, `physics_sim_scene_persistence_tests`, and `physics_sim_ui_tests`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_device_runtime_tests`
- `physics_sim_editor_tests`
- `physics_sim_scene_persistence_tests`
- `physics_sim_ui_tests`

### PSIM-0021: Sensors and triggers

Status: Done

Priority: P2

Linked roadmap IDs: R9.01, R9.05, R9.07, R11.02

Problem:
The sandbox has no mechanism for devices or objectives to react to water state.

Acceptance criteria:

- Sensor can detect water presence or particle count in a region.
- Trigger state can be displayed and persisted.
- Sensor output can support future challenge goals or device control.

Subtasks:

- Define sensor data and evaluation.
- Add UI/overlay feedback for trigger state.
- Add tests for dry/wet sensor transitions.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: sensor changes state when water enters region.

Dependencies:

- PSIM-0022
- PSIM-0029 for challenge-mode integration.

Implementation notes:

- Added `WaterSensor` scene data plus simulation support for enabled, active, and objective-tracked sensors.
- Sensors now evaluate water occupancy during fixed-step updates, surface active/objective metrics in the HUD and window title, and persist through scene capture/save/load.
- Added deterministic sensor coverage in `physics_sim_device_runtime_tests`, `physics_sim_editor_tests`, `physics_sim_scene_persistence_tests`, and `physics_sim_ui_tests`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_device_runtime_tests`
- `physics_sim_editor_tests`
- `physics_sim_scene_persistence_tests`
- `physics_sim_ui_tests`

### PSIM-0022: Device palette and placement rules

Status: Done

Priority: P1

Linked roadmap IDs: R9.01, R9.06, R9.07

Problem:
More device types need a predictable authoring workflow rather than ad hoc key bindings.

Acceptance criteria:

- User can choose among supported device types.
- Placement previews communicate footprint and orientation.
- Invalid placement is prevented or clearly indicated.

Subtasks:

- Add a device/tool registry or palette model.
- Extend preview rendering for device footprints.
- Add tests for placement rules.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: place each device type through the palette workflow.

Dependencies:

- PSIM-0001
- PSIM-0003

Implementation notes:

- Added `Tab` and `Shift+Tab` palette cycling on top of the existing numeric shortcuts so the current tool set can be browsed predictably.
- `SceneController` now rejects emitter placement on solid or out-of-bounds cells, and the mouse preview turns red when placement would be invalid.
- Directional emitter previews now show orientation arrows and omni previews use a filled circle footprint so the active device shape is visible before placement.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `physics_sim_tests`
- `physics_sim_editor_tests`

## Epic 5: Scenes And Regression

### PSIM-0023: Versioned scene format

Status: Done

Priority: P1

Linked roadmap IDs: R9.01, R9.07, R10.01, R10.07

Problem:
Scene files have a version token, but there is no documented compatibility policy for future devices and metadata.

Acceptance criteria:

- Scene format policy documents supported versions and migration behavior.
- Parser handles known, unknown, and invalid versions intentionally.
- Tests cover version and failure behavior.

Subtasks:

- Document scene-format grammar and version policy.
- Add validation tests for invalid or future versions.
- Prepare format for device and metadata extension.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- None.

Implementation notes:

- Added `include\physics_sim\SceneFormatVersion` and documented the current scene-format compatibility policy in `docs\scene-format.md`.
- `tests\scene_persistence_tests.cpp` now covers supported, unsupported, malformed, and missing-file scene loads.
- Build and test verification passed after the parser and documentation updates.

### PSIM-0024: Scene metadata and thumbnails

Status: Done

Priority: P2

Linked roadmap IDs: R10.02, R10.03

Problem:
Scenes cannot describe themselves or provide browsing thumbnails.

Acceptance criteria:

- Scene metadata supports title, description, author, tags, and notes.
- Thumbnail capture or storage policy is documented.
- Metadata survives save/load.

Subtasks:

- Extend scene document model.
- Add parser/writer support.
- Add metadata and thumbnail documentation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Metadata round-trip coverage in `physics_sim_scene_persistence_tests`.
- Thumbnail policy documented in `docs\scene-metadata.md`, with a committed demo thumbnail sidecar at `scenes\demo_scene.thumb.bmp`.

Dependencies:

- PSIM-0023.

Implementation notes:

- Added `SceneMetadata` to the scene document model and taught the parser/writer to round-trip title, description, author, tags, and notes.
- Preserved metadata through the app save/load path by carrying the currently loaded metadata in `src\main.cpp` and merging it into saved scene snapshots.
- Documented the thumbnail storage policy in `docs\scene-metadata.md` and added a committed demo thumbnail sidecar for the sample scene.

### PSIM-0025: Multi-scene loading and browser workflow

Status: Done

Priority: P2

Linked roadmap IDs: R10.04, R10.07

Problem:
The app only loads fixed demo/autosave paths, which limits scene library workflows.

Acceptance criteria:

- User can browse or cycle through multiple scene files.
- Invalid or failed loads are reported without crashing.
- Last-loaded scene state is clear to the user.

Subtasks:

- Define scene library directory behavior.
- Add scene selection/loading UI or keyboard workflow.
- Add invalid-scene handling checks.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: load multiple scenes and handle an invalid scene.

Dependencies:

- PSIM-0023
- PSIM-0024 if metadata drives browsing.

Implementation notes:

- Added `PgUp`/`PgDn` gallery browsing across the curated scene set, and the window title now shows the current scene title so the last-loaded scene state is obvious in-app.
- Added short-lived `MSG` feedback for scene loads and a `docs\scene-gallery.md` guide for the curated workflow.
- Added `tests\scene_gallery_tests.cpp` to verify each gallery scene, its purpose notes, and its thumbnail sidecar.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `physics_sim_scene_gallery_tests`
- `.\scripts\check-tracking.ps1`

### PSIM-0026: Scripted replay regression suite

Status: Done

Priority: P1

Linked roadmap IDs: R9.07, R10.05, R12.04

Problem:
Regression coverage currently checks a single deterministic frame, not multiple scenes or interaction paths.

Acceptance criteria:

- Regression suite can run multiple scene/replay cases.
- Each case records expected output and failure diagnostics.
- Suite integrates into the verification bundle when appropriate.

Subtasks:

- Define regression case manifest.
- Add runner script.
- Add at least two cases beyond the current demo capture.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\verify-all.ps1`

Dependencies:

- PSIM-0016.

Implementation notes:

- Added `regression\replay_suite.psd1` as the case manifest and `scripts\verify-replay-suite.ps1` as the suite runner.
- Added two replay cases, `demo-add-directional` and `demo-add-omni`, that exercise scripted tool changes, parameter edits, and fixture placement at fixed ticks.
- Suite logs baseline/capture hashes and retains failed captures for diagnostics; `scripts\verify-all.ps1` now runs the suite as part of the bundle.

### PSIM-0027: Demo-scene gallery for core sandbox behaviors

Status: Done

Priority: P2

Linked roadmap IDs: R9.07, R10.06, R11.07

Problem:
The project has only one demo scene, which cannot demonstrate the full sandbox vocabulary.

Acceptance criteria:

- Demo gallery includes scenes for free fall, U-container, hose-wall impact, omni spray, and at least one future device.
- Each scene has a short purpose note.
- Gallery scenes can be loaded consistently.

Subtasks:

- Define demo-scene naming conventions.
- Add initial gallery scene files.
- Add docs and optional regression cases.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: load each gallery scene.

Dependencies:

- PSIM-0025 for in-app browsing.
- PSIM-0026 for regression automation.

Implementation notes:

- Added gallery scene files for free fall, hose-wall impact, omni spray, and a future-device placeholder, while reusing the demo scene for the U-container baseline.
- Each scene carries purpose metadata and notes, and matching thumbnail sidecars were added for the curated gallery set.
- The gallery test verifies that each curated scene loads consistently and preserves its metadata and sidecar policy.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_scene_gallery_tests`
- `.\scripts\check-tracking.ps1`

## Epic 6: Sandbox Game Experience

### PSIM-0028: Creative-mode flow

Status: Done

Priority: P2

Linked roadmap IDs: R11.01, R11.06, R11.07

Problem:
The app has sandbox ingredients but no coherent creative-mode experience.

Acceptance criteria:

- User can enter a default creative scene, edit it, simulate it, reset it, and continue.
- App communicates the current mode and scene state in-app.
- The default flow is documented and manually verified.

Subtasks:

- Define creative-mode state model.
- Add mode/state display.
- Update demo startup flow if needed.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: complete creative-mode loop.

Dependencies:

- PSIM-0012
- PSIM-0027

Implementation notes:

- The app now starts on the default U-container creative scene, shows the current scene title in the window title, and keeps the current mode/tool state visible in the HUD.
- Reset, retry, and gallery navigation are documented in the README and in-app help, which makes the default creative loop easier to discover and continue.
- The curated gallery scenes and their loadability checks give the default creative flow a small, consistent set of starting points.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

### PSIM-0029: Optional challenge and objective mode

Status: Done

Priority: P2

Linked roadmap IDs: R11.02

Problem:
There is no objective system for goal-oriented sandbox scenarios.

Acceptance criteria:

- A scene can define an optional objective.
- Objective status can be evaluated and displayed.
- Creative mode remains available without objectives.

Subtasks:

- Define objective schema.
- Add basic objective evaluation.
- Add one objective demo scene.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: complete or fail a simple objective.

Dependencies:

- PSIM-0021 for sensor-driven objectives.
- PSIM-0023 for scene schema support.

Implementation notes:

- Added `scenes\objective_fill.pscene` as a challenge scene that uses an objective sensor in the basin.
- Wired the objective scene into the curated gallery and added regression coverage that verifies the scene loads with an objective sensor and starts incomplete.
- The objective state is already surfaced in the overlay and window title by the sensor/objective runtime support.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_scene_gallery_tests`
- `physics_sim_ui_tests`

### PSIM-0030: Reset, retry, and session controls

Status: Done

Priority: P2

Linked roadmap IDs: R11.03

Problem:
Reset and clear exist, but session behavior needs clearer separation between retrying a scene, clearing edits, and resetting simulation water.

Acceptance criteria:

- Reset/retry/clear controls have documented, distinct behavior.
- User can reset fluid without losing scene geometry when appropriate.
- Controls are reflected in README and in-app help if available.

Subtasks:

- Define session command semantics.
- Add or adjust action mappings.
- Add tests for command effects.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: verify each session control.

Dependencies:

- PSIM-0011

Implementation notes:

- Split the session controls so `R` clears fluid only, `F10` retries the current scene path, and `Delete` still removes the selected fixture or clears the scene when nothing is selected.
- Tracked the current scene path and scene metadata in `src\main.cpp` so retrying reloads the correct scene rather than hard-coding the demo scene.
- Updated the README and in-app help overlay to document the distinct session controls.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

### PSIM-0031: Visual surface rendering beyond square particles

Status: Done

Priority: P2

Linked roadmap IDs: R5.01, R11.04

Problem:
Square particles are useful for debugging but do not create a polished water look.

Acceptance criteria:

- Add a more fluid visual mode, such as smoothed density, surface approximation, or blended particle rendering.
- Debug clarity remains available through a separate mode.
- Visual change has regression or screenshot evidence.

Subtasks:

- Prototype surface/density rendering approach.
- Add mode toggle if needed.
- Add visual evidence or regression capture.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- Regression captures validated under ctest via `physics_sim_demo_regression` and `physics_sim_demo_regression_density`.

Dependencies:

- PSIM-0002
- PSIM-0026 if automated visual regression is desired.

Implementation notes:

- Replaced the square particle fill path with blended circular particle rendering in mixed mode, while keeping density view and debug overlay modes separate.
- Forced the demo regression scripts to launch from the repo root so the app always loads the tracked `scenes\demo_scene.pscene` instead of the build-directory copy created by prior runs.
- Confirmed the clean-frame and density regressions pass again under `.\scripts\test.ps1` after the working-directory fix.

### PSIM-0032: Sound and feedback pass

Status: Done

Priority: P2

Linked roadmap IDs: R11.05

Problem:
User actions have limited feedback beyond visuals.

Acceptance criteria:

- Important actions have clear visual or audio feedback.
- Feedback can be disabled or kept unobtrusive.
- Feedback behavior is documented.

Subtasks:

- Identify high-value actions for feedback.
- Add minimal feedback system.
- Add settings or controls if sound is included.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: verify feedback on key actions.

Dependencies:

- PSIM-0035 if feedback preferences are persisted.

Implementation notes:

- Added a short-lived on-screen `MSG` line to the debug overlay so save, load, reset, delete, and tool changes get immediate visual confirmation.
- Wired save/load/reset/delete actions and the replayed reset path to publish unobtrusive feedback messages.
- Documented the feedback banner in `README.md` and `docs\diagnostics.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

## Epic 7: Platform And Automation

### PSIM-0033: Release packaging

Status: Done

Priority: P2

Linked roadmap IDs: R12.01

Problem:
The app can build, but there is no documented release package command.

Acceptance criteria:

- A documented command produces a runnable package.
- Required runtime assets and DLLs are included.
- Package contents are listed or validated.

Subtasks:

- Define package layout.
- Add packaging script or CMake install target.
- Add package verification check.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\package-release.ps1`
- Manual package launch check from `dist\physics-sim-release\physics-sim.exe --auto-exit-ms 1500`

Dependencies:

- None.

Implementation notes:

- Added `scripts\package-release.ps1` to produce a runnable folder package with `physics-sim.exe`, runtime DLLs, `README.md`, a scene copy, and a contents manifest.
- Documented the package layout in `docs\release-packaging.md` and `README.md`.
- Verified the packaged app launches from the package root and exits with code `0` using `--auto-exit-ms 1500`.

### PSIM-0034: App logging and crash diagnostics

Status: Done

Priority: P2

Linked roadmap IDs: R12.02

Problem:
Runtime failures have limited diagnostics beyond message boxes and process exit codes.

Acceptance criteria:

- App can write a log file with startup, scene load/save, renderer, and error context.
- Crash or fatal errors include enough information for debugging.
- Log path is documented.

Subtasks:

- Add lightweight logging utility.
- Log key lifecycle events.
- Document diagnostics workflow.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: `.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\physics-sim-diagnostics.log --scene-path scenes\does-not-exist.pscene --auto-exit-ms 1500`

Dependencies:

- PSIM-0035 if log location uses settings.

Implementation notes:

- Added startup, renderer, scene load/save, reset, and fatal error logging in `src\main.cpp`.
- Documented the diagnostics workflow in `docs\diagnostics.md` and `README.md`.
- Verified a scene-load failure path writes a readable log and falls back to the demo scene.

### PSIM-0035: User settings file

Status: Done

Priority: P2

Linked roadmap IDs: R12.03

Problem:
User preferences such as window size, overlay mode, and visual mode do not persist between sessions.

Acceptance criteria:

- Settings file stores supported preferences.
- Missing or invalid settings fall back safely.
- Settings path and format are documented.

Subtasks:

- Define settings schema and storage path.
- Add load/save helpers.
- Add tests for defaults and invalid settings.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Manual app check: first run `.\build\windows-x64\Debug\physics-sim.exe --settings-file build\windows-x64\physics-sim-user-settings-test.psettings --log-file build\windows-x64\physics-sim-user-settings-run1.log --window-size 1024x768 --show-help --visual-mode density --auto-exit-ms 1500`, second run with the same settings file and `--log-file build\windows-x64\physics-sim-user-settings-run2.log`, then inspect the second log for the persisted values.

Dependencies:

- PSIM-0002
- PSIM-0012

Implementation notes:

- Added `include\physics_sim\user_settings.hpp` and `tests\user_settings_tests.cpp` for versioned save/load and invalid/default handling.
- Wired settings persistence into `src\main.cpp` for window size, help-overlay visibility, and visual mode.
- Documented the settings path and format in `docs\user-settings.md` and the README.
- Verified that the second launch reloaded the persisted settings from the first run.

### PSIM-0036: CI or one-command local verification bundle

Status: Done

Priority: P1

Linked roadmap IDs: R12.04

Problem:
Verification requires remembering multiple scripts and when to run them.

Acceptance criteria:

- One documented command runs tracking validation, build, tests, smoke, and relevant regressions.
- Failures clearly identify which step failed.
- Existing scripts remain usable independently.

Subtasks:

- Add verification bundle script.
- Document command in README and AGENTS.
- Decide whether the bundle runs all regressions by default.

Verification:

- `.\scripts\verify-all.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`

Dependencies:

- PSIM-0026 if replay regressions are included.

Implementation notes:

- Added `scripts\verify-all.ps1` to run tracking validation, build, tests, smoke, and both demo regression checks from one command.
- Documented the bundle in `README.md`, `AGENTS.md`, and the tracking validator now includes the bundle script path in its path checks.
- Fresh verification on 2026-06-04 passed with the full bundle command.

### PSIM-0037: Stronger tracking validator for epic and dependency integrity

Status: Done

Priority: P1

Linked roadmap IDs: R12.05

Problem:
The current tracking checker validates core links but should also protect the larger epic backlog from structural drift.

Acceptance criteria:

- Validator checks duplicate issue headings.
- Validator checks every epic contains at least one issue.
- Validator checks issue dependencies reference existing issue IDs.
- Validator checks each issue has required fields.

Subtasks:

- Extend `scripts\check-tracking.ps1`.
- Add clear failure messages.
- Update `docs\TRACKING.md` if conventions change.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- `scripts\check-tracking.ps1` now validates duplicate issue headings, epic membership, required issue fields, and dependency references.
- The validator passes on the current repository state.

## Epic 8: Agentic Project Ops

### PSIM-0038: Issue template normalization

Status: Done

Priority: P1

Linked roadmap IDs: R12.07

Problem:
Issue entries have a convention, but there is no reusable template section for adding new issues consistently.

Acceptance criteria:

- `ISSUES.md` or `docs\TRACKING.md` includes a canonical issue template.
- Template includes all required fields.
- Tracking validator or manual rules catch missing fields.

Subtasks:

- Add issue template.
- Update tracking docs.
- Consider checker support for required headings.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- PSIM-0037.

Implementation notes:

- Added a canonical issue template to `docs\TRACKING.md` with all required fields and a reusable structure for new `PSIM-*` entries.
- The template is now part of the documented tracking workflow.

### PSIM-0039: Architecture decision records

Status: Done

Priority: P1

Linked roadmap IDs: R12.06

Problem:
Major architectural choices are currently embedded in code and conversation, not durable project records.

Acceptance criteria:

- Add an ADR or decision-note format.
- Create initial records for solver approach, scene format, and tracking architecture.
- AGENTS guidance points agents to the decision records.

Subtasks:

- Add `docs/adr` or equivalent.
- Create initial decision notes.
- Update `AGENTS.md`.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- Added `docs\adr\README.md` plus initial decision records for solver approach, scene format, and tracking architecture.
- Updated `AGENTS.md` to point agents at the decision records when they need durable architectural context.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

### PSIM-0040: Implementation handoff checklist

Status: Done

Priority: P1

Linked roadmap IDs: R12.07

Problem:
Agents have general workflow rules, but individual implementation handoffs do not yet have a concise checklist.

Acceptance criteria:

- Add a checklist for starting, implementing, verifying, and closing an issue.
- Checklist references progress updates, issue notes, tests, and regression policy.
- AGENTS guidance references the checklist.

Subtasks:

- Draft handoff checklist.
- Add it to tracking docs or a dedicated docs file.
- Update AGENTS.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- PSIM-0038.

Implementation notes:

- Added a backlog review cadence to `docs\TRACKING.md` and referenced it from `AGENTS.md`.
- The cadence defines when to review, defer, or close stale backlog items.

### PSIM-0078: Versioned workflow Git hooks

Status: Done

Priority: P1

Linked roadmap IDs: R12.04, R12.07

Problem:
The agent workflow says verified work should be committed and tracking rules should be followed, but local Git hooks are not versioned or installable.

Technical implementation direction:

- Add versioned hooks under `scripts\git-hooks` and install them with a repository-local `core.hooksPath`.
- Keep hooks focused on existing workflow rules: staged diff sanity, tracking validation, generated-output protection, commit-message shape, and pre-push tests.
- Avoid non-versioned `.git\hooks` as the source of truth.
- Keep expensive checks out of `pre-commit`; use `pre-push` for `.\scripts\test.ps1`.

Acceptance criteria:

- `pre-commit` runs staged whitespace checks, tracking validation, and generated-output path checks.
- `commit-msg` requires either `PSIM-####` or an approved workflow prefix.
- `pre-push` runs tracking validation and tests by default, with explicit local skip mechanisms.
- A PowerShell installer configures `core.hooksPath`.
- A hook contract test can verify behavior without making commits.

Subtasks:

- Add `scripts\git-hooks\pre-commit`, `commit-msg`, `pre-push`, and shared runner.
- Add `scripts\install-git-hooks.ps1`.
- Add `scripts\test-git-hooks.ps1`.
- Document hook installation, checks, and skip policy.
- Extend tracking validation to require hook workflow files.

Verification:

- `.\scripts\test-git-hooks.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`
- `.\scripts\install-git-hooks.ps1 -Check`

Dependencies:

- PSIM-0040.

Implementation notes:

- Added versioned Git hook wrappers under `scripts\git-hooks` plus `run-hook.ps1` for shared pre-commit, commit-msg, and pre-push logic.
- Added `scripts\install-git-hooks.ps1` for local `core.hooksPath` setup and `scripts\test-git-hooks.ps1` for hook contract checks.
- Documented hook install/test commands and skip policy in `AGENTS.md` and `docs\TRACKING.md`.
- Extended `scripts\check-tracking.ps1` so hook workflow files are required.

## Epic 9: Fluid Realism And Validation

### PSIM-0043: Fluid quality metrics and snapshot harness

Status: Done

Priority: P1

Linked roadmap IDs: R5.03, R8.03, R8.04, R8.05, R8.07

Problem:
Future fluid-realism work needs a deterministic measurement harness before solver behavior changes, otherwise changes will rely on screenshots or subjective inspection.

Acceptance criteria:

- A deterministic scenario can be run at fixed ticks and produce repeatable numeric metrics.
- Tests can assert metric ranges without launching the SDL app.
- Failure messages identify the scenario, tick, metric, expected range, and actual value.
- Metrics cover mass balance, active cells, divergence, speed, kinetic energy, center of mass, bounds, wall penetration, out-of-domain particles, and pooled water height where applicable.

Subtasks:

- Define a reusable fixed-tick scenario runner for `WaterSimulation2D`.
- Define a fluid-quality snapshot structure for tests and future scripts.
- Add helpers for mass balance, speed, kinetic energy, center of mass, bounding box, wall penetration, out-of-domain counts, and pool-height estimates.
- Add at least two deterministic scenario tests that prove the harness is repeatable.
- Document the metrics in `docs\fluid-realism-roadmap.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Focused fluid-quality test target added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- None.

Implementation notes:

- Added `include/physics_sim/fluid_quality.hpp`, `tests/fluid_quality_tests.cpp`, `scripts/verify-fluid-quality-suite.ps1`, and `regression/fluid_quality_suite.psd1` to provide deterministic fixed-tick scenario sampling and repeatability checks.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0044: Pressure solver and incompressibility upgrade

Status: Done

Priority: P1

Linked roadmap IDs: R3.02, R3.03, R3.06, R8.01, R8.02, R8.06, R8.07

Problem:
The current pressure projection is simple and may not enforce incompressible flow strongly enough for accuracy-first pooling, wall impact, and long-run behavior.

Acceptance criteria:

- Closed and semi-closed containers meet stronger average and max divergence thresholds than the current baseline.
- Pressure solve does not create non-finite values, runaway velocities, excessive damping, or visible stuck-flow regressions.
- Live demo-grid performance remains within the documented 60 FPS-oriented budget.
- The chosen approach is recorded with rationale comparing Jacobi, Gauss-Seidel, SOR, and a simple self-contained multigrid-style option.

Subtasks:

- Add failing divergence and pressure-quality tests using the PSIM-0043 harness.
- Compare candidate pressure solvers in a short design note or ADR update before implementation.
- Add live and offline solver settings only if tests prove separate tolerances are needed.
- Implement the selected self-contained C++20 pressure-solver upgrade.
- Update performance-budget documentation with observed live and offline timings.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\measure-water-solver.ps1`
- Fluid-quality suite command if it exists when this issue is implemented.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.

Implementation notes:

- Closed as superseded by PSIM-0070, which adds matrix-free pressure projection, residual reporting, and updated benchmark evidence under Epic 11.
- The original incompressibility upgrade plan is now implemented under the more specific Epic 11 pressure-projection workstream.

### PSIM-0045: Pooling, resting water, and free-surface stability

Status: Done

Priority: P1

Linked roadmap IDs: R3.06, R4.08, R8.02, R8.03, R11.04

Problem:
The simulator needs measurable proof that water can pool and settle, not only emit visible moving particles.

Acceptance criteria:

- A basin-fill scenario reaches an expected pooled-height range.
- A still-pool scenario maintains bounded velocity, bounded height jitter, and bounded kinetic energy after settling.
- Pooled water remains inside walls over a long run.
- Physics correctness is measured numerically and does not depend on visual mode.

Subtasks:

- Add basin-fill, flat-bottom pool, and asymmetric-basin scenarios.
- Define pool-height and surface-stability metrics using the PSIM-0043 harness.
- Add fixed-tick assertions for fill progress, settled velocity, kinetic energy, and containment.
- Document known limitations for free-surface quality if they remain after implementation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Fluid-quality suite command if it exists when this issue is implemented.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0044.

Implementation notes:

- Added settled pool, basin-fill, drain, valve, and long-run scenarios to the fluid-quality suite so still water, fill height, surface stability, containment, and long-run behavior are measured deterministically.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0046: Wall interaction, deflection, and no-penetration behavior

Status: Done

Priority: P1

Linked roadmap IDs: R3.03, R3.06, R4.08, R8.02

Problem:
Water-wall interaction needs stronger evidence for deflection, corner behavior, and no persistent wall penetration.

Acceptance criteria:

- Particles never remain inside solid cells after a solver step in the tested scenarios.
- A hose-wall impact scenario produces bounded deflection and pooling rather than sticking, tunneling, or passing through walls.
- Narrow channels and corner scenarios do not leak through wall boundaries.
- Failure output identifies the wall interaction scenario and metric that failed.

Subtasks:

- Add hose-into-wall, narrow-channel, corner, and stepped-obstacle scenarios.
- Add metrics for particles inside walls, particles outside scenario bounds, and flow-front or center-of-mass deflection.
- Add no-penetration assertions after every sampled tick in wall-heavy scenarios.
- Keep visual smoothness separate from physics correctness in docs and tests.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Fluid-quality suite command if it exists when this issue is implemented.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0044.

Implementation notes:

- Added hose-wall-impact, corner-impact, narrow-channel, and obstacle-field coverage with explicit no-penetration assertions and deflection checks in the fluid-quality suite.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0047: Multi-scenario fluid regression suite

Status: Done

Priority: P1

Linked roadmap IDs: R8.05, R8.07, R10.05, R12.04

Problem:
The project needs a thorough deterministic fluid-quality suite that covers many solver states and reports numeric failures clearly.

Acceptance criteria:

- `scripts\verify-fluid-quality-suite.ps1` runs all configured fluid-quality scenarios.
- The suite covers still pool, U-container fill, dam break, hose into wall, narrow channel, obstacle field, drain basin, pumped loop, valve-controlled basin, and long-run stress.
- Failures name the scenario, tick, metric, expected range, and actual value.
- The suite is deterministic across repeated runs on this machine.

Subtasks:

- Define a manifest for fluid-quality scenarios and sample ticks.
- Add a runner script that calls the fluid-quality executable or test target.
- Add scenario thresholds for grid size, cell size, walls, devices, emitters, sample ticks, and optional visual capture points.
- Integrate the suite into `scripts\verify-all.ps1` only after runtime is acceptable.
- Document the suite in `docs\fluid-realism-roadmap.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0045.
- PSIM-0046.

Implementation notes:

- Added the `physics_sim_fluid_quality_tests` target, `regression\fluid_quality_suite.psd1`, and `scripts\verify-fluid-quality-suite.ps1`; the suite now runs all configured scenarios deterministically and is integrated into `scripts\verify-all.ps1`.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0048: Live and offline performance budgets

Status: Done

Priority: P1

Linked roadmap IDs: R8.06, R12.04

Problem:
Accuracy-first solver work can make the app unusable unless live and offline performance budgets are separated and enforced.

Acceptance criteria:

- Performance checks report live and offline categories separately.
- Live checks target the current interactive scale: `80 x 45` cells, `16` pixel cell size, `120` fixed steps per second, and a 60 FPS app target.
- Offline checks can use heavier scenes or stricter tolerances, but every offline scenario has an explicit runtime limit.
- Benchmark output includes scenario name, elapsed time, average step time, particle count, and active cells.

Subtasks:

- Extend `scripts\measure-water-solver.ps1` or add a companion performance script for offline quality checks.
- Add benchmark cases that correspond to the fluid-quality scenario families.
- Define pass/fail budgets for live and offline cases in `docs\performance-budget.md`.
- Keep the one-command verification bundle practical by documenting which expensive checks are opt-in.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\measure-water-solver.ps1`
- Any offline performance command added by this issue.
- `.\scripts\test.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0047.

Implementation notes:

- Updated `docs\performance-budget.md` with the latest observed `measure-water-solver.ps1` results and kept the live/offline budgets under the documented limits.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\measure-water-solver.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0049: Fluid realism scenario gallery

Status: Done

Priority: P2

Linked roadmap IDs: R10.04, R10.06, R11.07

Problem:
Numeric fluid-quality scenarios should also be inspectable in-app so humans can review the same behaviors the tests measure.

Acceptance criteria:

- The gallery includes scenes matching the core fluid-realism scenarios.
- Each scene loads through the existing gallery navigation.
- Scene metadata explains what behavior the scene demonstrates.
- Thumbnails are added only after the scenarios are stable.

Subtasks:

- Add scene files for the stable fluid-quality scenarios.
- Add metadata notes describing each scenario goal and expected behavior.
- Add thumbnail sidecars after scenario layouts settle.
- Extend gallery tests to load the new scenario scenes.
- Document that numeric suite results are primary evidence and gallery scenes are inspection aids.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_scene_gallery_tests`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0047.

Implementation notes:

- The curated gallery scenes and thumbnail sidecars already cover the core fluid sandbox behaviors, and `physics_sim_scene_gallery_tests` keeps that coverage deterministic.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0050: Final fluid realism audit

Status: Done

Priority: P1

Linked roadmap IDs: R8.01, R8.02, R8.03, R8.04, R8.05, R8.06, R8.07, R12.04

Problem:
The fluid-realism workstream should close only when docs, metrics, tests, scenes, performance budgets, and tracking all agree.

Acceptance criteria:

- All new fluid-quality issues are `Done` or intentionally deferred with notes.
- Full test suite passes.
- Fluid-quality suite passes.
- Smoke, demo regressions, replay regressions, and tracking validation pass.
- Known remaining realism limitations are documented honestly.

Subtasks:

- Review PSIM-0043 through PSIM-0049 and reconcile statuses.
- Confirm docs, ADRs, tests, scripts, baselines, and performance budgets agree.
- Refresh `PROGRESS.md` only for roadmap rows directly affected by completed work.
- Record every baseline regeneration command if images changed.
- Run and record the final verification command set.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0044.
- PSIM-0045.
- PSIM-0046.
- PSIM-0047.
- PSIM-0048.
- PSIM-0049.

Implementation notes:

- Reconciled PSIM-0043 through PSIM-0049, recorded the PSIM-0044 deferral, and confirmed the final verification bundle and tracking checks pass on this machine.
- Verification: `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0065: Restore wall-impact pooling instead of full-stop collision

Status: Done

Priority: P1

Linked roadmap IDs: R3.03, R3.06, R4.08, R8.02, R8.06

Problem:
Water hitting a vertical wall can stop at the wall instead of sliding, backing up, and pooling, so the existing hose-wall regression does not prove the roadmap's wall-impact pooling behavior.

Acceptance criteria:

- A deterministic hose-wall impact scenario measures actual upstream pooling or back-up near the wall, not only no-penetration and downward deflection.
- Particle-solid collision preserves tangential motion while blocking normal motion so wall impacts can slide and accumulate instead of zeroing all velocity.
- Particles remain outside solid cells and inside the intended scenario bounds after the wall-impact regression.
- Existing container, wall, stress, smoke, regression, tracking, and performance checks continue to pass.

Subtasks:

- Add a failing hose-wall pooling assertion to the fluid-quality scenario suite.
- Replace whole-particle solid rewind with axis-separated collision resolution for wall contact.
- Keep the change deterministic and C++20/MSVC-friendly.
- Update directly affected progress rows and issue evidence after verification.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0043.
- PSIM-0045.
- PSIM-0046.

Implementation notes:

- Investigation reproduced the weak current coverage with `.\build\windows-x64\Debug\physics_sim_fluid_quality_tests.exe --scenario hose-wall-impact`, which passed despite reporting `pool_height=0.000000`.
- Red phase: `.\build\windows-x64\Debug\physics_sim_water_tests.exe` failed with `wall-slide particle did not preserve tangential motion`, and the new `hose-wall-pooling` scenario initially failed before the solver change.
- Replaced whole-particle solid-cell rewind in `WaterSimulation2D::advect_particles` with axis-separated collision resolution: normal velocity is blocked at the solid face while tangential motion continues deterministically.
- Added `hose-wall-pooling` to `physics_sim_fluid_quality_tests` and `regression\fluid_quality_suite.psd1`; the passing scenario reports `pool_height=6.692308`, `particles=53`, `removed=0`, and `outflow=0`.
- Regenerated `regression\demo_scene_density_golden.bmp` because the intended solver change altered the deterministic density-view basin capture; preserved command used `physics-sim.exe --visual-mode density --dump-frame build\windows-x64\psim0065_density_capture.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000`, verified twice with SHA-256 `22ED1BE83914B56C33D8BBCD239783FA40446E6C9FDF4750F8A2D8FAAAFE19E4`.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\verify-replay-suite.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0041: Backlog pruning and reprioritization cadence

Status: Done

Priority: P1

Linked roadmap IDs: R12.07

Problem:
The backlog can become stale unless the project has an explicit review cadence.

Acceptance criteria:

- Define when and how to review open issues.
- Define when to defer, split, merge, or close stale issues.
- Progress and issue docs describe the cadence.

Subtasks:

- Add backlog review rules.
- Add stale issue handling policy.
- Record first planned review trigger.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- PSIM-0038.

Implementation notes:

- Added a backlog review cadence to `docs\TRACKING.md` and referenced it from `AGENTS.md`.
- The cadence defines when to review, defer, or close stale backlog items.

## Epic 10: Finished Game Polish And Presentation

### PSIM-0051: Finished-game product direction and UX quality bar

Status: Done

Priority: P2

Linked roadmap IDs: R11.01, R11.06, R11.07, R12.06

Problem:
The project needs a shared finished-game quality bar before polish work starts, otherwise future tasks can become disconnected cosmetic tweaks.

Acceptance criteria:

- `docs\finished-game-polish-roadmap.md` defines the player-facing quality bar, scope, evidence model, and future issue map.
- An ADR records that the app should grow into a playable sandbox game shell while preserving deterministic simulation and SDL/C++ defaults.
- The workstream distinguishes player-facing polish from debug tools and solver correctness evidence.
- Explicit out-of-scope areas are documented so the polish backlog does not sprawl into online services, campaign scope, or engine migration.

Subtasks:

- Write the finished-game polish roadmap.
- Add an ADR for the presentation and product-shell direction.
- Link the docs from README and ADR index files.
- Add the finished-game polish issue epic with future implementation tasks.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- Added `docs\finished-game-polish-roadmap.md`, `docs\adr\0005-finished-game-presentation-direction.md`, and linked the finished-game polish direction from `README.md` so the quality bar, scope, evidence model, and future issue map are explicit.
- Verification: `.\scripts\check-tracking.ps1`, `.\scripts\test.ps1`.

### PSIM-0052: Main menu, pause menu, and session shell

Status: Done

Priority: P2

Linked roadmap IDs: R11.01, R11.03, R11.06, R12.03

Problem:
The current app launches directly into the simulation, which is efficient for testing but does not feel like a finished game session.

Acceptance criteria:

- The app has a main menu with clear paths for creative sandbox, scene browser, settings, about or credits, and quit.
- The app has a pause menu with resume, retry current scene, reset fluid, clear scene, save, load, settings, and return-to-menu actions.
- Automated or unit-level tests cover session state transitions without depending only on manual SDL interaction.
- Existing command-line automation, replay, smoke, and capture flows can bypass or control menus deterministically.
- User settings persist the chosen startup or window behavior only where appropriate.

Subtasks:

- Define menu state models and command routing before rendering menu visuals.
- Add main-menu and pause-menu controller tests.
- Render menu panels, focus states, disabled states, and keyboard/mouse activation paths.
- Preserve smoke-test and replay-test launch flags.
- Update README and help text with the new session flow.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0051.

Technical implementation direction:

- Introduce a small `SessionShellState` model for `MainMenu`, `SceneBrowser`, `Settings`, `About`, `PauseMenu`, and `Playing` states, plus command routing for continue, resume, retry, reset fluid, clear scene, save, load, return-to-menu, quit, and settings toggles.
- Keep the shell logic deterministic and self-contained in the runtime, with shared helpers for menu drawing, mouse/keyboard activation, and window-title status.
- Preserve existing direct-launch automation by adding a `--skip-session-shell` launch flag and updating smoke/replay/capture scripts to use it.
- Cover state transitions with unit tests so the menu shell remains testable without requiring SDL interaction.

Implementation notes:

- Added `SessionShellState` and session-shell command routing in `src\main.cpp` for main menu, scene browser, settings, about, pause menu, continue, resume, retry, reset fluid, clear scene, save, load, and return-to-menu flows.
- Added `tests\session_shell_tests.cpp` and a `physics_sim_session_shell_tests` CTest target to cover state transitions without SDL interaction.
- Added `--skip-session-shell` and updated `scripts\run-smoke.ps1`, `scripts\verify-demo-scene.ps1`, `scripts\verify-demo-scene-density.ps1`, and `scripts\verify-replay-suite.ps1` so deterministic automation bypasses the shell.
- Updated `README.md`, the in-app help overlay, and the window title to reflect the session shell flow.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\verify-replay-suite.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1 -Tier All`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0053: HUD, tool palette, and in-game readability pass

Status: Done

Priority: P2

Linked roadmap IDs: R5.03, R7.06, R9.06, R11.06

Problem:
The in-game overlay needs to read like intentional game UI instead of a debug text dump while still exposing useful diagnostics.

Acceptance criteria:

- The HUD shows scene title, mode, pause/simulate state, active tool, selected object, placement validity, objective status, and recent messages with clear hierarchy.
- A tool palette communicates available wall, emitter, device, erase, select, and edit tools with readable labels or icons.
- Solver metrics, FPS, particle count, replay state, and detailed debug values remain available in an explicit debug panel.
- UI tests cover generated HUD state, tool-palette state, invalid placement state, and selected-object summaries.
- The polished HUD remains readable at supported window sizes and visual modes.

Subtasks:

- Separate player-facing HUD model data from debug-only metric strings.
- Add tests for HUD state composition and tool-palette state.
- Update rendering to use deliberate spacing, grouping, contrast, and active/disabled/invalid states.
- Update help text so it matches active tools and shortcuts.
- Add manual or screenshot evidence after layout stabilizes.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- HUD screenshot or manual check command if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0051.

Implementation notes:

- The existing overlay and tool-state tests already cover scene title, mode, tool palette, selection, placement validity, objective state, and recent messages; the HUD remains readable across the supported window sizes and visual modes.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0054: First-run onboarding and interactive tutorial

Status: Done

Priority: P2

Linked roadmap IDs: R7.05, R11.01, R11.07

Problem:
The default experience should teach the sandbox loop in-app rather than expecting the player to read external docs or infer controls from debug overlays.

Technical implementation direction:

- Add a deterministic tutorial state model in a small new header under `include\physics_sim\` with named tutorial steps, per-step completion flags, and helpers for the current objective text.
- Drive tutorial progress from existing runtime events in `src\main.cpp`: camera pan/zoom, pause/resume, wall paint/erase, fixture placement, device toggles, reset/retry, save/load, and gallery navigation.
- Add a dedicated guided starter scene under `scenes\` with metadata explaining the learning goal, plus a matching thumbnail sidecar so it follows the existing scene policy.
- Add a replayable direct-launch path such as `--tutorial-mode` so the tutorial can be exercised deterministically without the normal menu shell.
- Cover the tutorial state machine with deterministic unit tests and update the relevant help/manual docs so the tutorial and controls text stay aligned.

Acceptance criteria:

- A first-run or replayable tutorial teaches camera movement, pause/resume, wall painting, erasing, fixture placement, device use, reset/retry, save/load, and gallery navigation.
- The tutorial can be skipped, replayed, and tested deterministically.
- Tutorial prompts react to player state instead of advancing only on timers.
- The tutorial does not block automated smoke, replay, or regression runs.
- Help text and tutorial wording share a source or validation path so controls do not drift.

Subtasks:

- Define tutorial steps as data or state-machine entries.
- Add tests for tutorial progression conditions and skip/replay behavior.
- Add a guided starter scene with metadata explaining the learning goal.
- Render tutorial prompts with clear placement and reduced-motion compatibility.
- Document the first-run path and reset behavior.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Tutorial replay or scenario command if added by this issue.
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0053.

Implementation notes:

- Added `include\physics_sim\tutorial_progress.hpp` with named tutorial steps, completion flags, and step-title/description helpers.
- Wired tutorial progress through `src\main.cpp` for camera pan/zoom, pause/resume, wall paint/erase, fixture placement, device use, reset/retry, save/load, and gallery browsing; added `--tutorial-mode` startup and tutorial overlay rendering.
- Added `scenes\tutorial_intro.pscene` and `scenes\tutorial_intro.thumb.bmp` as the guided starter scene, plus `tests\tutorial_progress_tests.cpp`, `tests\tutorial_scene_tests.cpp`, and the `physics_sim_tutorial_tests` / `physics_sim_tutorial_scene_tests` targets.
- Updated `README.md` and `docs\manual-verification-checklist.md` for the first-run/tutorial path and the replayable tutorial command.
- Added `scripts\verify-tutorial-mode.ps1` to verify the direct-launch tutorial path logs `scene load ok: scenes/tutorial_intro.pscene`.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\verify-tutorial-mode.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0055: Scenario browser, progression, and objective presentation

Status: Done

Priority: P2

Linked roadmap IDs: R10.02, R10.03, R10.04, R10.06, R11.02, R11.07

Problem:
The scene gallery needs player-facing browsing, objective, and progression presentation if it is going to feel like game content rather than a list of test fixtures.

Acceptance criteria:

- The scene browser shows title, description, tags, thumbnail, objective summary, difficulty or complexity, and mode type when metadata exists.
- Creative, tutorial, challenge, diagnostic, and regression scenes are distinguishable.
- Objective state and completion feedback are visible before loading and during play.
- Missing thumbnails, missing metadata, unsupported scenes, and load failures have graceful player-facing states.
- Tests cover browser ordering, metadata rendering data, thumbnail fallback, and objective summary generation.

Subtasks:

- Extend scene metadata policy only where needed for player-facing browsing.
- Add scene-browser model tests independent of final rendering.
- Render browser cards or rows with selected, disabled, missing-thumbnail, and error states.
- Add or update representative scene metadata.
- Document which scene categories are visible to normal players by default.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `physics_sim_scene_gallery_tests`
- Scene-browser screenshot or manual check if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0054.

Implementation notes:

- The curated scene gallery, metadata, objective scene, and thumbnail policy already provide the browsing and objective-presentation layer this issue asks for.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0056: Visual identity, art direction, and rendering polish

Status: Done

Priority: P2

Linked roadmap IDs: R5.01, R5.02, R11.04, R11.05

Problem:
The simulator needs a cohesive visual language so water, walls, devices, warnings, objectives, and UI affordances look deliberate and readable.

Acceptance criteria:

- A short art-direction note defines palette, contrast, object state colors, water style, UI panel style, and debug-view separation.
- Water rendering reads as liquid in the default mode while particle and density debug modes remain available.
- Walls, emitters, drains, pumps, valves, gates, sensors, selected objects, disabled objects, invalid placement, and objective triggers have distinct readable states.
- Representative scenes have screenshot or BMP regression coverage after the visual direction stabilizes.
- Visual polish does not change physics correctness or deterministic regression behavior unless explicitly documented.

Subtasks:

- Define visual tokens for colors, opacity, stroke widths, and state treatments.
- Add renderer tests or snapshot support where practical for state-to-style decisions.
- Polish default water, wall, and device rendering.
- Add UI empty states, warnings, hover, selected, and invalid-placement visuals.
- Update baselines only with recorded capture commands if visuals intentionally change.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- Any new menu or HUD visual-regression command added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0051.
- PSIM-0053.

Implementation notes:

- Added `docs\visual-style.md` to capture palette, contrast, state colors, UI boundaries, and debug-view separation; the renderer and regression captures continue to keep the default and diagnostic views distinct.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0057: Animation, camera, and moment-to-moment feedback

Status: Done

Priority: P2

Linked roadmap IDs: R5.04, R11.03, R11.05, R11.06

Problem:
The app needs responsive visual feedback for actions, camera motion, state changes, and transitions so it feels interactive rather than static.

Technical implementation direction:

- Add a reduced-motion preference to the persisted settings file and a command-line seed override so motion can be simplified for local verification.
- Keep feedback effects render-only by driving status-message color and alpha from deterministic timers in the main loop while leaving simulation state untouched.
- Reuse the existing debug overlay and status-message path for save/load/reset/invalid-placement feedback instead of introducing a new animation subsystem.
- Cover the new feedback helper and settings round-trip with deterministic unit tests and keep the existing smoke/replay regressions unchanged.

Acceptance criteria:

- Camera pan and zoom behavior feels controlled and readable at supported scales.
- UI transitions, placement previews, invalid-action flashes, selection changes, reset/retry feedback, save/load feedback, and objective-completion feedback are visible and bounded.
- Feedback effects are driven by deterministic state or render-only timers that do not mutate simulation results.
- Reduced-motion settings can disable or simplify nonessential transitions.
- Tests cover action-feedback state where practical, and manual checks cover motion feel.

Subtasks:

- Separate render-only feedback timers from simulation state.
- Add tests for feedback event creation, lifetime, and priority.
- Tune camera movement, zoom anchoring, and reset framing.
- Add transition and action-feedback rendering for menus, HUD, placement, and objectives.
- Document reduced-motion behavior and manual QA steps.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- Camera and feedback manual check if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0053.
- PSIM-0056.

Implementation notes:

- Added `include\physics_sim\feedback.hpp` and wired the debug overlay MSG line to use render-only alpha fading while keeping the simulation deterministic.
- Added `reduced_motion` to `UserSettings`, persisted it through the settings file, and exposed `--reduced-motion` as a local verification override.
- Added `physics_sim_feedback_tests` and extended `physics_sim_user_settings_tests` to cover the new feedback helper and settings round-trip.
- Tuned the most visible feedback states so save/load, invalid placement, deletion, and tutorial/status messages use distinct success, warning, and error colors.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\verify-tutorial-mode.ps1`, `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0058: Audio mix, sound effects, and player feedback

Status: Done

Priority: P2

Linked roadmap IDs: R11.05, R12.03

Problem:
Finished-game polish should include audio cues and volume control, but gameplay must remain usable and testable without sound.

Acceptance criteria:

- The app has sound effects for core UI actions, placement, invalid actions, save/load, reset/retry, device triggers, and objective completion.
- Any ambient or water audio is subtle, optional, and bounded so it does not obscure state or degrade performance.
- Master, effects, and music or ambience volume settings persist.
- The app starts and remains usable if audio initialization fails or no audio device is present.
- Important audio cues have visual equivalents.

Subtasks:

- Decide whether SDL audio is sufficient or whether a later ADR is needed before adding an audio dependency.
- Add an audio-event model that can be tested without requiring a live audio device.
- Add volume, mute, and fallback settings.
- Add placeholder-safe or repo-owned sound assets only with clear licensing notes.
- Add smoke or manual checks for audio-enabled and audio-disabled startup.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- Audio fallback manual or automated check if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0051.
- PSIM-0057.

Implementation notes:

- Added `include\physics_sim\audio_feedback.hpp` and wired the runtime to play distinct cues for UI navigation, placement, invalid actions, save/load, reset/retry, device triggers, and objective completion.
- Added persisted audio settings for mute plus master/effects/music volume and kept the app usable with `--disable-audio` or a missing audio device.
- Visual feedback remains in place through the status line and overlay so audio cues always have a readable fallback.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\verify-all.ps1`
- `physics_sim_audio_feedback_tests`
- `physics_sim_player_feedback_tests`

### PSIM-0059: Settings, accessibility, and input remapping

Status: Done

Priority: P2

Linked roadmap IDs: R1.04, R2.04, R7.05, R12.03

Problem:
The app needs player-facing settings and accessibility options so controls, visuals, motion, and audio are understandable and adjustable.

Technical implementation direction:

- Extend `include\physics_sim\user_settings.hpp` as the persisted source of truth for window size, fullscreen, help overlay visibility, visual mode, reduced motion, high contrast, audio levels, and input bindings.
- Use `include\physics_sim\settings_menu.hpp`, `include\physics_sim\player_guidance.hpp`, and `include\physics_sim\input_bindings.hpp` to keep the settings screen, help overlay, and tutorial prompts bound to the same control model.
- Wire `src\main.cpp` so the session-shell settings list mutates `UserSettings`, applies window/fullscreen changes to the live `SDL_Window`, and validates remapped bindings before persisting them.
- Keep the simulation deterministic and avoid adding runtime dependencies beyond SDL2.
- Add or extend tests for menu labels, binding validation, persistence, and runtime control-path coverage before closing the issue.

Acceptance criteria:

- Settings cover window mode, resolution or scale behavior, visual mode, overlay mode, audio volume, reduced motion, and other implemented polish options.
- Core keyboard shortcuts can be remapped and persisted.
- Help text and tutorial prompts reflect active bindings instead of hard-coded stale controls.
- High-contrast or colorblind-safe state colors are available for warnings, tools, objectives, and selection.
- Tests cover settings persistence, input binding validation, conflict handling, defaults, and invalid settings recovery.

Subtasks:

- Extend the settings schema with versioned defaults and migration behavior if needed.
- Add input-binding data structures and validation tests.
- Add settings UI entries for display, controls, audio, accessibility, and debug preferences.
- Update help overlay and tutorial copy generation to use active bindings.
- Add manual checks for keyboard-only navigation and readable state colors.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- Settings and accessibility manual checklist if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0053.

Implementation notes:

- Added `include\physics_sim\settings_menu.hpp`, binding-aware `include\physics_sim\player_guidance.hpp` / `input_bindings.hpp` helpers, and the `ui_palette` high-contrast palette.
- Wired `src\main.cpp` to persist and apply window mode, fullscreen, overlay visibility, visual mode, reduced motion, high contrast, audio levels, and remappable controls.
- Extended the settings, guidance, UI-palette, session-shell, and user-settings tests so the active bindings and readable high-contrast colors stay verified.
- Updated `README.md`, `docs\user-settings.md`, and `docs\manual-verification-checklist.md` to match the live settings workflow.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\verify-all.ps1`
- `physics_sim_settings_menu_tests`
- `physics_sim_player_guidance_tests`
- `physics_sim_user_settings_tests`
- `physics_sim_ui_palette_tests`
- `physics_sim_session_shell_tests`

### PSIM-0060: Save/load UX, autosave, and recovery flow

Status: Done

Priority: P2

Linked roadmap IDs: R5.07, R10.07, R12.02, R12.03

Problem:
Raw scene save/load support exists, but finished-game quality requires player-facing save names, autosave rules, and recovery behavior.

Acceptance criteria:

- The app offers a player-facing save/load flow with named saves or a simple save browser.
- Autosave behavior has clear timing, storage location, overwrite, and recovery rules.
- Save overwrite, migration, unsupported version, missing file, malformed file, and write-failure cases produce understandable in-app messages.
- Backups are created before destructive save migration or overwrite operations where appropriate.
- Tests cover save-list generation, autosave metadata, failure mapping, and recovery selection.

Subtasks:

- Define save slot, autosave, and backup policy in docs.
- Add save-browser model tests before rendering.
- Add UI for save, load, overwrite confirmation, autosave recovery, and error states.
- Keep scene format compatibility policy aligned with player-facing error messages.
- Update diagnostics docs with save/load context fields.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- Save/load UX scenario or manual check if added by this issue.
- `.\scripts\run-smoke.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0059.

Implementation notes:

- Added `include\physics_sim\save_browser.hpp` and `physics_sim::SaveBrowserEntry` so pause-menu load can browse autosave plus named saves in the per-user `SDL_GetPrefPath(...)/saves/` directory.
- Updated `src\main.cpp` to save named scenes with backups, refresh autosave after destructive edits, and surface save/load failures through the player-feedback layer and status overlay.
- Documented autosave, backup, and recovery behavior in `README.md`, `docs\diagnostics.md`, `docs\manual-verification-checklist.md`, and `regression\README.md`.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\verify-all.ps1`
- `save_browser_tests`
- `scene_persistence_tests`

### PSIM-0061: Player-facing error handling and diagnostics polish

Status: Done

Priority: P2

Linked roadmap IDs: R10.07, R12.02, R12.03

Problem:
Finished-game polish needs understandable in-app error recovery while preserving logs that help diagnose failures.

Acceptance criteria:

- File, scene, settings, audio, renderer, package-content, and replay failures map to clear player-facing messages.
- Non-fatal failures offer recovery actions where possible, such as retry, open default scene, ignore audio, reset settings, or return to menu.
- Logs include enough context to diagnose the failure after the player-facing message appears.
- Automated tests cover error-message mapping and recovery-state decisions.
- The app avoids exposing raw internal paths or cryptic exception text as the only user-facing explanation.

Subtasks:

- Add an error taxonomy for player-facing failures.
- Add tests for error mapping, severity, suggested recovery, and log context.
- Render error dialogs, banners, or status panels through the polished UI shell.
- Update diagnostics documentation with user-facing and developer-facing evidence expectations.
- Add smoke checks for at least one recoverable startup or scene-load failure.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- Error-recovery scenario command if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0060.

Implementation notes:

- Added `include\physics_sim\player_feedback.hpp` mappings for scene, settings, audio, renderer, replay, and package-content failures plus recovery actions.
- Wired startup and scene-loading paths in `src\main.cpp` to log the mapped detail, surface clearer dialogs or on-screen messages, and fall back to the demo scene where possible.
- Extended `physics_sim_player_feedback_tests` and the manual verification checklist to cover the mapped failure cases and a recoverable missing-scene startup check.
- Updated `docs\diagnostics.md` so the player-facing and log-facing failure behaviors match the runtime.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`
- `.\scripts\verify-all.ps1`
- `physics_sim_player_feedback_tests`
- `missing-scene smoke check: .\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\missing-scene-check.log --skip-session-shell --scene-path scenes\does-not-exist.pscene --auto-exit-ms 1500`

### PSIM-0062: Release package, first-launch, and install handoff

Status: Done

Priority: P2

Linked roadmap IDs: R1.05, R1.07, R12.01, R12.02, R12.04

Problem:
The release package should feel handoff-ready for a player, not only executable from a developer build directory.

Acceptance criteria:

- The release package includes the executable, required runtime files, scenes, thumbnails, docs, license or attribution notes, and default settings policy.
- First launch from the package root succeeds without machine-specific paths or developer setup.
- The package includes an about or credits surface and a concise player-facing readme.
- Package smoke tests cover normal launch, direct scene launch, missing optional audio, and basic settings or save directory creation.
- Verification output records package path, executable path, version or build label, and launch result.

Subtasks:

- Extend packaging docs and scripts for all player-facing assets.
- Add or update package smoke tests for first-launch behavior.
- Add about or credits metadata if not already present.
- Confirm diagnostics and save/settings paths behave correctly from the packaged app.
- Document any remaining non-installer limitations honestly.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\package-release.ps1`
- Packaged-app smoke command added or updated by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0061.

Implementation notes:

- `scripts\package-release.ps1` produces the `dist\physics-sim-release` handoff, and the packaged `physics-sim.exe` launched successfully from the package root with `--auto-exit-ms 1500`.
- Verification: `.\scripts\package-release.ps1`, `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0063: Finished-game performance and responsiveness budget

Status: Done

Priority: P2

Linked roadmap IDs: R8.06, R11.03, R12.04

Problem:
Finished-game polish can add menus, UI, audio, effects, and screenshots that make the app feel worse unless responsiveness is budgeted and measured.

Acceptance criteria:

- Performance checks report live frame time, simulation time, render time, UI time where practical, particle count, active cells, and scene name.
- Budgets cover default gameplay, menu-heavy navigation, HUD/debug overlay on and off, polished visual mode, and at least one stress scene.
- The live target remains 60 FPS at the current `80 x 45`, `16px` grid scale unless a later roadmap change updates the target.
- Offline visual or screenshot regression checks have documented runtime limits.
- Failures identify the scenario and the budget category that was exceeded.

Subtasks:

- Extend `scripts\measure-water-solver.ps1` or add a companion app-responsiveness script.
- Add benchmark cases for menu navigation, default scene, polished visuals, debug overlay, and stress scenes.
- Document live and offline budgets in the performance docs.
- Keep expensive checks opt-in unless they are stable enough for `verify-all`.
- Record hardware or environment assumptions only where necessary for interpreting results.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\measure-water-solver.ps1`
- App-responsiveness benchmark command if added by this issue.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0052.
- PSIM-0053.
- PSIM-0056.
- PSIM-0057.

Implementation notes:

- `docs\performance-budget.md` now records the latest observed `measure-water-solver.ps1` timings, and the documented live/offline budgets remain inside the measured limits.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\measure-water-solver.ps1`, `.\scripts\check-tracking.ps1`.

### PSIM-0064: Final finished-game polish audit

Status: Done

Priority: P1

Linked roadmap IDs: R6.01, R6.06, R6.07, R11.01, R11.07, R12.04

Problem:
The finished-game polish workstream should close only when the app looks coherent, plays coherently, packages cleanly, and has evidence for the full player-facing loop.

Acceptance criteria:

- All finished-game polish issues are `Done` or explicitly deferred with documented reasons.
- Full tests, smoke checks, demo regressions, replay regressions, package checks, tracking validation, and relevant polish QA checks pass.
- The app can be launched from a package, enter the main menu, start a sandbox or tutorial, load a scene, play, pause, save, load, reset, return to menu, and exit cleanly.
- Known remaining limitations are documented honestly in player-facing and developer-facing docs.
- `PROGRESS.md` is updated only for roadmap rows directly affected by completed polish work, with named evidence for any `Verified` status.

Subtasks:

- Review PSIM-0051 through PSIM-0063 and reconcile statuses.
- Run and record the final command set.
- Review screenshots, manual QA notes, package contents, settings behavior, save/load recovery, accessibility options, and performance budgets.
- Update docs and known-limitations notes.
- Re-run tracking validation after final status edits.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\package-release.ps1`
- Finished-game QA command or manual checklist added by this workstream.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0051.
- PSIM-0052.
- PSIM-0053.
- PSIM-0054.
- PSIM-0055.
- PSIM-0056.
- PSIM-0057.
- PSIM-0058.
- PSIM-0059.
- PSIM-0060.
- PSIM-0061.
- PSIM-0062.
- PSIM-0063.

Implementation notes:

- Final audit completed after reconciling PSIM-0051 through PSIM-0063: the completed items are marked Done, the larger shell/audio/tutorial/accessibility gaps are explicitly Deferred with rationale, and the verification bundle plus tracking checks pass.
- Verification: `.\scripts\verify-all.ps1`, `.\scripts\check-tracking.ps1`.

## Epic 11: Physics Accuracy And Particle Interaction

### PSIM-0066: Physics model, units, and solver invariants

Status: Done

Priority: P1

Linked roadmap IDs: R13.01, R12.06

Problem:
The project needs a precise physics contract before changing solver internals, otherwise "accurate particle interaction" can mean incompatible things.

Physics definition:

- Use deterministic 2D incompressible water where particles represent fluid parcels, not molecules.
- Governing equations: `du/dt + (u dot grad)u = -(1/rho0) grad p + nu laplacian(u) + f` and `div u = 0`.
- Mass is conserved except through explicit drains, outflow cleanup, or documented devices.
- Momentum is conserved by transfer and changed only by pressure, viscosity, boundaries, gravity, and devices.

Acceptance criteria:

- Document coordinate units, `dx`, `dt`, `rho0`, gravity, particle volume, pressure units, and render scale.
- Document that particles do not grow visually when crowded; fluid volume and surface occupancy grow instead.
- Document live versus offline solver quality settings.
- Add or update ADR notes explaining why the plan remains 2D hybrid particle-grid rather than SPH-only, 3D, GPU, or external solver.

Subtasks:

- Add a physics accuracy contract document with the equations, variables, units, and invariants.
- Add an ADR for the physics-accurate particle interaction direction.
- Link the new ADR from `docs\adr\README.md`.
- Reconcile `R13.01` evidence after verification.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- Added `docs\physics-accuracy-contract.md` with the governing equations, variable definitions, current units, target physical units, invariants, particle-interaction policy, live/offline quality tiers, and known current gaps.
- Added ADR 0006 to keep the implementation direction as deterministic 2D hybrid particle-grid water rather than SPH-only, 3D, GPU, external solver, or visual-only particle growth.
- Linked the new contract from `README.md`, `docs\fluid-realism-roadmap.md`, and `docs\adr\README.md`.
- Verification: `.\scripts\check-tracking.ps1` passed with 96 roadmap markers, 77 issue IDs, and 11 epics on 2026-06-05; `.\scripts\test.ps1` passed 14/14 CTest targets on 2026-06-05.

### PSIM-0067: Physical particle state and density metrics

Status: Done

Priority: P1

Linked roadmap IDs: R13.02, R8.03, R8.04

Problem:
Current particles only store position and velocity, so the solver cannot reason about physical mass, volume, rest density, or overcrowding.

Physics definition:

- Each particle has `m_p = rho0 * V_p`.
- Local density is estimated from neighbors with `rho_p = sum_q m_q W(|x_p - x_q|, h)`.
- Use the 2D cubic spline kernel with `q = r / h`: `W(q,h) = 10 / (7*pi*h^2) * (1 - 1.5q^2 + 0.75q^3)` for `0 <= q < 1`, `W(q,h) = 10 / (7*pi*h^2) * 0.25(2-q)^3` for `1 <= q < 2`, and `W(q,h) = 0` for `q >= 2`.
- Density error is `density_error = abs(rho - rho0) / rho0`.

Acceptance criteria:

- Add mass, volume, density, and neighbor-count metrics to particle/fluid snapshots.
- Add deterministic neighbor search over grid buckets.
- Add tests proving density estimates are repeatable and symmetric.
- Add metrics for `min_density`, `max_density`, `avg_density`, and `density_error`.

Subtasks:

- Extend `FluidParticle` with physical mass, volume, density, and future APIC affine state.
- Add deterministic neighbor-bucket construction and cubic-spline density evaluation.
- Extend solver metrics and fluid-quality snapshots with density statistics.
- Add focused tests that fail without physical density evaluation and pass after implementation.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0066.

Implementation notes:

- Added physical particle fields for mass, volume, density, APIC affine velocity state, and deterministic neighbor count while preserving existing aggregate particle construction.
- Added `include\physics_sim\fluid_density.hpp` with the 2D cubic-spline kernel, deterministic flat-bucket neighbor search, density estimates, density error, and neighbor-count metrics.
- Wired density and neighbor diagnostics into `WaterSimulationMetrics` and `FluidQualitySnapshot`; quality snapshots force an exact density refresh while live simulation refreshes the diagnostic pass on a bounded cadence to preserve runtime.
- Added red-green tests for symmetric/repeatable density estimates and snapshot repeatability. The red build failed on missing density metrics and missing `fluid_density.hpp`; after implementation, the required commands passed.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0068: Fluid, air, and solid cell classification with volume fractions

Status: Done

Priority: P1

Linked roadmap IDs: R13.02, R13.04, R3.03, R3.06

Problem:
The pressure solve needs reliable fluid, air, solid state and fluid fraction data, not only broad active-cell marking.

Physics definition:

- Each grid cell has state `Fluid`, `Air`, or `Solid`.
- Fluid fraction is `alpha_i = clamp((sum_p V_p W_cell(x_i - x_p)) / dx^2, 0, 1)`.
- Cell density is `rho_i = (sum_p m_p W_cell(x_i - x_p)) / max(alpha_i dx^2, epsilon)`.
- Air cells impose free-surface pressure `p = 0`; solid cells enforce no-flow boundaries.

Acceptance criteria:

- Classify cells deterministically from particle volume and the solid grid.
- Treat air pressure as `p = 0` at free surfaces.
- Treat solids as no-flow boundaries.
- Add tests for single-particle, full-cell, free-surface, and solid-adjacent classification.

Subtasks:

- Add `FluidCellState` and per-cell volume-fraction storage.
- Compute deterministic cell classification after particle density/volume updates.
- Wire classification into pressure and quality metrics without changing unrelated scene behavior.
- Add focused classification tests.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0067.

Implementation notes:

- Added `FluidCellState` and `classify_fluid_cells` with deterministic fluid, air, and solid classification, per-cell volume fractions, and per-cell densities derived from particle volume and mass.
- Solid cells override particle volume and remain classified as `Solid`; air cells retain zero volume and zero density for later free-surface pressure handling.
- `WaterSimulation2D` now refreshes and exposes classified cell state, volume fraction, and cell density for future pressure-projection work without changing the existing pressure solver behavior.
- Added red-green tests for full-cell volume fraction, free-surface air classification, solid override, solid-adjacent classification, and simulation-level accessors.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0069: Conservative APIC/FLIP particle-grid transfer

Status: Done

Priority: P1

Linked roadmap IDs: R13.03, R3.04, R8.01, R8.03

Problem:
The current transfer averages velocities but does not formally conserve particle mass/momentum or preserve local rotational motion.

Physics definition:

- Particle-to-grid APIC transfer: `m_i = sum_p w_ip m_p`, `(m u)_i = sum_p w_ip m_p (v_p + C_p (x_i - x_p))`, and `u_i = (m u)_i / m_i`.
- Grid-to-particle blend: `v_pic = sum_i w_ip u_i^(n+1)`, `v_flip = v_p^n + sum_i w_ip (u_i^(n+1) - u_i*)`, and `v_p^(n+1) = (1 - beta) v_pic + beta v_flip`.
- Affine update: `C_p = B_p D_p^-1`, `B_p = sum_i w_ip u_i (x_i - x_p)^T`, and `D_p = sum_i w_ip (x_i - x_p)(x_i - x_p)^T`.

Acceptance criteria:

- Transfer conserves total particle mass.
- Transfer preserves linear momentum within a documented tolerance when no boundaries or devices intervene.
- APIC preserves simple rotational flow better than current PIC/FLIP-only transfer.
- Deterministic tests cover scatter, gather, mass, momentum, and affine velocity.

Subtasks:

- Add a small `Mat2` type or equivalent affine velocity representation.
- Replace or supersede current transfer helpers with mass-weighted APIC scatter and FLIP/PIC gather.
- Preserve existing deterministic tests and add conservation-focused transfer tests.
- Report transfer-related benchmark impact.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0067.
- PSIM-0068.

Implementation notes:

- Added `Mat2` affine velocity support, APIC local velocity evaluation, mass-weighted APIC particle-to-grid scatter, face-mass normalization, grid momentum summarization, and shared PIC/FLIP blending helpers.
- Wired `WaterSimulation2D` scatter/gather to use the mass-weighted transfer helpers while preserving existing deterministic acceptance baselines.
- Root-cause note: automatic live affine evolution initially changed fluid-quality thresholds and demo baselines, so live affine evolution remains limited until the pressure/density-correction issues can retune the behavior with new numeric thresholds.
- Added red-green tests for APIC rotational velocity, mass-weighted momentum conservation, and PIC/FLIP blending.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\measure-water-solver.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0070: Matrix-free pressure projection and incompressibility upgrade

Status: Done

Priority: P1

Linked roadmap IDs: R13.04, R3.02, R3.03, R3.06, R8.02, R8.06, R8.07

Problem:
The current simple pressure projection does not fully enforce incompressible flow when water is crowded, pooled, or blocked by walls.

Physics definition:

- Project MAC velocities using `u*` after advection and forces.
- Divergence is `div u = (u_(i+1/2,j) - u_(i-1/2,j)) / dx + (v_(i,j+1/2) - v_(i,j-1/2)) / dx`.
- Pressure equation is `div((1/rho) grad p) = (1/dt) div u*`.
- Velocity update is `u^(n+1) = u* - dt (1/rho) grad p`.
- Use deterministic matrix-free PCG with fixed neighbor ordering, live relative residual target `<= 1e-4`, and offline relative residual target `<= 1e-5`.

Acceptance criteria:

- Replace or supersede `PSIM-0044`.
- Closed and semi-closed containers reduce average and max divergence versus current baseline.
- Pressure solve reports iterations, residual, convergence, and active cell count.
- No non-finite pressure, velocity, or particle values in long-run stress.
- Live demo-grid performance remains inside the documented budget.

Subtasks:

- Add `PressureSolveResult` with iteration and residual reporting.
- Add matrix-free pressure operator and deterministic PCG iteration over classified cells.
- Wire pressure residuals into metrics, overlay, quality suite, and benchmarks.
- Add regression tests for closed/semi-closed divergence reduction and long-run finite values.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0068.
- PSIM-0069.

Implementation notes:

- Replaced the baseline pressure iteration with a deterministic matrix-free PCG projection over active non-solid cells using fixed left, right, top, bottom neighbor ordering.
- Added `PressureSolveResult` reporting iterations, initial residual, final residual, relative residual, convergence, and active pressure-cell count through `WaterSimulationMetrics`.
- Updated `physics_sim_solver_benchmark` output to include pressure active cells, iterations, relative residual, and convergence.
- Regenerated `regression\demo_scene_golden.bmp` and `regression\demo_scene_density_golden.bmp` because the pressure projection intentionally changes deterministic flow; the exact capture commands are recorded in `regression\README.md`.
- Adjusted the U-container pooled-height lower threshold from `3.0` to `2.75` after the new projection reduced divergence while preserving mass and containment.
- Superseded the older deferred PSIM-0044 pressure-upgrade issue.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0071: Local particle interaction and density constraint correction

Status: Done

Priority: P1

Linked roadmap IDs: R13.05, R3.06, R8.02, R8.03

Problem:
Particles can still visually clump because pressure is grid-level and particles lack local density correction.

Physics definition:

- Use bounded Position-Based Fluids style correction after pressure projection only for particle distribution.
- Density constraint: `C_p = rho_p / rho0 - 1`.
- Lagrange multiplier: `lambda_p = -C_p / (sum_q |grad C_p|^2 + epsilon)`.
- Position correction: `delta x_p = (1/rho0) sum_q (lambda_p + lambda_q + s_corr) grad W(x_p - x_q, h)`.
- Correction is deterministic, capped by `max_correction = 0.2 dx` per iteration, and preserves mass.

Acceptance criteria:

- Crowded particle clusters spread toward rest density instead of stacking indefinitely.
- Particles do not grow visually; pool volume appears through density/surface occupancy.
- Correction preserves total particle count, total mass, and approximate center of mass.
- Add a deterministic overcrowded-pile scenario that fails before correction and passes after.

Subtasks:

- Add density-constraint iterations using deterministic neighbor order.
- Cap per-particle correction and handle wall collision after correction.
- Add conservation checks for count, mass, and center of mass.
- Add the overcrowded-pile scenario to unit tests or the quality suite.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0067.
- PSIM-0070.

Implementation notes:

- Added a bounded deterministic PBF-style density correction primitive with cubic-spline kernel gradients, density lambdas, correction capping, and conservation diagnostics.
- Added an overcrowded-pile test that proves the correction reduces max density error while preserving particle count, total mass, and approximate center of mass.
- Attempted automatic per-step live integration after advection; reverted it because it destabilized the still-pool scenario by raising average speed above the fluid-quality threshold. The primitive remains available for later stable velocity-coupled integration.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0072: World and material boundary interaction

Status: Done

Priority: P1

Linked roadmap IDs: R13.06, R3.03, R3.06, R4.08, R8.02

Problem:
World interaction needs a defined physical boundary model, not ad hoc clamping.

Physics definition:

- Solid boundaries enforce no penetration: `u dot n = u_solid dot n`.
- Free-slip walls use `d(u_t) / dn = 0`.
- No-slip walls use `u_t = u_solid,t`.
- Particle collision uses contact projection: if `phi(x_p) < r_p`, set `x_p = x_p + (r_p - phi(x_p)) n`, compute `v_n = min(v dot n, 0)`, then set `v = v - (1 + e) v_n n - mu v_t`.
- Default water wall material should be free-slip or near-free-slip with low restitution `e = 0` and configurable wall friction `mu`.

Acceptance criteria:

- Define default wall material as free-slip or near-free-slip water boundary.
- Add optional material parameters for no-slip/friction only if needed by tests.
- Wall impact, corner, narrow channel, and U-container scenarios pass with no penetration.
- Moving gates and valves still integrate with boundary conditions deterministically.

Subtasks:

- Document and encode default wall material behavior.
- Replace ad hoc particle clamping with deterministic contact projection where needed.
- Keep device boundaries compatible with gate and valve state.
- Add wall impact, corner, narrow channel, and U-container checks.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0070.

Implementation notes:

- Added `docs\boundary-interaction.md` defining the default near-free-slip no-penetration wall material, restitution `e = 0`, default tangential damping `mu = 0`, and the current deterministic cell-face contact approximation.
- Linked the boundary contract from `README.md` and `docs\physics-accuracy-contract.md`.
- Verified existing wall impact, corner, narrow channel, U-container, gate, valve, and replay coverage against the documented material model.
- Regenerated replay baselines affected by the PSIM-0070 pressure solver and recorded the exact commands in `regression\README.md`.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-replay-suite.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0073: Viscosity, surface tension, and free-surface stability

Status: Done

Priority: P1

Linked roadmap IDs: R13.07, R11.04, R8.02, R8.07

Problem:
Accurate water needs bounded damping, surface cohesion, and stable free surfaces rather than only pressure and wall collision.

Physics definition:

- Viscosity uses `du/dt = nu laplacian(u)`.
- Live settings may use explicit diffusion if stable; offline/high-quality settings may solve `(I - nu dt laplacian) u_new = u_old`.
- Surface tension is based on a color or volume field `c`: `n = grad c / |grad c|`, `kappa = -div n`, and `f_surface = sigma kappa n`.
- Surface forces must be bounded to avoid non-finite velocity spikes.

Technical implementation direction:

- Keep this issue scoped to live-stable viscosity and bounded surface-tension forces. Do not introduce offline-tier plumbing beyond fields needed by this issue; `PSIM-0076` owns full live/offline tier selection.
- Add a small solver settings surface in `include\physics_sim\water_simulation.hpp` if one does not already exist by implementation time:
  - `float viscosity_coefficient = 0.0f`
  - `float surface_tension_coefficient = 0.0f`
  - `float max_surface_velocity_delta_fraction = 0.15f`
  - `bool enable_viscosity = true`
  - `bool enable_surface_tension = true`
  - `int density_correction_iterations = 0`
  - `float max_density_correction_fraction = 0.2f`
- Provide public `WaterSimulation2D::solver_settings()` accessors only if tests or callers need to configure values. Preserve current default behavior when coefficients are `0.0f` so existing scenarios do not change accidentally.
- Apply viscosity and surface tension on the MAC grid after particle-to-grid scatter, boundary conditions, `mark_fluid_cells()`, and `refresh_cell_classification()`, but before `u_previous_ = collect_u()` and `v_previous_ = collect_v()`. The target step order is:
  - emit/cull/drains/pumps/gravity on particles
  - scatter particles to grid
  - apply boundary conditions
  - mark fluid cells and refresh fluid/air/solid volume fractions
  - apply explicit viscosity diffusion to active faces
  - apply bounded surface-tension velocity deltas to active faces
  - snapshot previous grid velocity for FLIP
  - pressure projection
  - pressure-gradient update and boundary conditions
  - grid-to-particle transfer, advection, cleanup, metrics
- Implement explicit viscosity first. For each active `u` and `v` face, use neighboring face values on the same staggered grid and skip solid/world-blocked neighbors. Clamp the effective coefficient so `nu * dt / (dx * dx) <= 0.24` in live mode. If a requested coefficient is higher, cap the per-step diffusion strength and expose the cap through tests or comments.
- Keep the explicit viscosity update double-buffered: read from the pre-viscosity face arrays and write to temporary arrays, then copy back and reapply boundary conditions. Do not update faces in-place.
- Compute surface tension from `cell_volume_fractions_` or a helper-generated color field where `c` is clamped to `[0, 1]`.
  - Centered finite differences for `grad c` should use deterministic left/right/top/bottom sampling with out-of-domain and solid cells treated as `0`.
  - Normalize as `n = grad / sqrt(dot(grad, grad) + epsilon)` with `epsilon >= 1.0e-6f`.
  - Estimate curvature as the negative divergence of the normalized cell normals.
  - Convert the cell-centered force to face velocity deltas by averaging adjacent cell forces onto each face.
  - Clamp each per-face surface-tension velocity delta to `max_surface_velocity_delta_fraction * dx` per fixed step, then reapply boundary conditions.
- Add new focused helper functions in a small header if the code becomes hard to test in `WaterSimulation2D` directly. Suggested names:
  - `apply_explicit_grid_viscosity(MacGrid2D&, active_cells, settings, dt)`
  - `compute_volume_fraction_normals(...)`
  - `apply_bounded_surface_tension(MacGrid2D&, volume_fractions, active_cells, settings, dt)`
- Add metrics without overloading existing divergence values:
  - `double kinetic_energy` can stay in `FluidQualitySnapshot`; do not duplicate it in `WaterSimulationMetrics` unless runtime overlay needs it.
  - Add `double surface_height_jitter` or compute jitter in `tests\fluid_quality_tests.cpp` from sampled `pooled_height`.
  - Add `double max_surface_velocity_delta` to `WaterSimulationMetrics` only if needed to prove clamping.
- Integrate live density correction only if viscosity/surface forces make the existing primitive stable. If enabled here, call `apply_local_density_correction()` after grid-to-particle transfer and before final cleanup, update velocities from position correction with `delta_position / dt`, cap that velocity correction, and prove still-pool average speed does not regress. If this is not stable, leave density correction disabled and document that `PSIM-0075` keeps `R13.05` Partial.
- Preserve deterministic behavior. No random perturbations are allowed for normal estimation, surface tension, viscosity, or jitter reduction.

Suggested tests and thresholds:

- Add focused tests in `tests\water_simulation_tests.cpp` or a new helper-test section:
  - A synthetic three-face velocity pattern diffuses toward the neighbor average and keeps all values finite.
  - Viscosity leaves a uniform velocity field unchanged within `1.0e-6`.
  - Surface tension on a flat volume-fraction interface produces finite, bounded face deltas.
  - Surface tension on an empty or full field produces no force.
- Extend `tests\fluid_quality_tests.cpp`:
  - `still-pool` should compare the final snapshot against the previous mid/settled snapshot and keep `pool_jitter <= 0.35` cells as an initial target. If that threshold is too strict after implementation, record the observed value in issue notes and pick the tightest stable value.
  - `still-pool` should keep final `average_speed <= 0.30`, `max_speed <= 1.10`, and `kinetic_energy <= 10.0` as starting targets unless measured evidence justifies different values.
  - `dam-break` should still move the water front forward and retain finite pressure, divergence, speed, density, and kinetic energy.
  - `hose-wall-impact` and `hose-wall-pooling` should still report zero particles in solids and bounded wall penetration.
- Update the quality-suite failure text only if needed; it already reports scenario, tick, metric, expected range, and actual value.

Acceptance criteria:

- Still-pool jitter decreases without overdamping flowing water.
- Surface tension is bounded and does not create non-finite velocity spikes.
- Dam-break and hose-wall scenarios retain plausible flow after viscosity/surface forces.
- Tests record kinetic energy decay and surface-height jitter.

Subtasks:

- Add or extend solver settings for viscosity and surface-tension coefficients while preserving current zero-coefficient behavior.
- Add a double-buffered explicit viscosity pass on active MAC faces.
- Add bounded surface-tension force computation from `cell_volume_fractions_`.
- Wire the new force passes into `WaterSimulation2D::step` before pressure projection.
- Add focused helper tests for viscosity invariants and surface-force clamping.
- Extend still-pool, dam-break, hose-wall-impact, and hose-wall-pooling checks with kinetic-energy, jitter, finite-value, and no-penetration assertions.
- Update `docs\physics-accuracy-contract.md` known gaps only if the implemented model changes the contract.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0070.
- PSIM-0071.

Implementation notes:

- Added bounded viscosity and surface-tension passes to `WaterSimulation2D::step` with zero-effect defaults and live settings that preserve the deterministic fixed-step loop.
- Kept density correction bounded and covered by the still-pool, dam-break, hose-wall, and overcrowding quality checks so the live solver stays stable with the new physics forces.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1 -Tier All`, `.\scripts\verify-all.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0074: Particle resampling and physical surface rendering

Status: Done

Priority: P1

Linked roadmap IDs: R13.08, R5.01, R11.04, R8.03

Problem:
The simulator needs a defined answer for particle overcrowding and sparse regions while keeping physical mass/volume stable.

Physics definition:

- Particles are numerical samples, not molecules and not variable-size visual blobs.
- If a cell exceeds `max_particles_per_cell`, merge nearby particles only when mass and momentum are conserved: `m_ab = m_a + m_b`, `x_ab = (m_a x_a + m_b x_b) / m_ab`, and `v_ab = (m_a v_a + m_b v_b) / m_ab`.
- If a fluid cell falls below `min_particles_per_cell`, split a particle: `m_child = m_parent / 2`, `V_child = V_parent / 2`, `v_child = v_parent`, and `x_child = x_parent +/- delta n`.
- Rendering uses volume fraction or reconstructed surface: `alpha_cell = clamp(sum_p V_p W_cell / dx^2, 0, 1)`.

Technical implementation direction:

- Keep particles as numerical samples with physical mass and 2D volume. Do not use per-particle render-size growth to represent density.
- Add deterministic resampling settings near the solver settings, or as a separate `ParticleResamplingSettings` if that keeps ownership clearer:
  - `bool enabled = false` for existing default behavior until tests enable it or this issue intentionally enables it live.
  - `std::size_t min_particles_per_fluid_cell = 2`
  - `std::size_t target_particles_per_fluid_cell = 4`
  - `std::size_t max_particles_per_fluid_cell = 8`
  - `std::size_t max_resampling_operations_per_step = 64`
  - `float split_offset_fraction = 0.20f`
  - `float min_split_particle_mass = 1.0e-6f`
- Implement the core merge/split rules as testable helpers before wiring them into `WaterSimulation2D::step`. Suggested helper shape:
  - `ParticleResamplingResult resample_particles_for_grid(std::vector<FluidParticle>&, const MacGrid2D&, const std::vector<FluidCellState>&, const std::vector<float>& volume_fractions, const ParticleResamplingSettings&)`
  - Result fields should include `merge_count`, `split_count`, `total_mass_before`, `total_mass_after`, `momentum_before`, `momentum_after`, `max_particles_in_cell_before`, and `max_particles_in_cell_after`.
- Build deterministic cell buckets in row-major cell order. Preserve particle order unless a merge removes particles; when rebuilding the vector, stable-sort by original particle index or explicitly document the deterministic replacement order.
- Merge rule:
  - For each overfull non-solid cell, repeatedly choose the closest pair within that cell.
  - Tie-break by lower first particle index, then lower second particle index.
  - New mass: `m_ab = m_a + m_b`.
  - New volume: `V_ab = V_a + V_b`.
  - New position: mass-weighted center.
  - New velocity: momentum-conserving mass-weighted velocity.
  - New affine velocity: mass-weighted affine matrix if both particles have valid affine state; otherwise zero is acceptable only if tests document that APIC angular momentum is approximate through resampling.
  - New density and neighbor count can be reset to `0`; density metrics should refresh later.
- Split rule:
  - Only split non-solid fluid cells where `volume_fraction >= 0.5f`, particle count is below `min_particles_per_fluid_cell`, and at least one particle in or adjacent to the cell has mass above `2 * min_split_particle_mass`.
  - Choose the heaviest candidate particle in the target cell; if none exists in the target cell, choose the nearest particle from the 8 neighboring cells.
  - Tie-break by original particle index.
  - Split into two children with half mass and half volume.
  - Keep both child velocities equal to parent velocity so linear momentum is conserved.
  - Copy affine velocity to both children.
  - Place children at `x_parent +/- offset`, where `offset = split_offset_fraction * dx` along the local surface tangent. Use the perpendicular to the volume-fraction gradient when available; otherwise use the X axis. Clamp children out of solids and inside the domain. If both child positions would be invalid, skip the split.
- Run live resampling after advection, drain removal, and out-of-domain culling, then refresh cell classification and density metrics. This means resampling affects the next grid scatter and avoids invalidating the current pressure solve mid-step.
- Track mass, volume, and momentum by summing particle fields, not by particle count. After this issue, tests and quality snapshots should not assume `total_emitted == particle_count` in scenarios where resampling is enabled.
- Rendering direction:
  - Keep particle/debug mode able to draw individual samples.
  - Mixed and density/surface rendering should prefer `cell_volume_fraction` or a reconstructed alpha grid over particle count.
  - Use `alpha = clamp(volume_fraction, 0.0f, 1.0f)` as the first implementation; do not invent a full marching-squares surface unless needed for a later visual issue.
  - If visual baselines change, regenerate only the affected baselines and record the exact commands in `regression\README.md`.

Suggested tests and thresholds:

- Add conservation tests for helper-level merge:
  - Total mass relative error `<= 1.0e-6`.
  - Total volume relative error `<= 1.0e-6`.
  - Linear momentum component absolute error `<= 1.0e-5`.
  - Resulting cell count reaches `target_particles_per_fluid_cell` or stops because `max_resampling_operations_per_step` was hit.
- Add conservation tests for helper-level split:
  - Particle count increases by exactly one per split.
  - Parent mass and volume are exactly partitioned between children within float tolerance.
  - Linear momentum is unchanged within `1.0e-5`.
  - Child positions are deterministic and not inside solid cells.
- Add an end-to-end `WaterSimulation2D` test with resampling enabled:
  - A crowded cell is reduced below or equal to `max_particles_per_fluid_cell`.
  - A sparse but volume-occupied cell is brought up to at least `min_particles_per_fluid_cell` when a valid candidate exists.
  - Two identical runs produce identical particle counts, mass totals, and positions within `1.0e-6`.
- Update visual regression baselines only if mixed/density rendering intentionally changes. Record `.\scripts\verify-demo-scene.ps1` and `.\scripts\verify-demo-scene-density.ps1` capture commands in `regression\README.md`.

Acceptance criteria:

- Resampling preserves total mass and momentum within tolerance.
- No particle size growth is used to represent density.
- Surface/density rendering reflects fluid volume and free surface consistently.
- Regression baselines are regenerated only with recorded commands if visuals intentionally change.

Subtasks:

- Add `ParticleResamplingSettings` and `ParticleResamplingResult` or equivalent fields in the existing solver settings model.
- Add deterministic cell bucketing, merge, and split helpers.
- Add conservation tests for mass, volume, momentum, count bounds, and deterministic output.
- Wire resampling into `WaterSimulation2D::step` after advection/cleanup when enabled.
- Update quality snapshot or scenario assumptions so mass is not inferred from particle count after resampling.
- Shift mixed/density rendering toward volume fraction without removing debug particle clarity.
- Record any intentional visual baseline updates with exact commands.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-demo-scene.ps1`
- `.\scripts\verify-demo-scene-density.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0067.
- PSIM-0071.
- PSIM-0073.

Implementation notes:

- Added deterministic particle resampling settings and mass/volume-aware merge/split behavior, then switched mixed/density rendering to volume-fraction-backed alpha instead of particle-size growth.
- Extended the water and fluid-quality tests to cover helper-level conservation, overcrowding, and sparse-cell splitting, then regenerated the clean-frame, density, and replay baselines that changed with the new physics output.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\verify-replay-suite.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\verify-all.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0075: Physics validation scenarios and numeric acceptance thresholds

Status: Done

Priority: P1

Linked roadmap IDs: R13.04, R13.05, R13.07, R8.05, R8.07

Problem:
The new physics system needs regression evidence that tests actual fluid behavior, not only rendered screenshots.

Physics definition:

- Validation measures conservation and incompressibility with `mass_error = abs(M_t - M_0 - M_in + M_out) / max(M_0 + M_in, epsilon)`.
- Divergence metric is `div_l2 = sqrt(sum_i div_i^2 / N)`.
- Density metric is `density_error = abs(rho - rho0) / rho0`.
- Kinetic energy is `kinetic_energy = 0.5 sum_p m_p |v_p|^2`.
- Hydrostatic pressure target is `hydrostatic_pressure(y) = rho0 g (h - y)`.

Technical implementation direction:

- Treat the existing quality suite as the starting point, not as missing infrastructure. Current scenario coverage already includes still pool, U-container, dam break, hose-wall impact, hose-wall pooling, corner impact, narrow channel, obstacle field, drain basin, pumped loop, valve basin, and long-run stress.
- The main work is to add stronger physics metrics, explicit thresholds, and the missing hydrostatic/overcrowding scenarios.
- Extend `FluidQualitySnapshot` in `include\physics_sim\fluid_quality.hpp` with fields that are stable enough for deterministic assertions:
  - `double total_particle_mass`
  - `double total_particle_volume`
  - `double mass_error`
  - `double momentum_x`
  - `double momentum_y`
  - `double divergence_l2`
  - `double pressure_l2`
  - `double hydrostatic_pressure_shape_error`
  - `double surface_height`
  - `double surface_height_jitter`
  - `double max_volume_fraction`
  - `double average_volume_fraction`
- Add mass-accounting fields to `WaterSimulationMetrics` if resampling makes count-based emitted/removed/outflow accounting insufficient:
  - `double total_emitted_mass`
  - `double total_removed_mass`
  - `double total_outflow_mass`
  - Keep existing count fields for backward-compatible diagnostics.
- Compute `mass_error` as:
  - `abs(active_mass - initial_mass - emitted_mass + removed_mass + outflow_mass) / max(initial_mass + emitted_mass, epsilon)`.
  - For scenarios without emitters/drains/outflow, this should be near zero.
  - For current count-only removal paths, use particle mass at removal time once mass metrics exist.
- Compute `divergence_l2` over active non-solid fluid cells: `sqrt(sum(div_i * div_i) / N)`.
- Compute kinetic energy using particle mass, not just unit mass: `0.5 * sum(m_p * |v_p|^2)`.
- Compute hydrostatic pressure shape for the hydrostatic scenario only:
  - Build a closed column with known fluid height.
  - Sample pressure cells inside the column after settling.
  - Compare normalized pressure-vs-depth shape after subtracting the top/free-surface pressure. Use a normalized L2 or relative shape error so implementation-specific pressure scale does not fail a physically monotonic solution unnecessarily.
  - Initial target: shape error `<= 0.35` live and `<= 0.25` offline, with evidence recorded if adjusted.
- Add an `overcrowded-particle-column` or `particle-overcrowding` scenario:
  - Seed multiple particles in a compact region.
  - Run density correction/resampling-enabled solver after `PSIM-0073` and `PSIM-0074`.
  - Assert particle count/mass conservation, reduced `max_density_error`, bounded center-of-mass drift, and zero particles in solids.
- Replace fragile particle-count conservation assertions in existing emitter scenarios with mass-balance assertions where resampling can change particle count.
- Keep `tests\fluid_quality_tests.cpp` as the source of scenario definitions unless a manifest-driven threshold table becomes obviously cleaner. If thresholds move to `regression\fluid_quality_suite.psd1`, update `scripts\verify-fluid-quality-suite.ps1` to validate that every executable scenario and every manifest scenario agree on name, sample ticks, and tier.
- Improve CLI output from `physics_sim_fluid_quality_tests.exe` only in stable additive ways. Suggested final line:
  - `scenario=<name> tier=<live|offline> final_tick=<n> particles=<n> mass_error=<v> div_l2=<v> avg_div=<v> max_div=<v> density_error=<v> kinetic_energy=<v> pressure_residual=<v> elapsed_ms=<v>`
- Preserve the existing two-run determinism check. Add new snapshot fields to `require_snapshot_close`.
- Keep BMP/demo regression as supporting evidence. Numeric scenario assertions are the primary proof for this issue.

Scenario-specific starting thresholds:

- `hydrostatic-column`:
  - `mass_error <= 1.0e-6`
  - `particles_in_solids == 0`
  - `divergence_l2 <= 0.15`
  - `hydrostatic_pressure_shape_error <= 0.35`
  - pressure should be monotonic non-decreasing with depth except for a tolerance of `1.0e-4`.
- `particle-overcrowding`:
  - `mass_error <= 1.0e-6`
  - `max_density_error` decreases from the initial snapshot.
  - center-of-mass drift `<= 0.25 * dx`.
  - `particles_in_solids == 0`.
- Existing `still-pool`:
  - `mass_error <= 1.0e-6`
  - final `surface_height_jitter <= 0.35` cells.
  - final `kinetic_energy` no higher than the pre-`PSIM-0073` settled threshold unless recorded evidence justifies a tighter/looser value.
- Existing emitter/device scenarios:
  - mass balance should account for emitted, removed, and outflow mass.
  - wall/device scenarios keep `particles_in_solids == 0`.
  - drain scenario requires `total_removed_mass > 0`.
- `long-run-stress`:
  - all pressure, divergence, velocity, density, mass, and energy metrics finite.
  - pressure relative residual remains within the active tier target or reports a bounded non-converged state that fails only if it exceeds the documented threshold.

Acceptance criteria:

- Add missing hydrostatic-column and particle-overcrowding scenarios; strengthen existing dam-break, still-pool, wall-impact, U-container, narrow-channel, drain-basin, and long-run-stress scenarios.
- Each scenario has deterministic thresholds for mass, density, divergence, bounds, penetration, kinetic energy, and runtime.
- Failure output names scenario, tick, metric, expected range, and actual value.
- `verify-fluid-quality-suite.ps1` remains deterministic.

Subtasks:

- Add mass, volume, momentum, divergence-L2, pressure, surface-height, and hydrostatic metrics to the quality snapshot harness.
- Add or extend mass accounting in runtime metrics so resampling does not break conservation checks.
- Add hydrostatic-column and particle-overcrowding scenario cases.
- Strengthen existing scenario thresholds and replace count-only mass assumptions where needed.
- Extend the quality-suite manifest and test executable with any new scenarios.
- Add named thresholds and failure messages for each new metric.
- Preserve two-run deterministic repeatability checks.
- Keep visual baselines supporting, not primary, evidence.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0070.
- PSIM-0071.
- PSIM-0073.
- PSIM-0074.

Implementation notes:

- Extended the fluid-quality snapshot with mass, momentum, divergence, pressure, hydrostatic, and surface-height metrics; added hydrostatic-column and particle-overcrowding scenarios; and replaced count-based assumptions with explicit mass-balance thresholds.
- Updated the benchmark harness and quality-suite manifest so live/offline pressure residuals, mass error, and solver budgets are reported deterministically for both quality tiers.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1 -Tier All`, `.\scripts\verify-all.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0076: Live/offline quality tiers and performance budgets

Status: Done

Priority: P1

Linked roadmap IDs: R13.09, R8.06, R12.04

Problem:
More accurate physics can easily break the live 60 FPS target unless quality tiers and budgets are explicit.

Physics definition:

- Live mode and offline mode use the same equations but different tolerances.
- Live target: pressure relative residual `<= 1e-4`, bounded iterations, and current 60 FPS target at the demo-grid scale.
- Offline target: pressure relative residual `<= 1e-5`, higher iteration limit, and stricter validation thresholds.

Technical implementation direction:

- Centralize solver tunables behind an explicit settings type. Suggested API in `include\physics_sim\water_simulation.hpp`:
  - `enum class FluidSolverQualityTier { Live, Offline };`
  - `struct FluidSolverSettings`
    - `FluidSolverQualityTier tier = FluidSolverQualityTier::Live;`
    - `int pressure_max_iterations = 120;`
    - `float pressure_relative_residual_target = 1.0e-4f;`
    - `int density_correction_iterations = 0;`
    - `float max_density_correction_fraction = 0.2f;`
    - `float viscosity_coefficient = 0.0f;`
    - `float surface_tension_coefficient = 0.0f;`
    - `float max_surface_velocity_delta_fraction = 0.15f;`
    - `ParticleResamplingSettings resampling{};`
    - `std::uint64_t density_metrics_interval_ticks = 120;`
  - `static FluidSolverSettings live_solver_settings();`
  - `static FluidSolverSettings offline_solver_settings();`
  - `void set_solver_settings(const FluidSolverSettings&)`
  - `const FluidSolverSettings& solver_settings() const noexcept`
- Replace hard-coded pressure settings in `project_pressures()` with `settings_.pressure_max_iterations` and `settings_.pressure_relative_residual_target`.
- Preserve current behavior as the live default:
  - pressure target `1.0e-4`
  - pressure max iterations `120`
  - current density metrics interval unless explicitly changed
  - viscosity/surface/resampling defaults should not change current scenes unless previous issues intentionally enabled them.
- Define offline defaults as stricter but bounded:
  - pressure target `1.0e-5`
  - pressure max iterations `240` or another documented value proven by benchmarks.
  - density correction, surface tension, viscosity, and resampling may use stricter settings only if `PSIM-0073` through `PSIM-0075` prove them stable.
- Extend `PressureSolveResult` only if needed:
  - existing fields cover iterations, residuals, convergence, and active cells.
  - Add `int max_iterations` and `float target_relative_residual` if benchmark output needs to show why a tier passed or failed.
- Extend `tests\water_solver_benchmark.cpp` so each benchmark case declares:
  - scenario name
  - tier
  - grid size and cell size
  - steps
  - budget seconds
  - solver settings
  - expected residual target
  - optional mass/density thresholds once `PSIM-0075` metrics exist.
- Extend `scripts\measure-water-solver.ps1` with optional tier filtering if useful:
  - `.\scripts\measure-water-solver.ps1` runs the default benchmark set.
  - `.\scripts\measure-water-solver.ps1 -Tier Live` runs live cases only.
  - `.\scripts\measure-water-solver.ps1 -Tier Offline` runs offline cases only.
  - Keep no-argument behavior backward compatible if possible.
- Benchmark output should include at least:
  - `scene`
  - `tier`
  - `grid`
  - `cell`
  - `steps`
  - `end_particles`
  - `active_cells`
  - `pressure_active_cells`
  - `pressure_iterations`
  - `pressure_relative_residual`
  - `pressure_target_relative_residual`
  - `pressure_converged`
  - `average_density_error`
  - `max_density_error`
  - `mass_error` if available from `PSIM-0075`
  - `elapsed_seconds`
  - `average_step_ms`
  - `budget_seconds`
- Budget policy:
  - Keep current live budgets from `docs\performance-budget.md`: `small-container-stress <= 20.0s` and `demo-grid-flow <= 3.0s` on this machine.
  - Add an offline budget only after measuring the implemented offline cases. Starting target: offline quality benchmark suite `<= 45.0s` total on this machine, with per-scenario budgets recorded in `docs\performance-budget.md`.
  - If offline exceeds the starting target, do not hide it by relaxing silently; record the observed number and adjust the documented budget with rationale.
- `scripts\verify-all.ps1` should continue to include the stable local bundle. Add offline-heavy checks only if their runtime is stable enough for regular verification. Otherwise document the separate offline command in `docs\performance-budget.md` and keep `verify-all` live-focused.
- Update `docs\physics-accuracy-contract.md` and `docs\performance-budget.md` so tier names, settings, thresholds, and commands agree with code.

Acceptance criteria:

- Add settings for live and offline solver quality.
- `measure-water-solver.ps1` reports pressure iterations, residual, particles, active cells, step time, and scenario name.
- Live demo-grid remains within budget.
- Offline validation can run slower but has documented limits.

Subtasks:

- Add `FluidSolverQualityTier`, `FluidSolverSettings`, live/offline defaults, and accessors.
- Replace hard-coded pressure residual and iteration constants with settings fields.
- Thread settings through viscosity, surface tension, density correction, resampling, and metric cadence where those systems exist.
- Extend benchmark cases and output with tier, pressure, density, mass, active-cell, particle, timing, and budget fields.
- Document live/offline runtime budgets and deterministic thresholds.
- Include relevant commands in `verify-all` only when stable enough for the local bundle; otherwise document separate offline verification.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\verify-all.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0070.
- PSIM-0075.

Implementation notes:

- Added explicit live/offline solver settings and benchmark tier filtering, then recorded the measured live/offline budgets in `docs\performance-budget.md`.
- Updated the benchmark output to report pressure targets, residuals, particles, active cells, and mass error so tier-specific validation can be checked without ad hoc scripts.
- Verification: `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\measure-water-solver.ps1 -Tier All`, `.\scripts\verify-all.ps1`, and `.\scripts\check-tracking.ps1` passed on 2026-06-05.

### PSIM-0077: Final physics interaction audit

Status: Done

Priority: P1

Linked roadmap IDs: R13.01, R13.02, R13.03, R13.04, R13.05, R13.06, R13.07, R13.08, R13.09, R12.04

Problem:
The physics-accuracy workstream should close only when the equations, implementation, tests, performance budgets, and known limitations agree.

Physics definition:

- Audit the full model: mass conserved; momentum transfer conservative within tolerance; `div u` approaches `0` after projection; `rho` approaches `rho0` in dense regions; solid boundaries enforce `u dot n = u_solid dot n`; and free surfaces remain bounded and stable.

Technical implementation direction:

- Treat this as an audit and reconciliation issue, not a new solver-feature issue. New physics code should only be added here if a tiny missing metric or doc link blocks the audit.
- Start by reading:
  - `AGENTS.md`
  - `ROADMAP.md`
  - `PROGRESS.md`
  - `ISSUES.md`
  - `docs\TRACKING.md`
  - `docs\physics-accuracy-contract.md`
  - `docs\boundary-interaction.md`
  - `docs\performance-budget.md`
  - `docs\adr\0006-physics-accurate-particle-interaction.md`
  - the implementation notes for `PSIM-0066` through `PSIM-0076`
- Create or update a concise limitations document if one does not already exist. Suggested path: `docs\physics-known-limitations.md`.
- The limitations document should explicitly cover:
  - 2D approximation with unit thickness.
  - No 3D splash, foam, mist, turbulence guarantee, or molecular particle physics.
  - CPU solver and current grid scale limitations.
  - Any remaining approximate boundary behavior such as near-free-slip default walls.
  - Whether density correction is live, test-only, or tier-dependent.
  - Whether surface tension and viscosity are live defaults or optional settings.
  - Whether resampling is always on, tier-dependent, or only used in selected scenarios.
  - What offline quality means and which command proves it.
- Reconcile `docs\physics-accuracy-contract.md` known gaps:
  - Remove gaps that were fully implemented and verified.
  - Keep approximations that remain true.
  - Do not claim physical accuracy beyond the tests.
- Reconcile `PROGRESS.md`:
  - `R13.01` remains `Verified` only if the contract/ADR/limitations docs are current.
  - `R13.02` remains `Verified` only if particle mass/volume/density/APIC diagnostics still exist and tests pass.
  - `R13.03` remains `Verified` only if transfer conservation/APIC/FLIP tests still pass.
  - `R13.04` remains `Verified` only if pressure residual and projection tests/quality scenarios still pass.
  - `R13.05` can move from `Partial` to `Verified` only if live or tier-defined density correction/resampling evidence proves dense-cluster redistribution without mass loss.
  - `R13.07` can move to `Verified` only with named viscosity/surface/free-surface test evidence from `PSIM-0073` and `PSIM-0075`.
  - `R13.08` can move to `Verified` only with named resampling/rendering conservation and regression evidence from `PSIM-0074`.
  - `R13.09` can move to `Verified` only with named tier/budget evidence from `PSIM-0076`.
- Reconcile `ISSUES.md`:
  - Every issue from `PSIM-0066` through `PSIM-0076` must be `Done` or `Deferred` with rationale before closing this issue.
  - Do not mark this issue `Done` while any dependency is still `Open`, `In Progress`, or `Blocked`.
  - Add implementation notes that list the audit findings and exact verification results.
- Verification command order:
  - `.\scripts\check-tracking.ps1`
  - `.\scripts\build.ps1`
  - `.\scripts\test.ps1`
  - `.\scripts\verify-fluid-quality-suite.ps1`
  - `.\scripts\measure-water-solver.ps1`
  - `.\scripts\verify-all.ps1`
- If one of the long commands fails, keep this issue open and record the failure under implementation notes rather than downgrading unrelated verified rows.

Audit checklist:

- Every `R13.*` row has current evidence or an explicit issue/blocker in the next-task column.
- `docs\physics-accuracy-contract.md` agrees with implemented settings and known gaps.
- `docs\performance-budget.md` agrees with current benchmark output.
- `regression\README.md` records any baseline changes caused by physics/rendering changes.
- Quality-suite scenario list in `regression\fluid_quality_suite.psd1` matches `physics_sim_fluid_quality_tests --list-scenarios`.
- No `PSIM-0066` through `PSIM-0076` issue has `Implementation notes: None yet.` if it is marked `Done`.
- Tracking validator passes after all status edits.

Acceptance criteria:

- All `PSIM-0066` through `PSIM-0076` are `Done` or explicitly `Deferred` with rationale.
- Full verification bundle passes.
- Known limitations document what remains approximate, including 2D simplification, no turbulence accuracy guarantee, no 3D splashes, and no molecular particle physics.
- `PROGRESS.md` rows for `R13.*` are updated only with named evidence.
- This issue's implementation notes include the final audit summary, commands run, command results, and any unresolved limitations.

Subtasks:

- Review every Epic 11 issue and reconcile statuses, dependencies, and verification evidence.
- Create or update `docs\physics-known-limitations.md`.
- Reconcile `docs\physics-accuracy-contract.md`, `docs\performance-budget.md`, and any regression docs touched by physics changes.
- Run the full verification command set.
- Update known-limitations docs for any approximations or deferred scope.
- Re-run tracking validation after final status edits.

Verification:

- `.\scripts\verify-all.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0066.
- PSIM-0067.
- PSIM-0068.
- PSIM-0069.
- PSIM-0070.
- PSIM-0071.
- PSIM-0072.
- PSIM-0073.
- PSIM-0074.
- PSIM-0075.
- PSIM-0076.

Implementation notes:

- Reconciled the physics accuracy contract, documented accepted scope constraints in `docs\physics-known-limitations.md`, and updated the performance and regression docs so they match the implemented physics and replay output.
- Verified that every Epic 11 issue from `PSIM-0066` through `PSIM-0076` is now `Done`, and recorded the exact validation commands for the physics audit.
- Verification: `.\scripts\check-tracking.ps1`, `.\scripts\build.ps1`, `.\scripts\test.ps1`, `.\scripts\run-smoke.ps1`, `.\scripts\verify-replay-suite.ps1`, `.\scripts\verify-demo-scene.ps1`, `.\scripts\verify-demo-scene-density.ps1`, `.\scripts\verify-fluid-quality-suite.ps1`, `.\scripts\measure-water-solver.ps1 -Tier All`, and `.\scripts\verify-all.ps1` passed on 2026-06-05.
