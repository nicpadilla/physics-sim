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

Balanced coverage includes still pool, hydrostatic column, overcrowding, U-container fill, dam break, wall/corner impact, hose pooling, narrow channel, obstacle field, drain basin, pumped loop, valve basin, and 6000-tick stress. Quality repeats still pool, hydrostatic column, overcrowding, and 6000-tick stress against the stricter reference profile.

Scenario-specific density, energy, jitter, reach, and pool-shape thresholds remain inside the executable assertions. Their calibrated parameters are explicit in the scenario setup and may change only through an issue that records before/after numeric and visual evidence.

The recovery profile disables APIC affine feedback and surface-tension injection at runtime because the baseline showed that those settings injected energy and spread water outside accepted bounds. APIC transfer remains covered as an isolated deterministic primitive; it may return to a runtime profile only after a dedicated stability issue passes these gates.

Pressure activation uses the full four-neighbor stencil required by the accepted U-container behavior. The benchmark bounds pressure-active/visible-cell overreach at `3.5`; directional halos produced lower counts but failed the fixed pool-height contract and were rejected during calibration.
