# ADR 0004: Accuracy-First Fluid Quality

Status: Accepted

Context:

The current solver is a deterministic 2D hybrid particle-grid water simulation. It is playable and testable, and this ADR establishes a higher standard for realistic flow, pooling, wall interaction, mass conservation, and long-run stability.

Decision:

Future water-physics work will use an accuracy-first quality bar while preserving the existing self-contained C++20 solver architecture. The project will add measurable fluid-quality metrics, deterministic scenario tests, live and offline performance budgets, and a future issue epic before replacing or deeply modifying solver internals.

The live app must continue targeting 60 FPS on the current demo-grid scale: `80 x 45` cells with `16` pixel cell size. Offline validation scenarios may run slower if their budgets are documented.

Consequences:

- Solver changes must be justified by numeric behavior, not only by screenshots.
- The project can improve pressure solving, pooling, and wall interaction without adding runtime dependencies.
- Future tests may introduce stricter offline checks while keeping live defaults fast.
- Golden frame baselines remain supporting evidence and must be regenerated only inside issues that record the capture commands and rationale.

Non-goals:

- No GPU solver.
- No external fluid solver dependency.
- No 3D fluid simulation.
- No visual-only effects as substitutes for physics evidence.
