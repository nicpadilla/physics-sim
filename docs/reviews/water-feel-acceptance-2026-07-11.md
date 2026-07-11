# Water Feel Acceptance - 2026-07-11

Reviewer: Codex, acting under the project owner's explicit instruction to reach independent conclusions and continue without further input.

Decision: Accepted for the recovered sandbox and lab water experience, subject to the documented 2D scope limits.

## Build and evidence

- Windows C++20 Release and Debug builds, MSVC 19.44, Windows SDK 10.0.26100.0, SDL2 2.32.10, fixed `1/120` second simulation tick.
- `.\scripts\verify-fluid-quality-suite.ps1` passed all 30 balanced/quality runs in 299.1 seconds on 2026-07-11.
- `.\scripts\run-lab-smoke.ps1 -Canonical -SkipBuild` passed all 10 canonical captures in 18.4 seconds on 2026-07-11, including tick 6000 long run and tick 2400 determinism.
- `.\scripts\verify-all.ps1` passed 36/36 Full tests plus tracking, hygiene, package creation, replay, and visual verification in 169.6 seconds on 2026-07-11.
- Contact sheet: `build/windows-x64/recovery-visual-review/recovery-contact-sheet.bmp`.
- Structured named-review manifest: `build/windows-x64/recovery-visual-review/review-manifest.json`.
- Numeric evidence: `build/windows-x64/fluid-quality-suite/summary.json` and its per-scenario logs.
- Live sandbox review: 96.7 simulated seconds of pointer pouring, separated puddles, a sustained central stream, merging, and settling with 414 emitted parcels; the title telemetry remained around 127-138 FPS with the fixed `0.01`-second displayed step and no removed/outflow water.
- Continuous populated soak: Release balanced sandbox with `recovery-basin.replay` ran from 10:27:06 through 10:42:06 on 2026-07-11. It emitted through tick 1200, continuously simulated and rendered the settled pool, saved settings, and exited normally at the configured 900 seconds. Log: `build/windows-x64/water-feel-soak.log`.

## Review findings

- Pouring: the tick-240 basin capture is a continuous tapered stream with a rounded leading drop, not a cell column, rope, or enlarged blob.
- Spreading and leveling: corrected gravity units make sandbox-scale water fall and spread at the same cell-relative acceleration as unit-grid scenarios. The settled basin spans roughly 34 cells instead of remaining a central mound. Balanced asymmetric-leveling RMS slope improved from `0.365` to `0.117`.
- Grouping: supported particles reconstruct as a single cohesive body without nonlocal attraction. Separated support remains separated; the renderer does not invent connecting volume.
- Merging and obstacle flow: two streams merge to one component. Obstacle flow rejoins substantially, with the balanced largest-component fraction improving from `0.765` to `0.895`; small secondary droplets remain possible after energetic impacts.
- Slosh and settling: the revised FLIP/APIC blends retain more lateral and rotational motion while all settled-energy, pressure, determinism, and long-run gates remain bounded.
- Walls: solid-aware kernel visibility prevents reconstructed water from crossing walls or floors. Wall/corner and narrow-channel captures show bounded deflection and throughput with zero particles in solids.
- Rendering: the player surface uses an area-calibrated four-times-resolution particle field, continuous fill, depth color, and a restrained free-surface highlight. Reconstructed basin area differs from particle area by about `0.045%` at the accepted settled capture.
- Interaction: direct pointer drags produced small independent puddles when spatially separated, a rounded connected falling column when repeatedly poured in one place, and a shallow wavy pool after settling. Runtime motion matched the fixed captures without visible cell popping or geometry seams.
- Lab integrity: large reconstructed meshes initially exposed invalid cross-screen triangles caused by 16-bit ImGui vertex-range wrapping. The renderer now advertises and honors vertex offsets; the regenerated canonical sheet contains no unrelated geometry.

## Accepted regression baselines

| Artifact | SHA-256 | Review purpose |
| --- | --- | --- |
| `regression/recovery_basin_early_surface_golden.bmp` | `A631FEC2E079758D35B796BC18437878639A7E2D51244F7842ECE3522B849328` | Continuous tick-240 pour and leading-drop shape. |
| `regression/recovery_basin_density_golden.bmp` | `FDE45BC8A021EE7868DA92DD4E66E05A2B996A5690BFDA35C1688DFD0F30BBFD` | Truthful tick-2400 density distribution. |
| `regression/recovery_basin_surface_golden.bmp` | `9831F9628DFF4C61CC098E69289718B72D1E634BE352ED3F4FDE54A6A66EB7E6` | Area-faithful tick-2400 shallow pool and edge treatment. |

## Accepted limitations

- This is plausible deterministic 2D water, not 3D splash, foam, spray, or research-grade multiphase simulation.
- Active pours and obstacle breakup retain more particle-count variation than settled pools. Small physically separated droplets are shown rather than cosmetically merged.
- Surface tension remains disabled by default. Local curvature force infrastructure remains available, but the removed global center-attraction behavior is not acceptable for water.

These limits do not make the core pour, redirect, pool, split, merge, slosh, or settle interactions read as gel, sand, smoke, or disconnected grid cells in the reviewed build.
