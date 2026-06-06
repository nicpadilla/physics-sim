# Physics Accuracy Contract

This document defines the target physics model for `PSIM-0066` and the later Epic 11 implementation issues. It is a contract for future solver changes, not a claim that every equation below is already implemented.

## Scope

The simulator targets deterministic 2D incompressible water suitable for live sandbox play and offline regression tests.

Particles are numerical fluid parcels. They are not molecules, do not collide as rigid balls, and do not become visually larger when crowded. Crowding must be represented through local density, pressure, density correction, and occupied surface or volume reconstruction.

The solver remains:

- C++20 and SDL based.
- Self-contained, with no external fluid solver dependency.
- 2D, not 3D.
- CPU based, not GPU based.
- Deterministic under fixed-step replay and regression conditions.

## Governing Equations

The target continuum model is incompressible Navier-Stokes in 2D:

```text
du/dt + (u dot grad)u = -(1/rho0) grad p + nu laplacian(u) + f
div u = 0
rho = rho0
```

Variables:

```text
x_p       particle position
v_p       particle velocity
m_p       particle mass
V_p       particle area-volume in 2D, with unit thickness
rho_p     particle density estimate
rho0      rest density
u         MAC-grid velocity field
p         pressure
nu        kinematic viscosity
sigma     surface tension coefficient
dx        grid cell size
dt        fixed simulation timestep
```

The pressure projection target is:

```text
div((1/rho) grad p) = (1/dt) div u*
u^(n+1) = u* - dt (1/rho) grad p
```

The local density target is:

```text
m_p = rho0 * V_p
rho_p = sum_q m_q W(|x_p - x_q|, h)
C_p = rho_p / rho0 - 1
```

## Units

The current simulation uses world coordinates measured in the same units as `MacGrid2D::cell_size()`.

- `dx`: `grid.cell_size()`. Unit-test and quality scenarios commonly use `1.0`. The live demo grid uses `16.0`.
- `dt`: fixed simulation step, currently `1 / 120` seconds in the app and quality harness.
- Render scale: one live demo grid cell maps to `16` screen pixels by default at the `80 x 45` grid scale.
- Position `x_p`: world units.
- Velocity `v_p` and grid velocity `u`: world units per second.
- Gravity: current implementation uses `9.8` world units per second squared in the positive Y direction.
- Particle volume `V_p`: target 2D area-volume in world units squared, with thickness assumed to be `1`.
- Particle mass `m_p`: target mass unit, computed from `rho0 * V_p`.
- Rest density `rho0`: target mass per world-unit area.
- Pressure `p`: target pressure-like solver unit consistent with `rho0 * dx^2 / dt^2` in the discretized projection. Existing pressure values are implementation-specific until `PSIM-0070` replaces the current projection.
- Viscosity `nu`: world units squared per second.
- Surface tension `sigma`: force per unit length in the 2D model, applied through bounded surface forces.

For later solver work, a default water parcel should use a documented `rho0`, `V_p`, and particle spacing such that summing particle volume over a cell reconstructs the intended occupied area.

## Invariants

The following invariants define acceptable physics behavior:

- Mass is conserved except for explicit drains, outflow cleanup, or documented device removal.
- Particle count may change only through explicit resampling rules that conserve total mass and momentum within tolerance.
- Linear momentum is conserved by particle-grid transfer when no boundaries, pressure, viscosity, gravity, or devices intervene.
- Pressure projection should drive `div u` toward `0` over fluid cells.
- Dense particle clusters should move toward `rho_p = rho0` through pressure and bounded density correction rather than stacking indefinitely.
- Solid boundaries enforce no penetration.
- Free surfaces use air pressure `p = 0` and remain bounded.
- All solver state used by tests must remain finite.
- Random-looking emission or resampling patterns must be deterministic.
- Default solid boundaries use the near-free-slip no-penetration contract documented in [boundary-interaction.md](boundary-interaction.md).

## Particle Interaction Policy

Particle interaction is primarily continuum interaction:

- Particles transfer mass and momentum to the MAC grid.
- The grid solves pressure and boundary constraints.
- Grid velocities transfer back to particles through PIC/FLIP/APIC.
- Local density correction is allowed only as a bounded distribution fix after projection.

When too many particles move into one location, individual particles must not be rendered larger. The expected behavior is:

- Local density increases.
- Pressure and density correction push particles apart.
- The occupied fluid region or reconstructed free surface grows according to total volume.
- Rendering reads the larger pool from volume fraction or surface occupancy, not from per-particle visual size.

## Solver Profiles And Quality Budgets

The contract is profile-based. The benchmark still reports `tier` for compatibility, but the runtime quality contract is the solver profile:

| Profile | Benchmark tier | Role | Pressure target | Density target |
| --- | --- | --- | --- | --- |
| `fast` | `live` | Compatibility and performance path for legacy regression coverage | relative residual `<= 1e-4` | relaxed, not a hard quality gate |
| `balanced` | `live` | Default app profile and main runtime quality gate | relative residual `<= 5e-5` | average density error `<= 3.0`, max density error `<= 10.0`, active-cell overreach `<= 3.0` |
| `quality` | `offline` | Strict offline/reference profile | relative residual `<= 1e-5` | average density error `<= 1.0`, max density error `<= 1.25`, active-cell overreach `<= 3.0` |

All three profiles use the same governing equations and deterministic ordering. The differences are in enabled passes, iteration budgets, and density targets:

- `fast` preserves the legacy live-budget path and is allowed to keep relaxed density behavior.
- `balanced` is the default app profile and should remain inside the documented runtime density budget.
- `quality` enables the strictest density correction, resampling, and APIC affine transfer settings used for offline-style checks.

The current target scale remains `80 x 45` cells, `16` world units per cell, fixed `1 / 120` second simulation tick, 60 FPS interaction target. The live/offline benchmark tiers are retained only as compatibility labels for the benchmark and scripts.

## Current Implementation Notes

The solver now includes the physical state and bounded correction passes needed by Epic 11:

- Particle mass, volume, density, neighbor counts, and affine velocity are part of `FluidParticle`.
- Fluid, air, and solid cell classification uses deterministic volume fractions and cell density diagnostics.
- Transfer uses mass-weighted APIC scatter helpers, PIC/FLIP blending, and deterministic affine state support.
- Runtime APIC affine updates are profile controlled; the quality profile enables affine transfer while balanced keeps it disabled for current stability.
- Pressure projection reports matrix-free PCG iterations, residuals, convergence, and active pressure cells.
- Pressure projection also reports visible fluid cells, pressure-active cells, overreach ratio, RHS norm, solution norm, `dt`, and rest density for diagnostics.
- Local density correction is implemented as a bounded deterministic pass with mass and center-of-mass diagnostics.
- Boundary interaction is documented as a near-free-slip no-penetration material model using deterministic swept-sample and cell-face contact. A damped material setting can reduce tangential velocity, but scene-authored wall materials are not exposed yet.
- Viscosity and surface tension are implemented as bounded deterministic forces with zero-effect defaults.
- Particle resampling preserves physical mass and volume and keeps rendering tied to volume fraction rather than particle size.

Accepted scope constraints and remaining approximation notes are documented in [physics-known-limitations.md](physics-known-limitations.md).
