# Recovery Architecture

The recovery builds one executable from four compiled boundaries:

```text
physics-sim -> physics_sim_app -> physics_sim_content -> physics_sim_core
           -> physics_sim_lab --------------------------^
```

- `physics_sim_core` owns the deterministic simulation facade and legacy solver implementation. It may use only the C++ standard library and project physics headers.
- `physics_sim_content` is the persistence/replay boundary. PSIM-0092 will replace its current anchor with versioned format implementations.
- `physics_sim_app` owns the legacy SDL application while sandbox responsibilities are extracted under PSIM-0096.
- `physics_sim_lab` is an empty compiled boundary until Dear ImGui laboratory work lands under PSIM-0097.
- `src/main.cpp` contains only platform entry-point composition.

New clients use `SimulationConfig`, `SimulationCommand`, `SimulationSnapshot`, `SimulationMetrics`, and `Simulation`. The facade deliberately copies read-only snapshot data so rendering and UI cannot retain mutable solver references. Pause/single-step behavior is command-driven and the fixed timestep belongs to configuration.

The large legacy app implementation and internal water-solver orchestration remain behind these boundaries. Their further pass-by-pass extraction occurs only while implementing validated solver, presentation, sandbox, and laboratory issues, with baseline preservation checked at each stage.

Run the boundary check with:

```powershell
.\scripts\check-dependencies.ps1
```
