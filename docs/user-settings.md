# Recovery User Settings

Recovered settings intentionally reset to schema 1 with header:

```text
physics-sim-recovery-settings 1
```

Pre-recovery `physics-sim-settings` files and other versions are rejected and the application falls back to defaults. The recovered schema persists only currently exposed preferences: window size, help visibility, visual mode, solver profile, reduced motion, fullscreen, high contrast, audio mute/levels, and validated input bindings.

The default location remains under `SDL_GetPrefPath("Nic", "physics-sim")`; `--settings-file <path>` selects an isolated file for tests and captures. Invalid settings never partially apply: parsing succeeds as one complete document or defaults are used.
