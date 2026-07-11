# Recovery Regression Assets

The active recovery suite is basin-focused and uses scene format 2 plus replay format 2.

Active accepted baselines:

- `recovery_basin_early_surface_golden.bmp`: tick 240 while the recovered basin is filling.
- `recovery_basin_density_golden.bmp`: tick 2400 density inspection after the emitter stops at tick 1200.
- `recovery_basin_surface_golden.bmp`: tick 2400 settled continuous-surface result.

All three use `scenes/starter_basin.pscene` with `replays/recovery-basin.replay`. The replay places a low-speed directional emitter at tick 0 and clears it at tick 1200, so the final captures exercise the required 10-second pour plus 10-second settle interval.

Verification commands:

```powershell
.\scripts\verify-recovery-basin.ps1
.\scripts\verify-recovery-basin-density.ps1
.\scripts\verify-recovery-basin-surface.ps1
.\scripts\verify-replay-suite.ps1
```

The baselines were accepted on 2026-07-10 after the complete numeric suite, canonical Lab semantic gates, exact-capture inspection, and owner-delegated visual review recorded under `docs/reviews/recovery-visual-acceptance-2026-07-10.md`.

Pre-recovery demo and deferred-emitter baselines are retained only as historical evidence under `legacy/pre-recovery-2026-07-10`. They are not release truth and no active verifier references them.

Failed active captures are preserved under `build/windows-x64` with structured result JSON. Successful captures are removed after their SHA-256 matches the accepted baseline.
