# Water Feel Baseline - 2026-07-11

## Scope

This baseline is the pre-tuning evidence for PSIM-0102. It measures motion qualities that the recovery suite did not previously cover. The existing mass, pressure, penetration, finiteness, lifecycle, and determinism gates remain authoritative and unchanged.

## Environment And Commands

- Repository branch: `main`.
- Build: Debug, MSVC 19.44, Windows SDK 10.0.26100.0, SDL2 2.32.10.
- Fixed simulation step: `1/120` second.
- Command: `.\scripts\build.ps1`; passed on 2026-07-11 in 12.7 seconds as part of the 36.5-second build-and-baseline invocation.
- Command: `.\scripts\test.ps1 -Tier Fast`; passed 26/26 on 2026-07-11 in 4.277 seconds.
- Command: `.\scripts\test.ps1 -Tier Standard`; passed 28/28 on 2026-07-11 in 26.450 seconds.
- Command: `physics_sim_fluid_quality_tests.exe --profile balanced --scenario asymmetric-leveling --scenario steady-pour-feel --scenario slosh-decay --scenario wall-sheet-flow --scenario two-stream-merge --scenario obstacle-breakup-rejoin`; passed its safety gates on 2026-07-11. The measured scenario portion took 18.1 seconds after the build.
- Command: the same six-scenario matrix with `--profile quality`; passed its safety gates on 2026-07-11. The combined balanced and quality matrix took 52 seconds.
- Command: `.\scripts\verify-fluid-quality-suite.ps1`; passed all 30 manifest runs on 2026-07-11 in 278.7 seconds. Structured results are generated at `build/windows-x64/fluid-quality-suite/summary.json`, with per-scenario checkpoint logs beside it.

Each scenario executes twice and compares every sampled feel metric for deterministic equality. The fluid-quality suite logs retain one `sample=` record per checkpoint and a final `scenario=` record with scenario, profile, tick, actual metrics, hard-gate results, and artifact location.

## Final Baseline Measurements

| Scenario | Profile | Footprint cells | RMS slope | Max slope | Sampling CV | Components | Largest fraction | Vorticity RMS | Kinetic energy |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Asymmetric leveling | Balanced | 15.686 | 0.365 | 0.908 | 0.385 | 1 | 1.000 | 0.081 | 0.406 |
| Asymmetric leveling | Quality | 15.674 | 0.365 | 0.907 | 0.404 | 1 | 1.000 | 0.082 | 0.411 |
| Steady pour | Balanced | 18.150 | 0.428 | 1.255 | 0.482 | 1 | 1.000 | 0.174 | 2.531 |
| Steady pour | Quality | 16.922 | 0.654 | 1.776 | 0.489 | 1 | 1.000 | 0.136 | 1.429 |
| Slosh decay | Balanced | 16.363 | 0.066 | 0.132 | 0.362 | 1 | 1.000 | 0.001 | 0.027 |
| Slosh decay | Quality | 16.363 | 0.066 | 0.132 | 0.362 | 1 | 1.000 | 0.001 | 0.027 |
| Wall sheet | Balanced | 16.825 | 0.785 | 2.390 | 0.441 | 1 | 1.000 | 0.604 | 61.738 |
| Wall sheet | Quality | 13.856 | 1.092 | 2.711 | 0.415 | 1 | 1.000 | 0.468 | 54.509 |
| Two-stream merge | Balanced | 29.173 | 0.564 | 1.285 | 0.476 | 1 | 1.000 | 0.250 | 12.065 |
| Two-stream merge | Quality | 29.194 | 0.800 | 2.870 | 0.475 | 1 | 1.000 | 0.261 | 14.272 |
| Obstacle breakup/rejoin | Balanced | 21.938 | 2.755 | 10.185 | 0.531 | 2 | 0.765 | 0.604 | 51.025 |
| Obstacle breakup/rejoin | Quality | 21.571 | 3.043 | 10.214 | 0.515 | 2 | 0.787 | 0.526 | 84.536 |

All rows retained zero solid penetration, zero unexplained lifecycle changes, zero non-finite values, and reported mass error `0.000000`. Balanced pressure residuals remained at or below `4.4e-5`; quality remained at or below `8e-6`.

## Time-Series Findings

- Asymmetric leveling spreads from 5.3 to 15.7 cells, but after ten simulated seconds the surface still has RMS slope `0.365` and maximum adjacent-column step `0.908`. It remains visibly mound-like rather than level.
- The balanced pour reaches RMS slope `0.428` after three settling seconds. Quality is worse at `0.654` and has a smaller footprint, showing that stricter pressure convergence alone does not improve the perceived water motion.
- Slosh kinetic energy falls from `288.0` initially to `1.78` after one second and `0.061` after four seconds. This is severe transfer damping, not plausible water-scale oscillation decay.
- The two streams remain separate through tick 240 and merge by tick 480, but their final sampling CV remains `0.476`; the connected result is internally uneven.
- Obstacle flow splits into three support components at tick 480 and still has two at tick 840. Only 76-79% of particles belong to the largest component, so the current flow does not reliably rejoin into one body.
- Across the feel matrix, occupied-cell particle-count CV is `0.36-0.53`. This quantifies the sparse and hole-prone particle distribution visible in the density captures.

## Provisional Calibration Ranges

These are candidate acceptance ranges for implementation comparisons, not permission to relax hard physics gates. They may be promoted only after reviewed motion evidence shows that they correspond to convincing water.

- Settled asymmetric pool: RMS surface slope `<= 0.15`, maximum slope `<= 0.40` away from walls, and at least 17 occupied columns for the standard fixture.
- Settled standard pour: RMS surface slope `<= 0.25`, maximum slope `<= 0.75`, footprint at least 19 cells, and sampling CV `<= 0.35`.
- Slosh: retain at least 5% of the initial kinetic energy after one second, exhibit at least two center-of-mass direction changes, and decay below 10% by eight seconds without monotonic overdamping.
- Two-stream merge: transition from two components to one by the impact checkpoint and remain at least 98% in the largest component after emission stops.
- Obstacle breakup/rejoin: split during obstacle contact, return to one component or at least 95% in the largest component after pooling, and avoid unsupported isolated droplets.
- Sampling quality: occupied-cell particle-count CV `<= 0.35` in settled pools without mass, momentum, or center-of-mass drift.

## Classification

- Numerical debt: transfer damping and partial-cell/free-surface treatment prevent plausible sloshing and leveling.
- Sampling debt: local density correction and per-cell resampling leave high particle-count variation and persistent fragments.
- Rendering debt: the current quarter-cell volume raster and thresholded vertex averaging can hide holes and enlarge tiny volumes.
- Test-quality debt addressed by PSIM-0102: the suite now records motion-specific signals and deterministic checkpoint sequences.
- Human acceptance remains missing: no still-image hash or numeric range in this report establishes convincing real-time water by itself.
