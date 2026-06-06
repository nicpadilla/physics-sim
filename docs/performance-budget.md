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

- `small-container-stress` live: `end_particles=600`, `active_cells=44`, `visible_cells=19`, `pressure_active_cells=44`, `active_cell_overreach=2.315789`, `pressure_iterations=17`, `pressure_relative_residual=0.000084`, `pressure_converged=true`, `average_density_error=462.232177`, `max_density_error=502.098719`, `mass_error=0.000000`, `elapsed_seconds=2.89`, `average_step_ms=0.4823`
- `small-container-stress` offline: `end_particles=160`, `active_cells=110`, `visible_cells=69`, `pressure_active_cells=110`, `active_cell_overreach=1.594203`, `pressure_iterations=41`, `pressure_relative_residual=0.000009`, `pressure_converged=true`, `average_density_error=0.334579`, `max_density_error=0.870596`, `mass_error=0.000000`, `elapsed_seconds=15.67`, `average_step_ms=2.6114`
- `demo-grid-flow` live: `end_particles=240`, `active_cells=64`, `visible_cells=30`, `pressure_active_cells=64`, `active_cell_overreach=2.133333`, `pressure_iterations=17`, `pressure_relative_residual=0.000093`, `pressure_converged=true`, `average_density_error=18.901172`, `max_density_error=36.431042`, `mass_error=0.000000`, `elapsed_seconds=0.22`, `average_step_ms=0.9340`
- `demo-grid-flow` offline: `end_particles=477`, `active_cells=965`, `visible_cells=347`, `pressure_active_cells=965`, `active_cell_overreach=2.780980`, `pressure_iterations=35`, `pressure_relative_residual=0.000009`, `pressure_converged=true`, `average_density_error=0.758781`, `max_density_error=0.937365`, `mass_error=0.000000`, `elapsed_seconds=3.19`, `average_step_ms=13.2760`
