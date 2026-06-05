# Physics Known Limitations

This document records accepted scope constraints for the current physics model. These are not unfinished implementation tasks.

## Scope Constraints

- The simulator is a 2D solver with unit thickness. It does not model full 3D fluid behavior.
- The solver is CPU based, deterministic, and intentionally self-contained. It does not depend on a GPU fluid solver or an external physics library.
- The live gameplay target remains the `80 x 45` demo grid with `16` world units per cell and a fixed `1 / 120` second simulation tick.
- The solver does not guarantee 3D splash behavior, foam, mist, or molecular-scale particle physics.
- The default wall material is the documented near-free-slip no-penetration boundary model.
- Viscosity, surface tension, density correction, and particle resampling are bounded deterministic passes. They are designed to stay inside the documented live/offline budgets rather than emulate a full multiphase or free-surface research solver.
- Volume-fraction-based rendering is an approximation for density and surface occupancy. It is intended for stable visual feedback, not a full geometric surface reconstruction.

## Verification Boundary

- Live and offline performance budgets are tracked in [performance-budget.md](performance-budget.md).
- Boundary interaction expectations are tracked in [boundary-interaction.md](boundary-interaction.md).
- Physics accuracy and regression evidence are tracked through the Epic 11 issues and the associated test and regression scripts.
