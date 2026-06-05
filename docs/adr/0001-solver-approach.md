# ADR 0001: Solver Approach

Status: Accepted

Context:

The simulator needs water behavior that is deterministic enough for regression tests while still being expressive enough for interactive sandbox play.

Decision:

Use a 2D hybrid particle-grid water solver with a MAC grid, particle-to-grid transfer, pressure projection, and grid-to-particle feedback.

Consequences:

- Water motion remains stable enough for repeatable regression captures.
- Particles remain visible and easy to inspect during interactive editing.
- Future devices can interact with the solver through region-based effects without replacing the core simulation model.
