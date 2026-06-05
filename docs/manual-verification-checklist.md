# Manual Verification Checklist

Use this checklist after building the app on this machine:

```powershell
.\scripts\build.ps1
```

## Tutorial And First Run

1. Launch the tutorial directly:

   ```powershell
   .\build\windows-x64\Debug\physics-sim.exe --tutorial-mode
   ```

2. Press `Esc` and choose `Return to Menu` from the pause menu.
   - Expected: the guided tutorial can be skipped without restarting the executable, and the main menu returns.
3. Press `R` before leaving the tutorial.
   - Expected: the current tutorial scene resets without restarting the executable.

## Window And Input

1. Launch the app:

   ```powershell
   .\build\windows-x64\Debug\physics-sim.exe
   ```

2. Move the mouse across the window.
   - Expected: the crosshair follows the cursor.
3. Resize the window larger and smaller.
   - Expected: rendering continues without a crash or blank frame.
4. Close the window with the title-bar close button or `Esc`.
   - Expected: the app exits cleanly with no crash dialog.

## Pause, Step, Reset

1. Press `Space`.
   - Expected: the title shows the paused state.
2. Press `S` once while paused.
   - Expected: the simulation advances by one fixed tick.
3. Press `R`.
   - Expected: the current scene resets without restarting the executable.
4. Press `Delete`.
   - Expected: the selected fixture is removed, or the scene clears if nothing is selected.

## Wall Drawing

1. Press `1` to select wall paint mode.
2. Left-drag a short line in the scene.
   - Expected: the wall appears immediately on the next fixed tick.
3. Press `2` to select wall erase mode.
4. Left-drag over part of the wall line.
   - Expected: only the touched wall cells are removed.

## Fixture Selection And Editing

1. Press `3` or `4` to select a hose or omni emitter.
2. Left-click an existing emitter.
   - Expected: the emitter is highlighted and the overlay shows its values.
3. Press `Q` and `E`.
   - Expected: the selected fixture or active tool direction rotates.
4. Press `[` and `]`.
   - Expected: the selected fixture speed changes.
5. Press `-` and `=`.
   - Expected: the selected fixture emission rate changes.
6. Press `T`.
   - Expected: the selected fixture toggles enabled/disabled.

## Visual And Help Overlays

1. Press `V`.
   - Expected: the visual mode cycles between mixed, density, and particles.
2. Press `H`.
   - Expected: the in-app help overlay appears and disappears.
3. Confirm the overlay shows FPS, fixed step, tick count, particle count, divergence, visual mode, active tool, and selected fixture state.
   - Expected: the text remains readable at the default window size.
4. Launch with `--reduced-motion`.
   - Expected: the status-message flash stays static instead of fading, while the app remains fully readable and responsive.

## Settings, Audio, And Save Browser

1. Open the pause menu and choose `Settings`.
2. Toggle fullscreen, high contrast, reduced motion, and audio mute.
   - Expected: the window mode changes immediately, the palette switches to the higher-contrast colors, and the settings persist after restart.
3. Adjust the master, effects, and music volume entries.
   - Expected: the values clamp between `0` and `100` and remain after restart.
4. Remap `Pause / Resume` to a different key, then use the new key.
   - Expected: the help overlay and tutorial text show the new binding, and the new key still toggles pause/resume.
5. Press `F5` to save the current scene, then open the pause menu and choose `Load Save`.
   - Expected: the browser shows the autosave plus named saves, and loading one restores that scene without restarting the executable.

## Failure Paths

1. Launch with a missing startup scene:

   ```powershell
   .\build\windows-x64\Debug\physics-sim.exe --scene-path scenes\does-not-exist.pscene --auto-exit-ms 1500
   ```

   - Expected: the log records the failure, the app falls back to the demo scene, and the player-facing message stays readable.
2. Create a malformed replay file and launch it:

   ```powershell
   $invalidReplay = "build\windows-x64\physics-sim-invalid.replay"
   Set-Content -LiteralPath $invalidReplay -Value "physics-sim-replay 1`ntick 0 nope"
   .\build\windows-x64\Debug\physics-sim.exe --replay-file $invalidReplay --auto-exit-ms 1500
   ```

   - Expected: the app reports a `REPLAY FAILED` message and exits cleanly without exposing raw parser text as the only explanation.
