# Solver Performance Budget

The current repeatable performance check is `physics_sim_solver_benchmark`.

Benchmark scenes:

- `small-container-stress`: `12 x 12`, cell size `1.0`, bottom and side walls, directional hose at `(6, 2)`, speed `6`, emission rate `12`, and `6000` fixed simulation steps at `1 / 120` seconds per step.
- `demo-grid-flow`: `80 x 45`, cell size `16.0`, the default U-container walls, directional hose at `(640, 160)`, speed `8`, emission rate `120`, and `240` fixed simulation steps at `1 / 120` seconds per step.

Budgets:

- `small-container-stress` must complete in `20.0` seconds or less on this machine.
- `demo-grid-flow` must complete in `3.0` seconds or less on this machine.

Command:

```powershell
.\scripts\measure-water-solver.ps1 -Tier All
```

Most recent observed result on this machine:

- `small-container-stress` live: `end_particles=600`, `active_cells=116`, `pressure_iterations=27`, `pressure_relative_residual=0.000048`, `pressure_converged=true`, `elapsed_seconds=6.61`, `average_step_ms=1.1024`
- `small-container-stress` offline: `end_particles=600`, `active_cells=116`, `pressure_iterations=30`, `pressure_relative_residual=0.000006`, `pressure_converged=true`, `elapsed_seconds=6.82`, `average_step_ms=1.1363`
- `demo-grid-flow` live: `end_particles=240`, `active_cells=338`, `pressure_iterations=41`, `pressure_relative_residual=0.000092`, `pressure_converged=true`, `elapsed_seconds=0.56`, `average_step_ms=2.3469`
- `demo-grid-flow` offline: `end_particles=240`, `active_cells=338`, `pressure_iterations=49`, `pressure_relative_residual=0.000007`, `pressure_converged=true`, `elapsed_seconds=0.62`, `average_step_ms=2.5634`
