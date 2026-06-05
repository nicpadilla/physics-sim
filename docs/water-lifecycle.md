# Water Lifecycle

The water solver keeps its lifecycle policy intentionally simple for now:

- Particles that remain inside the simulation domain stay active and continue participating in the solver.
- Particles that start a step outside the domain, or move outside the domain during advection, are culled and counted as outflow.
- Particles that collide with solid cells are pushed back to their previous position and have their velocity zeroed instead of being removed.

The live metrics exposed by `WaterSimulation2D::metrics()` are:

- `total_emitted`: cumulative particles spawned by emitters.
- `active_particles`: particles currently still in the simulation.
- `total_removed`: cumulative particles removed from the simulation.
- `total_outflow`: cumulative particles removed because they left the domain.
- `active_cells`: fluid cells currently participating in the solver.
- `min_density`, `max_density`, `average_density`, `average_density_error`, and `max_density_error`: deterministic density diagnostics from the 2D cubic-spline kernel.
- `average_neighbor_count` and `max_neighbor_count`: deterministic particle-neighbor diagnostics from the density pass.

`resize()` and `clear_scene()` reset the particle state and lifecycle counters so a new scene starts from a clean slate.
