# User Settings

Physics Sim stores user preferences in a plain-text settings file.

Default location:

- `SDL_GetPrefPath("Nic", "physics-sim")/physics-sim-settings.txt`

Override for testing or local workflows:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --settings-file build\windows-x64\physics-sim-settings.txt
```

You can seed settings for a local verification run with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --settings-file build\windows-x64\physics-sim-settings.txt --window-size 1024x768 --show-help --visual-mode density --solver-profile quality --reduced-motion
```

Saved settings:

- window width and height
- fullscreen state
- help overlay visibility
- visual mode
- solver profile
- reduced motion
- high contrast
- audio mute state
- master, effects, and music volumes
- remappable keyboard bindings

File format:

```text
physics-sim-settings 3
window 1280 720
help_overlay 0
visual_mode surface
solver_profile balanced
reduced_motion 0
fullscreen 0
high_contrast 0
audio_muted 0
audio_master 100
audio_effects 100
audio_music 25
bind pause_resume space
bind step_once s
bind reset f10
```

The file continues with `bind` lines for every remappable action, including tool cycling, editing shortcuts, save/load, and fullscreen toggles.

The file format accepts version `1`, `2`, or `3`. Version `1` files load with defaults for the newer audio, fullscreen, high-contrast, and binding fields. Version `2` files load with the default `balanced` solver profile. The app writes version `3` on exit.

Supported visual modes are `surface`, `mixed`, `density`, and `particles`. `surface` is the default player-facing mode; the other modes remain available for diagnostics.

Supported solver profiles are `fast`, `balanced`, and `quality`. `balanced` is the default runtime profile. `fast` preserves the legacy live benchmark behavior, and `quality` enables the stricter density correction, resampling, and APIC settings used for offline-style checks.

If the file is missing or invalid, the app falls back to default values and writes a new settings file on exit.
