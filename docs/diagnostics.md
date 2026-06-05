# Diagnostics

Physics Sim writes a plain-text log file for startup, scene load/save, replay parsing, renderer setup, audio fallback, settings persistence, and fatal SDL errors.

Default log path:

- `physics-sim.log` in the current working directory

Override the path with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\physics-sim-diagnostics.log
```

Scene-load failure check:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\physics-sim-diagnostics.log --scene-path scenes\does-not-exist.pscene --auto-exit-ms 1500
```

Replay-load failure check:

```powershell
$invalidReplay = "build\windows-x64\physics-sim-invalid.replay"
Set-Content -LiteralPath $invalidReplay -Value "physics-sim-replay 1`ntick 0 nope"
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\physics-sim-diagnostics.log --replay-file $invalidReplay --auto-exit-ms 1500
```

The log records the failed scene or replay load, the fallback to the demo scene when one is available, the renderer setup, and the shutdown path. Player-facing messages use the same failure taxonomy for scene, replay, audio, settings, renderer, and package-content errors, while the log includes the detailed technical context.

User-facing feedback uses the on-screen debug overlay as well: save, load, reset, delete, and tool changes flash a short `MSG` line so important actions are visible without opening the log.
