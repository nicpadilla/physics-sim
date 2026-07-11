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

## 20. Convincing Water Motion

Goal: make pouring, spreading, merging, splashing, sloshing, and settling read as real water to a user rather than a stable grid-based gel.

Completion markers:

- Deterministic feel scenarios measure leveling, footprint growth, stream continuity, slosh decay, wall-sheet flow, merging, breakup, vorticity, and particle sampling quality.
- Particle volume contributes smoothly to a sub-cell free-surface representation, and tiny fluid volumes render at approximately their conserved area instead of expanding into coarse polygons.
- Balanced and quality transfer and free-surface pressure behavior preserve believable shear and lateral flow while retaining conservation, bounded energy, and deterministic results.
- Conservative particle regularization prevents holes and clumps without introducing center-of-mass drift, unexplained mass changes, or global cohesion.
- Real-time sandbox water passes numeric, semantic, and named human review for pouring, spreading, grouping, sloshing, and settling before replacement goldens are accepted.

## 21. Complete Water Sandbox

Goal: finish the remaining simulation, presentation, creative-tool, challenge, content, audio, and release work as one coherent product rather than leaving recovered systems dormant.

Completion markers:

- Conservative neighborhood regularization reduces active-pour sampling variation and suppresses implausible tiny breakup without hiding or deleting physically supported droplets.
- Deterministic derived foam, spray, highlights, and impact accents enrich water motion without changing solver mass, state digests, reduced-motion behavior, or area accounting.
- Directional/omni emitters, gates, sensors, drains, pumps, and valves are restored through an accessible advanced sandbox palette with command, replay, persistence, performance, and usability evidence.
- Objectives return only as explicit challenge-scene contracts with visible progress, restart, completion feedback, deterministic state, and no hidden solver coupling.
- A curated gallery exposes only reviewed sandbox and challenge scenes with thumbnails, keyboard navigation, safe load failure, and package coverage.
- Layered water/device/objective audio is responsive, bounded, mute/volume aware, reduced-motion compatible, and resilient to missing or lost audio devices.
- The complete sandbox passes numeric, visual, audio, accessibility, tutorial, keyboard, package-directory, and extended-play human acceptance without regressing the pour/build loop.
- A versioned prerelease is committed, pushed through protected GitHub CI, tagged, checksummed, published with release artifacts, and verified from the downloadable package.
