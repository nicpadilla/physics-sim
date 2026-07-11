# Physics Sim 0.2.0-alpha.1

Release date: 2026-07-10

This is the first recovery prerelease: one deterministic simulation core serving a narrow basin sandbox and an engineering laboratory.

## Included

- Pour water; draw and erase walls; pause, single-step, resume, clear fluid, undo, redo, save, and reload.
- Interactive first-run guidance for the core sandbox loop.
- Dear ImGui laboratory with all canonical scenarios, balanced/quality profiles, metrics, plots, field views, deterministic replay, capture, and comparison.
- Scene v2, replay v2, and recovered-settings v1 contracts with safe failure and atomic scene persistence.
- Validated mass, transfer, pressure, collision, density, resampling, long-run, and determinism behavior.
- Connected water-surface rendering with semantic and exact-image regression evidence.
- Fast, Standard, and Full verification tiers, Windows CI, hygiene checks, and reproducible Release packaging.

## Intentionally deferred

Directional and omni emitters, broad gallery navigation, drains, pumps, valves, gates, sensors, objectives, progression, expanded tutorials, decorative audio/animation, and multi-save management remain outside the release UI. Their source history is retained, but none is considered release-ready without a separate value and evidence gate.

## Compatibility and limitations

- Pre-recovery scenes, settings, replays, saves, and goldens are not migrated.
- Windows x64 is the only supported packaged target.
- The solver targets deterministic, plausible 2D water; it is not a 3D, GPU, multiphase, or research-grade model.
- This remains an alpha. The intentionally narrow sandbox is accepted, while broader game content and presentation polish are future product decisions.

## Water-feel recovery update

- Corrected gravity units at sandbox cell scale, restored bounded FLIP/APIC motion, and removed nonlocal surface cohesion.
- Replaced coarse cell-expanded player water with an area-calibrated, solid-aware, four-times-resolution particle surface plus depth shading and a restrained highlight.
- Added deterministic leveling, steady-pour, slosh, wall-sheet, stream-merge, and obstacle breakup/rejoin validation in balanced and quality profiles.
- Replaced the basin visual baselines only after the 30-run numeric matrix, canonical contact sheet, and named review passed. Evidence is recorded in `docs/reviews/water-feel-acceptance-2026-07-11.md`.
