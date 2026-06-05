# Finished Game Polish Roadmap

## Summary

This roadmap defines the future polish work needed for the simulator to feel like a finished sandbox game instead of a technical testbed. It does not implement those changes. It creates a measurable quality bar for menus, HUD clarity, onboarding, art direction, feedback, accessibility, persistence, packaging, and release-grade QA.

The goal is a player-facing product shell around the existing deterministic water sandbox. Debug tools remain available, but the default experience should communicate what to do, why it matters, and whether the game state is healthy without relying on source-code knowledge, logs, or the window title.

## Finished-Game Definition

A finished game version should:

- Launch into a coherent product shell with clear start, continue, scene browser, settings, and quit paths.
- Teach the core creative loop through interaction: choose a scene, edit, place devices, simulate, reset, retry, save, and continue.
- Make tools, scene state, simulation state, objectives, and errors visible in the app.
- Present water, walls, devices, selection, invalid placement, and objective feedback with deliberate visual language.
- Provide useful audio and animation feedback without making correctness depend on effects.
- Persist settings and player-facing save/load state safely.
- Package cleanly enough that a user can run it from a release folder without developer setup knowledge.
- Stay deterministic where simulation and regression evidence require determinism.
- Preserve responsive live play at the current `80 x 45`, `16px` grid scale.

## Core Principles

- Player-first, debug-second: debug overlays and metrics stay available, but they are not the main interface.
- Polish must expose simulation truth, not hide broken physics with visual tricks.
- Menus, HUD, scene browser, and settings must be testable as state transitions, not only manual visuals.
- Numeric and state tests are primary evidence; screenshots support presentation regressions.
- The live build should remain responsive at 60 FPS on the current demo-scale scene.
- C++20 and SDL remain the default runtime boundary unless a later ADR approves a new UI, audio, or rendering dependency.
- Accessibility, input clarity, and error recovery are part of finish quality, not late extras.

## Explicitly Out Of Scope For This Workstream

- Online multiplayer, cloud saves, account systems, workshop uploads, or network scene sharing.
- Storefront integration, achievements, monetization, DLC, or platform certification.
- A full campaign, character story, voice-over pipeline, or localization system.
- 3D rendering, GPU fluid solvers, external physics middleware, or a wholesale engine migration.
- Procedural content generation beyond curated scenes and challenge metadata.
- Final marketing art, trailers, or store-page assets.

## Product Shell Considerations

The app should have a clear shell around the sandbox:

- Main menu with sandbox, scene browser, settings, credits/about, and quit.
- Pause menu with resume, retry, reset fluid, clear scene, save, load, settings, and return-to-menu.
- Direct launch flags for smoke tests, replay tests, and automated captures.
- A first-run path that starts with a guided scene or tutorial rather than a blank debug canvas.
- A clear separation between creative mode, challenge/objective scenes, and diagnostic/regression scenes.

## HUD And Interaction Considerations

The in-game UI should communicate the current state without becoming a wall of debug text:

- Tool palette with readable icons or labels, selected tool, placement validity, and shortcuts.
- Selection inspector for fixtures and devices with clear editable fields.
- Scene title, objective state, pause/simulate state, and save/load status.
- Short action feedback for save, load, undo, redo, reset, invalid placement, objective completion, and errors.
- Debug panel that can be shown for FPS, particle count, solver metrics, replay state, and test diagnostics.

## Visual Polish Considerations

The visual direction should make the game readable and intentional:

- Consistent palette for water, solids, devices, sensors, selection, warnings, and objectives.
- Water rendering that reads as liquid while keeping particle, density, and debug views available.
- Device shapes that communicate behavior before the user reads a tooltip.
- Hover, placement preview, invalid placement, selected, disabled, active, and triggered states.
- Menu screens, panels, focus states, transitions, and empty states that look designed rather than default.
- Screenshot regression points for important screens and scenes once layouts stabilize.

The current visual-style note lives in [docs/visual-style.md](visual-style.md) and captures the palette and separation rules that the renderer already follows.

## Audio And Feedback Considerations

Audio should support comprehension and polish without becoming required for gameplay:

- UI sounds for selection, confirm, cancel, save, load, invalid action, and objective completion.
- Subtle water/device sounds where they reinforce state changes and do not mask performance issues.
- Master, effects, and music volume controls with mute.
- Graceful fallback if audio initialization fails or no audio device exists.
- Visual alternatives for important audio-only cues.

## Accessibility And Settings Considerations

Finished-game polish should include settings that make the sandbox usable:

- Window mode, resolution or scale behavior, overlay mode, visual mode, audio levels, and reduced motion.
- Rebindable keyboard shortcuts for core actions.
- Mouse sensitivity or pan/zoom tuning if camera feel becomes a blocker.
- High-contrast and colorblind-safe state colors for tools, warnings, and objective feedback.
- Help text generated from active input bindings so documentation does not drift.

## Save, Load, And Recovery Considerations

Player-facing persistence needs more than a raw scene file path:

- Named saves or a simple save browser for user-created scenes.
- Autosave behavior with clear timing, location, and recovery rules.
- Backups before overwriting or migrating scene data.
- Human-readable load errors for unsupported versions, malformed files, missing sidecars, and write failures.
- Logs that include enough context for debugging without requiring players to inspect them first.

## Testing And Evidence

Future implementation should add evidence before claiming polish work is complete:

- Unit tests for menu state, command routing, settings persistence, input remapping, and save/load error mapping.
- Scenario tests for tutorial progression, scene browser loading, objective display, and session reset/retry flows.
- Replay tests for player-facing interactions where deterministic input matters.
- Screenshot or BMP regressions for the main menu, pause menu, scene browser, HUD, and key polished scenes.
- Manual QA matrix for first launch, keyboard-only operation, resize/fullscreen, audio fallback, invalid saves, and packaged launch.
- Performance tests that report simulation, render, UI, and frame pacing costs for live and debug modes.

## Future Issue Map

- `PSIM-0051`: Finished-game product direction and UX quality bar.
- `PSIM-0052`: Main menu, pause menu, and session shell.
- `PSIM-0053`: HUD, tool palette, and in-game readability pass.
- `PSIM-0054`: First-run onboarding and interactive tutorial.
- `PSIM-0055`: Scenario browser, progression, and objective presentation.
- `PSIM-0056`: Visual identity, art direction, and rendering polish.
- `PSIM-0057`: Animation, camera, and moment-to-moment feedback.
- `PSIM-0058`: Audio mix, sound effects, and player feedback.
- `PSIM-0059`: Settings, accessibility, and input remapping.
- `PSIM-0060`: Save/load UX, autosave, and recovery flow.
- `PSIM-0061`: Player-facing error handling and diagnostics polish.
- `PSIM-0062`: Release package, first-launch, and install handoff.
- `PSIM-0063`: Finished-game performance and responsiveness budget.
- `PSIM-0064`: Final finished-game polish audit.

## Risks

- A polished shell can hide missing solver quality if numeric physics evidence is not kept primary.
- Menu and UI work can become brittle if it is not modeled as testable state.
- Visual effects can make deterministic regression captures harder if timing is not controlled.
- Audio and animation can add platform-specific failure modes.
- A broad polish pass can sprawl unless every task is tied to a named player-facing outcome and verification command.
- Tight screenshot baselines can become maintenance-heavy if layouts are still changing.
