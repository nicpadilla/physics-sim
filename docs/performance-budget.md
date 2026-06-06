# Solver Performance Budget

The current repeatable performance check is `physics_sim_solver_benchmark`.

Benchmark scenes:

- `small-container-stress`: `12 x 12`, cell size `1.0`, bottom and side walls, directional hose at `(6, 2)`, speed `6`, emission rate `12`, and `6000` fixed simulation steps at `1 / 120` seconds per step.
- `demo-grid-flow`: `80 x 45`, cell size `16.0`, the default U-container walls, directional hose at `(640, 160)`, speed `8`, emission rate `120`, and `240` fixed simulation steps at `1 / 120` seconds per step.

Budgets:

- `fast`: `small-container-stress` `20.0s` or less, `demo-grid-flow` `3.0s` or less.
- `balanced`: `small-container-stress` `24.0s` or less, `demo-grid-flow` `4.0s` or less.
- `quality`: `small-container-stress` `30.0s` or less, `demo-grid-flow` `6.0s` or less.

Command:

```powershell
.\scripts\measure-water-solver.ps1 -Profile All
```

Most recent observed result on this machine:

- `small-container-stress` `fast` / `live`: `end_particles=600`, `active_cells=44`, `visible_cells=19`, `pressure_active_cells=44`, `active_cell_overreach=2.315789`, `pressure_iterations=17`, `pressure_relative_residual=0.000084`, `pressure_converged=true`, `average_density_error=462.232177`, `max_density_error=502.098719`, `kinetic_energy=642.841951`, `mass_error=0.000000`, `elapsed_seconds=2.62`, `average_step_ms=0.4360`
- `demo-grid-flow` `fast` / `live`: `end_particles=240`, `active_cells=64`, `visible_cells=30`, `pressure_active_cells=64`, `active_cell_overreach=2.133333`, `pressure_iterations=17`, `pressure_relative_residual=0.000093`, `pressure_converged=true`, `average_density_error=18.901172`, `max_density_error=36.431042`, `kinetic_energy=502983870.681756`, `mass_error=0.000000`, `elapsed_seconds=0.21`, `average_step_ms=0.8548`
- `small-container-stress` `balanced` / `live`: `end_particles=221`, `active_cells=116`, `visible_cells=106`, `pressure_active_cells=116`, `active_cell_overreach=1.094340`, `pressure_iterations=38`, `pressure_relative_residual=0.000042`, `pressure_converged=true`, `average_density_error=0.052533`, `max_density_error=1.417903`, `kinetic_energy=308.477467`, `mass_error=0.000000`, `elapsed_seconds=13.35`, `average_step_ms=2.2251`
- `demo-grid-flow` `balanced` / `live`: `end_particles=267`, `active_cells=550`, `visible_cells=212`, `pressure_active_cells=550`, `active_cell_overreach=2.594340`, `pressure_iterations=32`, `pressure_relative_residual=0.000033`, `pressure_converged=true`, `average_density_error=0.305755`, `max_density_error=1.705209`, `kinetic_energy=1369642830.824932`, `mass_error=0.000000`, `elapsed_seconds=1.23`, `average_step_ms=5.1349`
- `small-container-stress` `quality` / `offline`: `end_particles=130`, `active_cells=112`, `visible_cells=59`, `pressure_active_cells=112`, `active_cell_overreach=1.898305`, `pressure_iterations=46`, `pressure_relative_residual=0.000006`, `pressure_converged=true`, `average_density_error=0.447172`, `max_density_error=0.932722`, `kinetic_energy=538.904483`, `mass_error=0.000000`, `elapsed_seconds=11.50`, `average_step_ms=1.9172`
- `demo-grid-flow` `quality` / `offline`: `end_particles=474`, `active_cells=925`, `visible_cells=325`, `pressure_active_cells=925`, `active_cell_overreach=2.846154`, `pressure_iterations=33`, `pressure_relative_residual=0.000008`, `pressure_converged=true`, `average_density_error=0.754656`, `max_density_error=0.937365`, `kinetic_energy=7431869224.119374`, `mass_error=0.000000`, `elapsed_seconds=2.20`, `average_step_ms=9.1699`
