# Roadmap

This file is the source of truth for project goals and completion criteria.

## Completion Rules

- A stage is complete only when every marker in that stage is true.
- "Testable" means the result can be reproduced from a documented command or demo scene on this machine.
- "Measurable" means the stage has a test, a numeric threshold, a saved scene, or a repeatable manual demo.
- Future stretch ideas do not count toward completion unless they are listed here.

## 1. Bootstrap And Windowing

Goal: a clean Windows C++ app shell that builds and runs reliably.

Completion markers:

- `scripts/build.ps1` succeeds on a clean checkout.
- `scripts/test.ps1` succeeds on a clean checkout.
- `scripts/run-smoke.ps1` launches the app, opens a window, and exits with code `0`.
- The app handles window resize, mouse movement, keyboard input, and close events without crashing.
- The build is reproducible from the repository docs and wrappers, without copying DLLs by hand.
- No repo-local step depends on a machine-specific absolute path outside the documented helper scripts.
- The README or equivalent docs show the exact commands needed to configure, build, test, and run.

Stage exit demo:

- Launch the app.
- Confirm the window opens.
- Move the mouse and see the cursor response.
- Resize the window and confirm rendering still works.
- Close the window cleanly.

## 2. Fixed Timestep Core

Goal: the simulation runs on a deterministic update cadence independent of render rate.

Completion markers:

- The main loop uses a fixed simulation step rather than advancing physics directly by render delta time.
- Time accumulation, stepping, and clamping are handled in one place, not scattered through rendering code.
- Pause, single-step, and reset are available from input.
- Input is translated into actions/commands before it reaches simulation code.
- Reset returns the simulation to its initial state without restarting the executable.
- Core helpers for math, indexing, and time accumulation have unit tests.
- A paused simulation can still render and accept UI input correctly.

Suggested measurable checks:

- Run the app with a forced variable render delay and confirm the simulation advance remains stable.
- Step the simulation one frame at a time and confirm the state advances exactly one fixed tick per step.

## 3. 2D Water Solver

Goal: water exists as an actual simulated fluid, not just animated particles.

Completion markers:

- Water is represented by a real fluid simulation state.
- The solver uses a 2D hybrid particle-grid approach suitable for water behavior.
- Gravity, advection, pressure projection, and solid boundary handling are implemented.
- Particle-to-grid and grid-to-particle transfers are implemented and tested.
- A configurable water fixture can emit fluid with position, direction, emission rate, and initial speed.
- Water collides with walls and accumulates in containers instead of passing through them.
- The solver produces only finite values during long runs.

Suggested measurable checks:

- In a closed container test, average post-projection divergence stays below the agreed threshold.
- In a fixed-duration fill test, total fluid mass stays within the agreed tolerance after accounting for intentional inflow and outflow.
- In a 10-minute stress scene, the solver does not produce NaNs, infinities, or unstable explosions.

Acceptance scenes:

- Falling stream into open space.
- Water filling a U-shaped container.
- Directional hose stream impacting a wall and pooling.
- Continuous emitter run with no obvious numerical breakdown.

## 4. Walls And Fixtures

Goal: the user can shape the fluid environment interactively.

Completion markers:

- The user can draw walls with the mouse.
- The user can erase or modify walls without restarting the app.
- Wall edits affect the simulation immediately or on the next fixed step.
- Wall geometry is stored as scene data, not only as transient runtime state.
- Fixtures are data-driven objects rather than one-off hard-coded behaviors.
- A directional hose fixture exists and emits water in a configurable direction.
- A 360-degree emitter exists and sprays water radially.
- A U-shaped wall with a hose above it retains water in the container instead of leaking through.

Suggested measurable checks:

- Draw a box or U-shape wall and verify particles cannot escape through the solid boundary.
- Place a hose, rotate it, and verify the water stream follows the chosen direction.
- Place an omni emitter and verify the emission pattern is radial rather than directional.

## 5. Visualization And Editing

Goal: the simulator is easy to understand, inspect, and modify while it is running.

Completion markers:

- Water and wall geometry are visually clear in motion.
- At least one useful debug visualization mode exists, such as particle view, density view, or surface view.
- Overlay diagnostics show FPS, fixed timestep info, particle count, and at least one solver health metric.
- The user can zoom and pan the view.
- The user can select, place, and remove fixtures from the scene.
- The user can reset the scene without relaunching the app.
- Scene save and load work for walls and fixtures.
- Demo scenes can be loaded consistently and reproduce the same starting state.
- Regression capture exists for at least one golden scene, such as a screenshot comparison or scripted replay.

Suggested measurable checks:

- Load the same scene file twice and confirm the same initial objects appear.
- Capture a known demo scene and compare it against a stored baseline.
- Toggle between visual modes and confirm the simulation remains unchanged.

## 6. Complete Testable Product

Goal: a user can run a full interactive water simulation loop from start to finish.

Completion markers:

- A fresh checkout can be configured, built, tested, and run from the documented commands on this machine.
- The app can create or load a scene, emit water, interact with walls, and show the expected contained-water behavior.
- Automated tests pass.
- Smoke tests pass.
- A repeatable end-to-end demo scene exists and can be used as a regression check.
- The core workflow is complete: launch app, draw or load walls, place a fixture, observe water behavior, reset, repeat.
- There are no known blockers preventing the v1 experience from being used as a real interactive testbed.

Completion criteria:

- If any stage above is incomplete, the product is not complete.
- Future expansion ideas are allowed, but they are not required for the first complete testable product.

## Future Sandbox Vision

The stages below describe the longer-term sandbox game direction. They are product goals, not requirements for the first complete testable product unless explicitly promoted into stages 1-6.

## 7. Editor UX And Fixture Authoring

Goal: editing scenes feels intentional, inspectable, and reversible instead of relying on one-shot placement.

Completion markers:

- Users can select and delete individual fixtures without clearing the scene.
- A selected fixture inspector shows kind, position, direction, speed, emission rate, and enabled state.
- Users can move and rotate selected fixtures while seeing immediate visual feedback.
- Undo and redo work for wall painting, wall erasing, fixture placement, fixture deletion, and fixture edits.
- An in-app help or controls reference is available without leaving the simulation.
- The overlay or equivalent UI shows active tool, selected fixture state, and editable parameter values.
- Editor workflows have repeatable automated or manual verification evidence.

## 8. Solver Correctness, Performance, And Determinism

Goal: the water solver is trustworthy enough for sandbox play, regression testing, and future gameplay systems.

Completion markers:

- Particle-to-grid and grid-to-particle transfer behavior has focused tests beyond indirect integration coverage.
- Leak and retention acceptance thresholds are documented and tested for closed and U-shaped containers.
- Inflow, outflow, particle count, or mass behavior is measured against explicit tolerances.
- Particle lifecycle and out-of-domain cleanup are defined, tested, and visible in metrics.
- Simulation replay and frame capture can be made deterministic for regression scenarios.
- Solver performance has a documented budget and a repeatable profiling or timing check.
- Long-run stress verification has a named command, duration, and stability threshold.

## 9. Sandbox Devices And Interactive Systems

Goal: the sandbox offers interactive water devices beyond static walls and emitters.

Completion markers:

- Scene data supports device types beyond walls and water emitters.
- Drains or sinks remove water in a controlled, inspectable way.
- Pumps and valves can route or regulate water flow.
- Gates or doors can be opened and closed by user input.
- Sensors or triggers can react to water or device state.
- A device palette or equivalent tool workflow lets users choose and place devices predictably.
- Devices persist through scene save/load and have regression or acceptance coverage.

## 10. Scene Library, Sharing, And Regression Coverage

Goal: scenes are durable artifacts that can be organized, reused, shared, and tested.

Completion markers:

- The scene format is versioned with a documented compatibility policy.
- Scenes can store metadata such as title, description, author, tags, and notes.
- Scenes can store or generate thumbnails for browsing and regression evidence.
- Users can browse and load multiple scenes without manually editing file paths.
- A scripted replay regression suite can exercise more than one scene or interaction path.
- A demo-scene gallery covers the core sandbox behaviors.
- Scene import, invalid-scene handling, and save/load failures are tested or manually documented.

## 11. Sandbox Game Loop And Player Experience

Goal: the project feels like a playable sandbox game rather than only a technical simulator.

Completion markers:

- Creative mode has a coherent start, edit, simulate, reset, and continue flow.
- Optional challenge or objective mode can define goals without replacing creative play.
- Session controls support reset, retry, pause, resume, and clear-scene workflows cleanly.
- Water visualization moves beyond square particles where practical while preserving debug clarity.
- User feedback such as sound, animation, or visual affordances reinforces important actions.
- The app communicates scene state, tool state, and simulation state without relying only on the window title.
- The default demo experience teaches the core sandbox loop through interaction rather than external explanation.

## 12. Packaging, Diagnostics, And Project Automation

Goal: the project can be built, verified, diagnosed, and handed off reliably.

Completion markers:

- A release package can be produced from a documented command.
- App logging or crash diagnostics capture enough context to debug runtime failures.
- User settings can persist preferences such as window size, overlay mode, and visual mode.
- A one-command local verification bundle runs tracking, build, tests, smoke, and relevant regression checks.
- The tracking validator checks epic structure, duplicate issue headings, invalid dependencies, and missing roadmap links.
- Architecture decision records or equivalent notes exist for major technical choices.
- Backlog pruning, reprioritization, and implementation handoff conventions are documented.

## 13. Physics Accuracy And Particle Interaction

Goal: water particles interact as deterministic 2D incompressible fluid parcels with physical mass, volume, density, pressure, viscosity, surface behavior, and boundary constraints.

Completion markers:

- Fluid physics model, units, invariants, and governing equations are documented.
- Particles carry physical mass, volume, density, and interaction diagnostics.
- Particle-grid transfer conserves mass and momentum and supports APIC/FLIP behavior.
- Pressure projection enforces incompressibility with measured residuals.
- Dense particle clusters redistribute through density constraints without mass loss.
- World boundaries implement defined free-slip or no-slip material interaction.
- Viscosity, surface tension, and free-surface behavior are modeled and tested.
- Particle resampling and rendering preserve physical volume.
- Live and offline physics-quality budgets pass deterministically.
