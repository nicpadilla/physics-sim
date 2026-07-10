# Recovery Roadmap

This is the active product contract for the Physics Sim recovery program. The pre-recovery contract is preserved by the `pre-recovery-2026-07-10` Git tag and documented in `docs/history/pre-recovery-snapshot.md`.

## Completion Rules

- A marker is `Human Accepted` only after a named visual or usability review.
- A deterministic golden hash proves reproducibility, not product quality.
- Numeric physics gates and their scenario manifests are the primary solver evidence.
- No stage is complete while one of its markers is `Missing`, `Partial`, or `Blocked`.

## 14. Recovery Foundation

Goal: establish truthful tracking, durable architecture boundaries, and reproducible baseline evidence.

Completion markers:

- The legacy roadmap, progress ledger, issues, and regression evidence are preserved as a named pre-recovery snapshot.
- Active tracking distinguishes implemented behavior, automated evidence, and human acceptance.
- Recovery issues are implementation-ready and summary rows agree with detailed issue records.
- Recovery ADRs define the dual-mode product, compiled modules, SDL/ImGui boundary, breaking formats, and verification tiers.
- A baseline report records toolchain, commands, timings, metrics, captures, and classified failures.
- The application is decomposed into compiled core, content, app, and lab targets with enforced dependency direction.

## 15. Validated Water Core

Goal: make the shared deterministic solver numerically credible and independently inspectable.

Completion markers:

- Stable simulation config, command, snapshot, and metrics interfaces separate clients from solver internals.
- Unit tests cover conservation, transfer, pressure, equilibrium, gravity, collision, density correction, resampling, accounting, and determinism.
- Canonical scenario manifests cover closed box, U-container, still pool, hydrostatic column, dam break, impacts, narrow channel, overcrowding, and long-run stability.
- Hard gates enforce mass and momentum accounting, pressure convergence, zero solid penetration, finite state, and deterministic digests.
- Balanced and quality profiles pass their numeric and performance budgets without hiding earlier-stage failures in later tuning.

## 16. Cohesive Water Presentation

Goal: render the validated volume field as readable, connected water.

Completion markers:

- Player water uses deterministic continuous surface reconstruction rather than isolated filled cells.
- Rendering interpolates fixed simulation states without changing physics state.
- Lab views expose particles, density, velocity, pressure, divergence, volume fractions, and solid classification.
- Semantic visual checks measure expected region, connectivity, pool dimensions, escaped water, and isolated cells.
- Basin and still-pool captures pass numeric checks and a named human visual review before goldens are accepted.

## 17. Sandbox And Lab Experiences

Goal: provide two intentional clients of the same simulation core.

Completion markers:

- Sandbox mode supports pour, wall draw/erase, pause/step/resume, reset, undo/redo, save/load, and mode/menu navigation.
- Sandbox UI is player-facing SDL UI with mouse-first controls, keyboard alternatives, accessibility settings, and no visible ImGui.
- First-run interaction teaches the narrow sandbox loop without requiring external documentation.
- Lab mode uses Dear ImGui for scenarios, profiles, controls, metrics, plots, field views, capture, replay, and side-by-side comparison.
- Determinism-affecting lab changes reset or fork the run and visibly identify digest divergence.
- Secondary devices, progression, objectives, broad gallery content, and decorative polish are absent from the first recovery release UI.

## 18. Verification And Release

Goal: make development feedback fast and releases reproducible.

Completion markers:

- Fast, standard, and full verification tiers meet 30-second, 90-second, and 8-minute targets respectively.
- CTest labels separate unit, integration, solver, visual, benchmark, smoke, and release coverage.
- Failures retain structured metrics and captures with scenario, profile, tick, seed, threshold, and artifact paths.
- Windows CI builds, runs appropriate tiers, uploads failure evidence, and performs nightly and tagged full verification.
- Static analysis enforces warnings, formatting, include direction, absolute-path policy, and secret hygiene.
- A `0.2.0-alpha.1` package launches outside the build tree and includes manifests, licenses, checksums, and verification logs.

## 19. Selective Feature Restoration

Goal: restore only features that strengthen the sandbox or laboratory experience.

Completion markers:

- Every deferred feature has a documented product decision: restore, redesign, or remove.
- Restored features have an isolated interface, automated evidence, human acceptance, persistence representation, and performance budget.
- Restored features do not regress the pour/build/reset/save vertical slice.
- Final recovery acceptance has no open P0 or P1 issues and reconciles roadmap, evidence, package, and release notes.
