# Fluid Realism Roadmap

This document defines the future accuracy-first water workstream. The goal is to make water behavior measurable, repeatable, and more physically plausible before deeper solver changes are made.

The more specific physics contract for particle interaction, units, invariants, live/offline tiers, and governing equations lives in [docs/physics-accuracy-contract.md](physics-accuracy-contract.md). Future solver changes should treat that contract and ADR 0006 as the source of truth for Epic 11.

## Target Model

The simulator should continue to use deterministic 2D water, but the quality bar should move closer to a real incompressible fluid:

- Water should flow away from emitters, respond to gravity, and transfer momentum through the particle-grid solver.
- Water should pool in basins with stable height, bounded jitter, and bounded velocity after settling.
- Water should collide with walls, deflect from surfaces, and never persist inside solid cells.
- Water should conserve mass except for explicit outflow, drains, or other documented removal devices.
- Solver health should be visible through numeric metrics, not only through rendered screenshots.

The live game should remain responsive at the current demo scale: `80 x 45` cells, `16` pixel cell size, fixed simulation at `120` ticks per second, and a 60 FPS interaction target.

## Out Of Scope

The next realism pass should not attempt:

- 3D fluid.
- Spray mist, foam, or particle effects as physics evidence.
- Turbulence accuracy beyond bounded and plausible motion.
- Thermodynamics, phase changes, or viscosity presets.
- GPU solvers.
- External fluid or physics libraries.

These can be reconsidered only with a later ADR.

## Metrics

Future tests should sample scenario state at fixed ticks and assert numeric ranges. Important metrics include:

- Particle count and active particle count.
- Total emitted, total removed, total outflow, and mass balance.
- Average and max divergence after projection.
- Average speed, max speed, and kinetic energy.
- Active fluid cells.
- Center of mass and water bounding box.
- Particles inside solid cells.
- Particles outside intended domain or scenario bounds.
- Pooled water height by columns or occupancy bands.
- Scenario elapsed time, average step time, active cells, and particle count.

Metrics should be exposed through test helpers first. Script output can use stable text or JSON once the scenario suite exists.

## Scenario Suite

The future suite should include deterministic scenario tests that run without launching SDL:

- Still pool: verifies resting water remains stable.
- U-container fill: verifies mass conservation, height increase, and containment.
- Dam break: verifies water-front progress and final pooling range.
- Hose into wall: verifies impact, deflection, and no wall penetration.
- Narrow channel: verifies flow through a constrained path with no corner leakage.
- Obstacle field: verifies split and recombine behavior around blocks.
- Drain basin: verifies explicit removal and mass accounting.
- Pumped loop: verifies device acceleration remains bounded.
- Valve-controlled basin: verifies open and closed device states affect flow.
- Long-run stress scene: verifies finite values and bounded metrics over extended runs.

Each scenario should define grid size, cell size, walls, devices, emitters, tick sample points, metric thresholds, optional visual captures, and whether it is a live-speed or offline-only check.

The current implementation lives in `scripts\verify-fluid-quality-suite.ps1` and uses `regression\fluid_quality_suite.psd1` to drive the `physics_sim_fluid_quality_tests` target one scenario at a time.

## Performance Policy

Solver improvements should split performance checks into two categories:

- Live checks: current interactive scale, 60 FPS target, strict runtime budget.
- Offline checks: heavier scenarios or stricter solver tolerances, allowed to run slower but with documented limits.

The current `scripts\measure-water-solver.ps1` benchmark already includes a demo-grid case. Future work should either extend it or add a companion script for offline accuracy checks.

## Evidence Policy

Numeric scenario tests are the primary evidence for physics correctness. Golden BMP captures remain useful for visual regressions, but they should not be the only proof that water flows, pools, or collides correctly.

Every future implementation issue should record:

- The scenario or unit tests that failed first.
- The solver or harness change made.
- The exact verification commands run.
- Any baseline regeneration command and reason.

## Workstream Exit

The future fluid-realism workstream is complete only when:

- The metric snapshot harness exists.
- Stronger pressure/incompressibility behavior is tested.
- Pooling and free-surface stability are measured.
- Wall interaction and no-penetration behavior are measured.
- The multi-scenario suite passes deterministically.
- Live and offline performance budgets are documented and passing.
- Scenario gallery scenes exist for visual inspection.
- The final audit records limitations and verification results.
