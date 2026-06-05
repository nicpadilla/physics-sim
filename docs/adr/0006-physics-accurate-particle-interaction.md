# ADR 0006: Physics-Accurate Particle Interaction

Status: Accepted

Date: 2026-06-05

## Context

The simulator has a deterministic 2D hybrid particle-grid water solver and a quality-suite direction for stronger fluid realism. Recent observations showed that water can still stop at walls or appear weakly interactive when particles crowd together. The project needs a precise physics contract before replacing solver internals so "accurate particle interaction" means a measurable fluid model rather than visual tuning.

## Decision

Continue with a deterministic 2D hybrid particle-grid solver and evolve particles into physical fluid parcels with mass, volume, density diagnostics, and APIC affine velocity state.

The target model is 2D incompressible Navier-Stokes:

```text
du/dt + (u dot grad)u = -(1/rho0) grad p + nu laplacian(u) + f
div u = 0
rho = rho0
```

Particle interaction is modeled through particle-grid transfer, pressure projection, viscosity, surface tension, boundary constraints, and bounded local density correction. Particles do not become visually larger when crowded; fluid volume and surface occupancy represent the larger pool.

The roadmap remains self-contained C++20 and SDL, CPU based, deterministic, and constrained by the current live target of `80 x 45` cells, `16` world units per cell, fixed `1 / 120` second simulation steps, and 60 FPS interaction.

## Consequences

- Future solver changes must expose physical metrics such as mass, density, divergence, residuals, and kinetic energy.
- Crowding fixes must preserve mass and avoid per-particle visual size growth.
- The pressure upgrade should supersede the deferred `PSIM-0044` work through the more specific `PSIM-0070` matrix-free projection issue.
- Live and offline quality tiers can use different iteration limits and residual tolerances while keeping the same equations.
- Visual baselines remain supporting evidence; numeric deterministic tests are the primary proof of physics behavior.

## Alternatives Considered

- SPH-only particles. Rejected because it would replace the existing MAC-grid architecture, risk a larger rewrite, and make current grid-based walls/devices less direct.
- 3D fluid simulation. Rejected because the product is a 2D sandbox and the current performance, rendering, and test harnesses are 2D.
- GPU solver. Rejected because it would add platform and determinism risk before the CPU solver reaches the target quality bar.
- External fluid dependency. Rejected because the project currently values a small self-contained C++20 solver with deterministic regression behavior.
- Visual-only particle growth. Rejected because it hides mass and density errors instead of solving particle interaction.
