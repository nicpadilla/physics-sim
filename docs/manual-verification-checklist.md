# Recovery Sandbox Manual Verification

Run this matrix against the packaged Release application, not a developer-only Debug launch. Record the date, reviewer, commit/package hash, and result in a dated file under `docs/reviews`.

## First and returning launch

1. Launch `physics-sim.exe` from outside the package working directory.
2. Confirm the guided basin loads without a missing-content dialog or engineering overlay.
3. Pour with the pointer and follow the interactive draw, erase, pause, reset, and save/load prompts.
4. Close and relaunch; confirm persisted settings and the returning flow remain usable.

## Core loop

1. Pour water with the left pointer button.
2. Select draw wall (`1`) and erase wall (`2`); confirm edits appear on the fixed tick.
3. Pause/resume with `Space`, single-step with `S`, and clear fluid with `R`.
4. Undo/redo with `Ctrl+Z` and `Ctrl+Y`.
5. Save with `F5` and load with `F9`; confirm the scene is restored and save feedback is visible.
6. Open the menu with `Esc`, switch to Lab, return to the menu, and return to Sandbox.

## Keyboard and window behavior

1. Navigate every menu entry using arrows, `Enter`, and `Esc` without the pointer.
2. Complete the sandbox actions using the documented keyboard alternatives.
3. Resize smaller/larger, toggle fullscreen with `F11`, and restore windowed mode.
4. Remove and restore focus while running and while paused; confirm no stuck action or simulation-time jump.
5. Close through the title bar and through the menu; confirm clean exit.

## Accessibility and degraded devices

1. Enable high contrast and reduced motion and confirm both persist after restart.
2. Remap a core control, use it, restore defaults, and confirm the guidance label follows the mapping.
3. Navigate settings with the keyboard only.
4. Start with the audio device unavailable; confirm silent operation with readable feedback and no crash.

## Persistence failures

1. Launch with a missing or malformed save and confirm a player-facing recovery message plus a usable basin.
2. Simulate failure before atomic replacement and confirm the previous save remains valid.
3. Confirm a `.backup` file is retained after successful replacement.

## Soak and package

1. Run the packaged sandbox continuously for at least 15 minutes while pouring, editing, pausing, resizing/fullscreening, saving, and loading.
2. Confirm finite state, responsive input, readable water, no crash dialog, and clean exit.
3. Run packaged sandbox and lab smoke from a clean temporary directory.

Deferred emitters, devices, objectives, progression, and gallery navigation must remain unreachable. Dear ImGui must never appear in Sandbox.
