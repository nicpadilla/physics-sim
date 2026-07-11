# Physics Known Limitations

This document records accepted scope constraints for the current physics model. These are not unfinished implementation tasks.

## Scope Constraints

- The simulator is a 2D solver with unit thickness. It does not model full 3D fluid behavior.
- The solver is CPU based, deterministic, and intentionally self-contained. It does not depend on a GPU fluid solver or an external physics library.
- The live gameplay target remains the `80 x 45` demo grid with `16` world units per cell and a fixed `1 / 120` second simulation tick.
- The solver does not guarantee 3D splash behavior, foam, mist, or molecular-scale particle physics.
- The default wall material is the documented near-free-slip no-penetration boundary model.
- Runtime solver profiles are `fast`, `balanced`, and `quality`. `balanced` is the default app profile and main runtime quality gate. `fast` preserves legacy compatibility and performance behavior and may keep large density-error values. `quality` is the strict offline/reference profile used for regression checks.
- Viscosity, density correction, APIC, and particle resampling are bounded deterministic passes. Surface tension is disabled by default. These systems are profile-dependent and designed to stay inside the documented budgets rather than emulate a full multiphase or research solver.
- Player rendering uses a deterministic four-times-resolution compact-kernel particle field, area-calibrated contour threshold, solid-aware support clipping, and triangle reconstruction. It is a convincing 2D visual surface, not a research-grade signed-distance or 3D free-surface mesh.
- Sparse fast impacts can still form a few small secondary droplets, and particle-count variation remains higher in active pours than in settled pools. The renderer does not artificially merge physically separated support or add hidden volume to conceal that limitation.
- The benchmark and fluid-quality harness still emit the legacy `tier` label for compatibility, but the governing contract is expressed through the solver profile.

## Verification Boundary

- Live and offline performance budgets are tracked in [performance-budget.md](performance-budget.md).
- Boundary interaction expectations are tracked in [boundary-interaction.md](boundary-interaction.md).
- Physics accuracy and regression evidence are tracked through the Epic 11 issues and the associated test and regression scripts.
