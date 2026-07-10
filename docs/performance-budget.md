# Solver Performance Budget

The current repeatable performance check is the Release build of `physics_sim_solver_benchmark`. `scripts/measure-water-solver.ps1` builds that target before measuring; pass `-Configuration Debug` only for diagnostic comparison.

Benchmark scenes:

- `small-container-stress`: `12 x 12`, cell size `1.0`, bottom and side walls, directional hose at `(6, 2)`, speed `6`, emission rate `12`, and `6000` fixed simulation steps at `1 / 120` seconds per step.
- `demo-grid-flow`: `80 x 45`, cell size `16.0`, the default U-container walls, directional hose at `(640, 160)`, speed `8`, emission rate `120`, and `240` fixed simulation steps at `1 / 120` seconds per step.

Budgets:

- `fast`: `small-container-stress` `20.0s` or less, `demo-grid-flow` `3.0s` or less.
- `balanced`: `small-container-stress` `24.0s` or less, `demo-grid-flow` `4.0s` or less.
- `quality`: `small-container-stress` `30.0s` or less, `demo-grid-flow` `6.0s` or less.
- The balanced Release `demo-grid-flow` average step must remain at or below `8.3334ms`, preserving the `1 / 120` simulation-step budget.

Command:

```powershell
.\scripts\measure-water-solver.ps1 -Profile All
```

Most recent Release result on this machine, 2026-07-10:

| Scene | Profile | Pressure residual | Density avg/max | Elapsed | Average step |
| --- | --- | ---: | ---: | ---: | ---: |
| small-container | fast | 0.000084 | 462.232 / 502.099 | 0.57s | 0.0945ms |
| demo-grid | fast | 0.000093 | 18.901 / 36.431 | 0.05s | 0.1885ms |
| small-container | balanced | 0.000029 | 0.037 / 0.510 | 2.61s | 0.4356ms |
| demo-grid | balanced | 0.000039 | 0.507 / 1.112 | 0.19s | 0.7855ms |
| small-container | quality | 0.000007 | 0.038 / 0.364 | 2.65s | 0.4416ms |
| demo-grid | quality | 0.000009 | 0.500 / 1.234 | 0.19s | 0.7933ms |
