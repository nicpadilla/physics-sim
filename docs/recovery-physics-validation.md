# Recovery Physics Validation

The recovery contract is enforced by `regression/fluid_quality_suite.psd1`, focused core tests, and `.\scripts\verify-fluid-quality-suite.ps1`.

## Hard Gates

- Relative mass-accounting error: `<= 1e-5`, excluding explicitly reported drain/outflow mass.
- Force-free particle-grid momentum relative error: `<= 1e-4` in focused transfer tests.
- Pressure relative residual: balanced `<= 5e-5`; quality `<= 1e-5`.
- Particles inside solids: `0`.
- Non-finite values in reported particle/solver state: `0`.
- Unexplained lifecycle changes: `0`.

The suite writes `build/windows-x64/fluid-quality-suite/summary.json` after each completed case. Every result records scenario, profile, actual values, thresholds, pass state, and its retained case-log path.

## Canonical Scenarios

Balanced coverage includes still pool, hydrostatic column, overcrowding, U-container fill, dam break, wall/corner impact, hose pooling, narrow channel, obstacle field, drain basin, pumped loop, valve basin, 6000-tick stress, asymmetric leveling, steady-pour feel, slosh decay, wall-sheet flow, two-stream merge, and obstacle breakup/rejoin. Quality repeats the reference scenarios and all six motion-feel scenarios against the stricter profile.

Scenario-specific density, energy, jitter, reach, and pool-shape thresholds remain inside the executable assertions. Their calibrated parameters are explicit in the scenario setup and may change only through an issue that records before/after numeric and visual evidence.

Balanced and quality now use a bounded `0.10` APIC affine contribution with calibrated FLIP blends of `0.78` and `0.71`, respectively. The 30-run matrix retains energy, pressure, containment, lifecycle, and repeated-digest gates with those values. Surface tension remains disabled by default; nonlocal center attraction was removed rather than used to make particles appear connected.

Gravity is stored as world acceleration and converted to grid-relative velocity with `cell_size` during integration. A focused scale-invariance test prevents a change in physical cell size from changing acceleration measured in cells per second squared.

Particle volume is rasterized conservatively to neighboring cell centers for smooth diagnostics and rendering, while pressure topology remains center-occupied. This prevents tiny raster tails from becoming full pressure cells. Pressure activation uses the full four-neighbor stencil required by the accepted U-container behavior; the benchmark continues to bound pressure-active/visible-cell overreach at `3.5`.
