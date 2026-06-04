# Regression Baseline

`demo_scene_golden.bmp` is the committed clean-frame golden used by `scripts/verify-demo-scene.ps1`.

To regenerate it, rebuild the app and capture the demo scene with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --dump-frame build\windows-x64\probe_tick240_ok.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000
```

The tick-based capture mode suppresses the HUD so the frame stays stable across runs.
Then compare the new frame to the committed baseline before replacing it.
