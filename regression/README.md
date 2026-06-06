# Regression Baseline

`demo_scene_golden.bmp` is the committed clean-frame baseline used by `scripts/verify-demo-scene.ps1`.

`demo_scene_density_golden.bmp` is the committed later-stage density-view baseline used by `scripts/verify-demo-scene-density.ps1`.

`demo_scene_surface_golden.bmp` is the committed later-stage surface-view baseline used by `scripts/verify-demo-scene-surface.ps1`.

To regenerate the clean-frame baseline, rebuild the app and capture the demo scene with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --dump-frame build\windows-x64\probe_tick240_ok.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000
```

To regenerate the later density baseline, rebuild the app and capture the demo scene with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --visual-mode density --dump-frame build\windows-x64\probe_density_960.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000
```

To regenerate the later surface baseline, rebuild the app and capture the demo scene with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --visual-mode surface --dump-frame build\windows-x64\probe_surface_2400.bmp --dump-frame-after-ticks 2400 --auto-exit-ms 60000
```

To regenerate the replay baselines, rebuild the app and capture each manifest replay with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --replay-file regression\replays\demo-add-directional.replay --dump-frame regression\demo_scene_replay_add_directional_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
.\build\windows-x64\Debug\physics-sim.exe --replay-file regression\replays\demo-add-omni.replay --dump-frame regression\demo_scene_replay_add_omni_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
```

The tick-based capture mode suppresses the HUD so the frame stays stable across runs.
Compare the new frame to the committed baseline before replacing it.

PSIM-0085 added a dedicated surface-view regression baseline after making `surface` the default visual mode and adding volume-fraction surface rendering. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0085-surface-baseline-app.log --settings-file build\windows-x64\psim0085-surface-baseline-settings.txt --skip-session-shell --visual-mode surface --dump-frame regression\demo_scene_surface_golden.bmp --dump-frame-after-ticks 2400 --auto-exit-ms 60000
.\scripts\verify-demo-scene-surface.ps1
```

PSIM-0085 also refreshed the directional replay baseline because replay captures use the default visual mode and now render through the surface view. Commands used:

```powershell
.\scripts\verify-replay-suite.ps1
Copy-Item -LiteralPath build\windows-x64\replay-suite\demo-add-directional-capture.bmp -Destination regression\demo_scene_replay_add_directional_golden.bmp -Force
.\scripts\verify-replay-suite.ps1
```

PSIM-0070 regenerated `demo_scene_golden.bmp` and `demo_scene_density_golden.bmp` after replacing the pressure iteration with matrix-free residual-reporting projection. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0070-demo-baseline-app.log --settings-file build\windows-x64\psim0070-demo-baseline-settings.txt --dump-frame regression\demo_scene_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0070-density-baseline-app.log --settings-file build\windows-x64\psim0070-density-baseline-settings.txt --visual-mode density --dump-frame regression\demo_scene_density_golden.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000
```

PSIM-0072 regenerated replay baselines against the PSIM-0070 pressure solver while verifying boundary/device replay determinism. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0072-replay-directional-app.log --settings-file build\windows-x64\psim0072-replay-directional-settings.txt --replay-file regression\replays\demo-add-directional.replay --dump-frame regression\demo_scene_replay_add_directional_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0072-replay-omni-app.log --settings-file build\windows-x64\psim0072-replay-omni-settings.txt --replay-file regression\replays\demo-add-omni.replay --dump-frame regression\demo_scene_replay_add_omni_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
```

PSIM-0074 regenerated the clean-frame and density baselines after switching mixed/density rendering to volume-fraction-backed alpha and after the resampling threshold update. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\demo_scene_manual.log --settings-file build\windows-x64\demo_scene_manual-settings.txt --dump-frame build\windows-x64\demo_scene_manual.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\demo_scene_density_manual.log --settings-file build\windows-x64\demo_scene_density_manual-settings.txt --visual-mode density --dump-frame build\windows-x64\demo_scene_density_manual.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000
```

Those captures were compared with `.\scripts\verify-demo-scene.ps1` and `.\scripts\verify-demo-scene-density.ps1` after the baselines were replaced.

PSIM-0074 also regenerated the replay baselines after the same physics and rendering updates changed the replay frames. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\demo_add_directional_manual.log --settings-file build\windows-x64\demo_add_directional_manual-settings.txt --replay-file regression\replays\demo-add-directional.replay --dump-frame build\windows-x64\demo_add_directional_manual.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\demo_add_omni_manual.log --settings-file build\windows-x64\demo_add_omni_manual-settings.txt --replay-file regression\replays\demo-add-omni.replay --dump-frame build\windows-x64\demo_add_omni_manual.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
```

Those captures were compared with `.\scripts\verify-replay-suite.ps1` after the baselines were replaced.

PSIM-0059 refreshed the replay baselines again after the accessibility and palette wiring slightly adjusted the replay-frame colors. The directional case came from `.\scripts\verify-replay-suite.ps1`, and the omni case was captured with:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\probe_omni_replay.log --settings-file build\windows-x64\probe_omni_replay-settings.txt --skip-session-shell --replay-file regression\replays\demo-add-omni.replay --dump-frame build\windows-x64\probe_omni_replay.bmp --dump-frame-after-ticks 240 --auto-exit-ms 30000
```

Both replay baselines were then compared with `.\scripts\verify-replay-suite.ps1` after replacement.

PSIM-0059 refreshed the clean-frame and density baselines again after the high-contrast and palette wiring slightly adjusted the rendered colors in the demo scene. Commands used:

```powershell
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0059-demo-baseline-app.log --settings-file build\windows-x64\psim0059-demo-baseline-settings.txt --skip-session-shell --dump-frame regression\demo_scene_golden.bmp --dump-frame-after-ticks 240 --auto-exit-ms 10000
.\build\windows-x64\Debug\physics-sim.exe --log-file build\windows-x64\psim0059-density-baseline-app.log --settings-file build\windows-x64\psim0059-density-baseline-settings.txt --skip-session-shell --visual-mode density --dump-frame regression\demo_scene_density_golden.bmp --dump-frame-after-ticks 960 --auto-exit-ms 30000
```

The refreshed baselines are verified with `.\scripts\verify-demo-scene.ps1` and `.\scripts\verify-demo-scene-density.ps1`.
