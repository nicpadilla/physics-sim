# Recovery Issues

The pre-recovery issue ledger is preserved by the `pre-recovery-2026-07-10` tag. This file is the active implementation queue.

## Open Queue

| ID | Status | Priority | Epic | Linked roadmap IDs | Title |
| --- | --- | --- | --- | --- | --- |
| PSIM-0089 | Done | P0 | Recovery Foundation | R14.01, R14.02, R14.03, R14.04 | Reset recovery governance and architecture decisions |
| PSIM-0090 | Done | P0 | Recovery Foundation | R14.05 | Capture trusted pre-refactor baseline |
| PSIM-0091 | Done | P0 | Recovery Architecture | R14.06, R15.01 | Introduce compiled boundaries and stable simulation API |
| PSIM-0092 | Done | P1 | Recovery Architecture | R15.01, R17.01, R17.04 | Replace scene, replay, and settings contracts |
| PSIM-0093 | Done | P0 | Validated Water | R15.02, R15.03, R15.04 | Rebuild invariant and scenario validation |
| PSIM-0094 | Done | P0 | Validated Water | R15.05 | Correct solver behavior against recovery gates |
| PSIM-0095 | Done | P0 | Water Presentation | R16.01, R16.02, R16.04, R16.05 | Reconstruct cohesive water and semantic visual gates |
| PSIM-0096 | Done | P0 | Product Experiences | R17.01, R17.02, R17.03, R17.06 | Deliver narrow sandbox vertical slice |
| PSIM-0097 | Done | P0 | Product Experiences | R16.03, R17.04, R17.05 | Deliver Dear ImGui laboratory mode |
| PSIM-0098 | Done | P1 | Verification And Release | R18.01, R18.02, R18.03 | Introduce tiered structured verification |
| PSIM-0099 | Done | P1 | Verification And Release | R18.04, R18.05, R18.06 | Add CI, hygiene, and prerelease packaging |
| PSIM-0100 | Done | P2 | Selective Restoration | R19.01, R19.02, R19.03 | Audit and gate deferred features |
| PSIM-0101 | Done | P0 | Recovery Acceptance | R19.04 | Complete recovery release acceptance |
| PSIM-0102 | Done | P0 | Convincing Water Motion | R20.01 | Add deterministic water-feel validation |
| PSIM-0103 | Done | P0 | Convincing Water Motion | R20.02 | Build an area-faithful particle free surface |
| PSIM-0104 | Done | P0 | Convincing Water Motion | R20.03 | Restore believable transfer and free-surface flow |
| PSIM-0105 | Done | P1 | Convincing Water Motion | R20.04 | Regularize particles and remove artificial cohesion |
| PSIM-0106 | Done | P0 | Convincing Water Motion | R20.05 | Tune, review, and accept real-time water feel |
| PSIM-0107 | Done | P0 | Complete Water Sandbox | R21.01 | Regularize active pours and refine droplets |
| PSIM-0108 | Open | P1 | Complete Water Sandbox | R21.02 | Add deterministic foam and spray presentation |
| PSIM-0109 | Open | P0 | Complete Water Sandbox | R21.03 | Restore advanced water tools and devices |
| PSIM-0110 | Open | P1 | Complete Water Sandbox | R21.04 | Restore challenge objectives |
| PSIM-0111 | Open | P1 | Complete Water Sandbox | R21.05 | Ship the curated scene gallery |
| PSIM-0112 | Open | P1 | Complete Water Sandbox | R21.06 | Polish water and device audio |
| PSIM-0113 | Open | P0 | Complete Water Sandbox | R21.07, R21.08 | Accept and publish the complete prerelease |

## Epic 12: Recovery Foundation

### PSIM-0089: Reset recovery governance and architecture decisions

Status: Done

Priority: P0

Linked roadmap IDs: R14.01, R14.02, R14.03, R14.04

Problem:
The pre-recovery tracker reports universal verification even though evidence is stale, visual quality is unaccepted, and its summary disagrees with the detailed status of legacy issue 0088.

Technical implementation direction:

- Preserve commit `647153c` with a named tag and a history document that explains how to inspect the legacy tracker and goldens.
- Replace active roadmap/progress/issues with the recovery contract, beginning at PSIM-0089 and roadmap stage 14.
- Update `scripts/check-tracking.ps1`, `docs/TRACKING.md`, and `AGENTS.md` for the new statuses, dated evidence contract, visual acceptance rule, and exact summary/detail validation.
- Add accepted ADRs for the dual product, compiled modules, SDL/ImGui boundary, breaking formats, and verification tiers.
- Rewrite `README.md` so it describes the current recovery state rather than a finished scaffold.

Acceptance criteria:

- Tracking rejects summary/detail differences in ID, title, status, priority, or epic.
- The only allowed progress statuses are Missing, Partial, Implemented, Automated, Human Accepted, and Blocked.
- Visual/usability acceptance cannot be claimed from a hash-only regression.
- Every later issue is implementation-ready and links to recovery roadmap IDs.
- Tracking and tests pass; changes are committed as a governance-only unit.

Subtasks:

- Create snapshot reference and recovery ADRs.
- Replace active tracking documents.
- Strengthen the tracking validator and its workflow documentation.
- Rewrite the README and verify referenced commands.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- None.

Implementation notes:

- Replaced the legacy all-green roadmap, progress dashboard, and issue queue with 32 recovery markers and 13 implementation-ready issues across eight recovery epics.
- Preserved commit `647153c` with annotated tag `pre-recovery-2026-07-10` and documented direct inspection commands in `docs/history/pre-recovery-snapshot.md`.
- Added ADRs 0007 through 0011 for the dual-mode product, compiled modules, SDL/ImGui boundary, breaking formats, and tiered verification.
- Updated the tracker and operating rules to distinguish Implemented, Automated, and Human Accepted evidence and to reject hash-only visual acceptance.
- Strengthened `scripts/check-tracking.ps1` so every detailed issue must have a unique summary row with the same title, status, priority, and epic.
- Rewrote `README.md` to state the recovery status, current commands, known limitations, and intentionally deferred breadth.
- Verification on 2026-07-10: `.\scripts\check-tracking.ps1` passed in 0.6 seconds with 32 markers, 13 issues, and 8 epics; `.\scripts\test.ps1` passed 26/26 tests in 170.7 seconds.

### PSIM-0090: Capture trusted pre-refactor baseline

Status: Done

Priority: P0

Linked roadmap IDs: R14.05

Problem:
The project has ad hoc June evidence but no complete recovery baseline that ties toolchain, commands, metrics, images, timings, and classified failures to one commit.

Technical implementation direction:

- Add a read-only capture script that records Git/toolchain/build configuration and invokes tracking, build, CTest, smoke, replay, fluid-quality, all-profile benchmark, visual captures, and packaging.
- Emit a Markdown report plus structured JSON and an artifact manifest under a documented generated baseline directory; commit only the report, manifest schema, review sheet, and intentionally preserved legacy goldens.
- Create fixed-tick evidence for starter basin, U-container, still pool, hydrostatic column, dam break, wall/corner impact, narrow channel, overcrowding, and long-run scenarios.
- Classify every observed failure as numerical, rendering, interaction, performance, test-quality, documentation, or release debt.

Acceptance criteria:

- One command recreates the baseline without modifying accepted goldens.
- Every command records date, duration, exit code, profile/scenario, and artifact path.
- The report contains a visual review sheet and explicit failure classification.
- Legacy goldens are labeled non-acceptance evidence.

Subtasks:

- Add capture/report schema and script.
- Run the complete baseline.
- Inspect captures and classify failures.
- Reconcile R14.05 evidence.

Verification:

- `.\scripts\capture-recovery-baseline.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0089.

Implementation notes:

- Added `scripts/capture-recovery-baseline.ps1` with environment identity, timed step execution, per-step logs, structured JSON, legacy golden hashes, optional published report, and strict `-FailOnStepFailure` mode.
- Added the baseline summary JSON schema and published the dated report/review sheet in `docs/baselines/recovery-baseline-2026-07-10.md` without duplicating the large legacy BMPs preserved by the pre-recovery tag.
- The clean capture completed in 570.5 seconds on 2026-07-10. Tracking, build, 26/26 aggregate CTest, smoke, all-profile benchmark, three demo hashes, and package creation passed.
- The baseline correctly retained two product failures: directional replay hash mismatch (`0A939310...70D83` versus `DE40E02A...3F5B2`) and quality long-run average density error `1.311553` above the `1.0` limit.
- Classified numerical, determinism, rendering, interaction, performance, test-quality, documentation, and release debt. No golden or threshold was changed.
- Verification: `.\scripts\capture-recovery-baseline.ps1` completed with exit code 0 in 570.5 seconds and `.\scripts\check-tracking.ps1` passed on 2026-07-10. The two recorded product failures are baseline findings assigned to later recovery issues, not capture-script failures.

## Epic 13: Recovery Architecture

### PSIM-0091: Introduce compiled boundaries and stable simulation API

Status: Done

Priority: P0

Linked roadmap IDs: R14.06, R15.01

Problem:
The application is concentrated in a monolithic `main.cpp` and large algorithmic headers behind an INTERFACE library, allowing UI and rendering to couple directly to solver implementation.

Technical implementation direction:

- Create compiled `physics_sim_core`, `physics_sim_content`, `physics_sim_app`, and `physics_sim_lab` targets while retaining one `physics-sim` executable.
- Introduce `SimulationConfig`, `SimulationCommand`, `SimulationSnapshot`, `SimulationMetrics`, and a `Simulation` facade with apply, step, snapshot, and metrics operations.
- Move algorithms into `.cpp` units by solver pass and app responsibility; keep public headers declarative.
- Enforce core -> standard library only, content -> core, app -> content/core/SDL, and lab -> content/core/SDL/ImGui dependency direction.
- Extract behavior in small commits without changing legacy state digests before PSIM-0093 begins.

Acceptance criteria:

- `main.cpp` performs only option parsing and composition.
- Rendering and UI cannot mutate solver internals outside commands.
- Core sources contain no SDL or ImGui includes.
- Baseline state digests and supported captures remain unchanged through the refactor.

Subtasks:

- Add compiled targets and facade types.
- Extract solver passes and content parsing.
- Extract app lifecycle, input, rendering, menus, and diagnostics.
- Add include-boundary and facade tests.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0090.

Implementation notes:

- Replaced the INTERFACE core with compiled `physics_sim_core`, `physics_sim_content`, `physics_sim_app`, and `physics_sim_lab` static targets behind one executable.
- Moved the legacy 4,500-line SDL implementation into `src/app/application.cpp`; `src/main.cpp` now contains only platform entry-point composition.
- Added the PIMPL-based `Simulation` facade with sanitized config, command application, pause/single-step, fixed stepping, copied snapshots, and client-facing metrics. The core facade exposes no SDL or ImGui types.
- Added configurable gravity to solver settings without changing the 9.8 default.
- Added simulation API tests and an automated dependency check enforcing core/content platform independence, app exclusion of ImGui, and composition-only main includes.
- Documented the compiled boundaries and the remaining staged extraction: legacy app responsibilities will separate under PSIM-0096 and internal solver passes under PSIM-0093/0094 rather than performing an unsafe rewrite in this behavior-preserving issue.
- Verification on 2026-07-10: `.\scripts\build.ps1` passed in 12.1 seconds; `.\scripts\test.ps1` passed 28/28 in 170.8 seconds; `.\scripts\run-smoke.ps1`, the simulation API test, and `.\scripts\check-dependencies.ps1` passed. The known directional replay failure retained the exact pre-refactor capture hash `DE40E02A...3F5B2`, proving this refactor did not alter that baseline defect.

### PSIM-0092: Replace scene, replay, and settings contracts

Status: Done

Priority: P1

Linked roadmap IDs: R15.01, R17.01, R17.04

Problem:
Legacy persistence and replay formats expose old implementation choices and do not provide atomic recovery, command-level replay identity, or structured regression metrics.

Technical implementation direction:

- Introduce scene v2 with explicit grid, metadata, walls, emitters, supported devices, and solver profile; reject v1 with a player-facing unsupported message.
- Implement atomic temporary-write, validation, backup, and replace for player saves.
- Introduce replay v2 with scene digest, timestep, profile, tick-indexed `SimulationCommand` records, and optional expected final digest.
- Reset recovered settings to schema v1 and persist only settings present in the recovered UI.
- Separate player saves, authored content, generated captures, and structured metrics paths.

Acceptance criteria:

- All bundled content uses the new formats and deterministic round-trips.
- Malformed/unsupported inputs fail safely and explain recovery.
- Interrupted save tests preserve the previous valid file.
- Replays fail clearly when scene/config identity differs.

Subtasks:

- Define and document schemas.
- Implement parsers, writers, atomic save, and validation.
- Regenerate bundled scenes/replays and reset settings tests.
- Remove obsolete compatibility paths and baselines after acceptance.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1`
- `.\scripts\verify-replay-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0091.

Implementation notes:

- Made scene v2 intentionally exclusive: version 1, missing explicit solver profile, duplicate/malformed headers, and unknown versions are rejected. Regenerated all eight bundled scenes with explicit balanced profiles.
- Added validated atomic scene publication through `.tmp` and `.bak`, including round-trip validation before replacement and restoration on publish failure. Tests prove replacement and previous-file backup behavior.
- Added replay v2 identity fields for exact scene digest, fixed timestep, solver profile, and optional expected final digest. The digest is deterministic FNV-1a 64 over exact scene bytes; application startup rejects scene/timestep/profile mismatch before replay execution.
- Regenerated all repository replays with v2 identity. Events remain semantic tick-indexed commands, never SDL events; direct facade-command expansion continues with lab work.
- Reset settings to the distinct `physics-sim-recovery-settings 1` contract, requiring the complete recovered settings document and rejecting all pre-recovery headers/versions.
- Rewrote scene, replay, and settings documentation and added parser, identity, unsupported-version feedback, backup, and bundled-content tests.
- Verification on 2026-07-10: `.\scripts\build.ps1` passed in 15.6 seconds; targeted scene/replay/settings/gallery/tutorial tests and clean demo regression passed; `.\scripts\test.ps1` passed 28/28 in 178.1 seconds. The dedicated replay suite reaches execution through valid v2 identity and retains the known pre-recovery directional hash mismatch unchanged for PSIM-0093/0094.

## Epic 14: Validated Water

### PSIM-0093: Rebuild invariant and scenario validation

Status: Done

Priority: P0

Linked roadmap IDs: R15.02, R15.03, R15.04

Problem:
Existing tests prove many deterministic implementation behaviors but do not independently establish the recovery physics contract or structured scenario evidence.

Technical implementation direction:

- Add focused conservation, transfer, pressure, equilibrium, gravity, material, tunneling, density, resampling, accounting, and repeat-digest tests through the compiled core API.
- Define a structured scenario manifest for closed box, U-container, still pool, hydrostatic column, dam break, wall/corner impacts, narrow channel, overcrowding, and 6000-tick stress.
- Enforce mass error <=1e-5, force-free transfer momentum error <=1e-4, balanced residual <=5e-5, quality residual <=1e-5, zero particles in solids, zero non-finite values, and zero unexplained lifecycle changes.
- Emit JSON failures containing scenario, profile, tick, seed, actual, threshold, and artifact paths.

Acceptance criteria:

- Each hard gate has a focused failing test against a controlled defect or fixture.
- Scenario output is deterministic and machine-readable.
- Scenario-specific perceptual thresholds remain explicitly unaccepted until PSIM-0094/0095 review.
- Failures preserve artifacts for diagnosis.

Subtasks:

- Add core invariants.
- Replace the scenario manifest and runner output.
- Add digest and metrics serialization.
- Document threshold ownership and change policy.

Verification:

- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0091.
- PSIM-0092.

Implementation notes:

- Added deterministic facade state digests and repeat-run equality coverage, expanded focused solver tests, and made the recovery hard gates explicit in the versioned fluid-quality manifest.
- Extended scenario output with penetration, non-finite, unexplained-lifecycle, seed, tick, threshold, and artifact evidence; the runner now retains and summarizes results as structured JSON.
- Verification on 2026-07-10: `test.ps1 -Tier Fast` passed 23/23 in 4.1 seconds; `verify-fluid-quality-suite.ps1` passed 18/18 balanced/quality runs in about 303 seconds; `check-tracking.ps1` passed after the evidence was recorded.

### PSIM-0094: Correct solver behavior against recovery gates

Status: Done

Priority: P0

Linked roadmap IDs: R15.05

Problem:
Current committed captures show sparse, repelling, over-spread water despite passing legacy thresholds.

Technical implementation direction:

- Correct failures in strict order: units/accounting, active cells/free surface, conservative transfer, pressure/boundaries, particle collision, density/resampling, viscosity/surface tension, then profile tuning.
- Do not relax hard gates or alter rendering to hide solver defects.
- Calibrate scenario-specific density, energy, jitter, and surface thresholds only from physically accepted runs, then freeze them in the manifest.
- Preserve the live 80x45, 1/120-second target with measured balanced and quality budgets.

Acceptance criteria:

- Balanced and quality profiles pass all hard recovery gates.
- U-container retains at least 99% of remaining mass after 10 seconds emission and 10 seconds settling with zero solid penetration.
- Still-pool energy decays and surface jitter remains within the accepted manifest threshold.
- Numeric behavior agrees with reviewed diagnostic captures.

Subtasks:

- Diagnose the first failing solver stage.
- Correct and verify one stage at a time.
- Calibrate/freeze scenario thresholds.
- Re-run full numeric and performance suites.

Verification:

- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1 -Profile All`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0093.

Implementation notes:

- Re-established full pressure-halo activation, tightened balanced/quality pressure targets, reused solver workspaces, and calibrated density correction, resampling, viscosity, transfer retention, and particle counts in solver order. Runtime APIC and surface tension remain disabled because the validated implementation does not yet justify exposing them as quality improvements.
- The recovery scenarios pass mass, momentum, pressure, penetration, finiteness, lifecycle, U-container retention, still-pool settling, density, and long-run gates for both balanced and quality profiles. Numeric acceptance is separate from the still-open PSIM-0095 visual/human gate.
- The 2026-07-10 contact-sheet review exposed that the prior U-container test emitted for only 1 second and ended after 6 seconds. The issue was reopened and the canonical test was corrected to the required 10-second emission plus 10-second settling interval with an explicit 99% remaining-mass containment gate; current visual settling remains under investigation.
- Corrected balanced and quality runs passed the 2400-tick containment, mass, pressure, determinism, and zero-penetration gates on 2026-07-10 (31.7 and 33.2 seconds including repeated executions). The long canonical suite is now a full-tier stress test so Standard retains representative solver coverage without violating its 90-second budget.
- Root-cause correction on 2026-07-10 removed the artificial pressure-cell air halo, applied the complete projected PIC/FLIP transfer instead of discarding 90% of it, and limited density-correction velocity injection. The corrected U scenario now also freezes pool height and settled kinetic-energy thresholds; balanced/quality ended at 27,961/12,844 energy with zero penetration and full containment.
- Aligned the Quality particle/resampling discretization and corrected the long-run density kernel calibration. `verify-fluid-quality-suite.ps1` passed the complete balanced/quality matrix in 111.1 seconds; `test.ps1 -Tier Standard` passed 27/27 in 24.9 seconds.
- Verification on 2026-07-10: `test.ps1 -Tier Standard` passed 25/25 in 78.2 seconds; `verify-fluid-quality-suite.ps1` passed 18/18 in about 303 seconds; `measure-water-solver.ps1 -Profile All` passed in Release with balanced and quality demo averages of 0.5028 ms and 0.6951 ms; `check-tracking.ps1` passed after tracking reconciliation.
- Final release audit found the clean all-profile Quality demo exceeded the frozen maximum-density gate. Quality now uses six density-correction iterations while preserving the 2.5% cell displacement cap per iteration; demo maximum density error fell from 4.285411 to 0.546378 without changing the 1.25 threshold. The final clean `verify-all.ps1` run passed all 35 tests, including canonical Quality coverage, on 2026-07-10.

## Epic 15: Water Presentation

### PSIM-0095: Reconstruct cohesive water and semantic visual gates

Status: Done

Priority: P0

Linked roadmap IDs: R16.01, R16.02, R16.04, R16.05

Problem:
Legacy surface rendering reproduces disconnected filled cells and uses hash equality as a proxy for quality.

Technical implementation direction:

- Reconstruct a deterministic continuous contour from the validated volume field using marching squares or equivalent connected geometry.
- Interpolate render snapshots only; never feed interpolation back into physics.
- Add semantic analysis for allowed region, connected components, pool width/depth, escaped water, and isolated cells.
- Generate contact sheets for numeric-passing scenes and require a dated human review before accepting new goldens.

Acceptance criteria:

- Basin and still-pool scenes pass semantic and numeric checks.
- Player view reads as connected liquid rather than isolated cells or square particles.
- Debug views remain available through lab mode.
- Golden changes record generation command, semantic results, reviewer, and date.

Subtasks:

- Add snapshot interpolation and contour extraction.
- Render filled surface and edge treatment.
- Implement semantic image/state analysis.
- Review contact sheets and accept new baselines.

Verification:

- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\verify-recovery-basin-surface.ps1`
- `.\scripts\check-tracking.ps1`
- Manual visual review of the recovery contact sheet.

Dependencies:

- PSIM-0094.

Implementation notes:

- Added deterministic thresholded marching-triangle surface reconstruction, read-only fixed-step interpolation, and SDL geometry rendering with a uniform connected fill.
- Added semantic state analysis for occupied extents, connected components, isolated cells, and water outside an allowed region, with focused synthetic basin coverage.
- A Release fixed-tick capture was generated at `build/windows-x64/release-surface-240.bmp` and inspected as implementation evidence. Human acceptance, canonical scene thresholds, contact sheets, and baseline replacement remain open; no golden was changed.
- Verification on 2026-07-10: `build.ps1` passed; `test.ps1 -Tier Fast` passed 24/24 in 3.4 seconds; `check-tracking.ps1` passed. The legacy Debug 2400-tick visual command exceeds 120 seconds after solver recovery and remains a PSIM-0098/0095 optimization target.
- Extracted a platform-independent `physics_sim_presentation` target so sandbox and lab use the exact same continuous reconstruction. Lab capture metadata now records component, isolated-cell, and escaped-region counts; smoke enforces the U-container and still-pool semantic gates.
- Added `capture-recovery-contact-sheet.ps1`, which produces fixed-tick continuous-surface scenario captures plus a review manifest without modifying goldens. The current manifest remains `Pending named human review`; visual tuning and acceptance are intentionally not self-certified.
- Replaced the misleading universal tick-60 sheet with scenario-specific accepted checkpoints (including U at 2400, still pool at 1200, and long run at 6000). The regenerated candidate passes semantic gates, but its review manifest remains pending and legacy goldens remain untouched.
- The project owner explicitly delegated the remaining acceptance decisions to Codex on 2026-07-10. The first exact regression candidates were rejected for overflow and fragmentation; active regression truth was then rebuilt around `starter_basin.pscene` plus the tick-indexed recovery replay.
- Accepted evidence is recorded in `docs/reviews/recovery-visual-acceptance-2026-07-10.md`. The three recovered basin baselines pass exact SHA-256 verification, canonical contact-sheet review includes solid geometry, and pre-recovery demo/deferred-emitter goldens are isolated under `regression/legacy/pre-recovery-2026-07-10`.

## Epic 16: Product Experiences

### PSIM-0096: Deliver narrow sandbox vertical slice

Status: Done

Priority: P0

Linked roadmap IDs: R17.01, R17.02, R17.03, R17.06

Problem:
The current player surface exposes broad nominal features before the pour/build/reset/save loop meets a coherent usability and presentation bar.

Technical implementation direction:

- Limit release sandbox navigation to pointer water, wall draw/erase, pause/step/resume, fluid reset, undo/redo, atomic save/load, lab switch, and menu return.
- Keep sandbox UI custom SDL; hide secondary devices, objectives, progression, gallery breadth, and decorative polish without deleting their history.
- Implement mouse-first interaction, keyboard alternatives, DPI/resize/fullscreen/focus handling, high contrast, reduced motion, remapped core controls, and audio fallback.
- Replace the dense help overlay with an interactive first-run pour/wall lesson.

Acceptance criteria:

- A fresh user completes the core loop without README or debug UI.
- Fresh/returning launch, keyboard-only use, resize/fullscreen, audio loss, invalid/interrupted saves, 15-minute play, and packaged launch pass the manual matrix.
- ImGui never appears in sandbox mode.
- Deferred features are unreachable from the recovery release UI.

Subtasks:

- Reduce sandbox mode and navigation.
- Rebuild player HUD/tool feedback and onboarding.
- Complete accessibility and failure recovery paths.
- Run and record the manual matrix.

Verification:

- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\run-smoke.ps1`
- Manual recovery sandbox acceptance matrix.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0092.
- PSIM-0095.

Implementation notes:

- Reduced the release-facing sandbox to pour, draw wall, erase wall, pause/step/reset, undo/redo, save/load, and sandbox/lab navigation. Tab cycling is constrained to those three tools; keys 3-9 and gallery paging now report that the feature is deferred.
- Replaced the broad main/pause navigation with recovery menus and added in-process sandbox/lab switching through the thin executable composition loop. Dear ImGui remains absent from the sandbox target.
- Rebuilt first-run guidance as a six-step interactive pour/draw/erase/pause/reset/save loop and removed device, objective, and gallery instructions from player help.
- Removed the engineering metrics overlay from ordinary sandbox launches; it is now explicitly opt-in with `--debug-overlay`, while player action feedback remains in the custom SDL UI.
- Verification on 2026-07-10: `build.ps1`, sandbox smoke, lab smoke, dependency validation, tracking validation, and `test.ps1 -Tier Fast` (25/25 in 3.3 seconds) passed. The full manual acceptance matrix and named usability review remain open.
- Corrected packaged relative-resource resolution after an arbitrary-working-directory launch exposed a missing tutorial. The executable now resolves bundled content from its own directory while preserving caller-relative paths that already exist.
- Owner-delegated acceptance is recorded in `docs/reviews/recovery-sandbox-acceptance-2026-07-10.md`. The Release package passed fresh/returning launch, pointer onboarding, fullscreen/window transition, arbitrary-working-directory launch, and an 1,129-second soak ending at 133,297 ticks with finite metrics and clean close. Focused input, menu, persistence, failure, accessibility, and audio tests cover the remainder of the matrix.

### PSIM-0097: Deliver Dear ImGui laboratory mode

Status: Done

Priority: P0

Linked roadmap IDs: R16.03, R17.04, R17.05

Problem:
Engineering controls and diagnostics are mixed into the player application and cannot launch, compare, capture, or inspect canonical scenarios as a coherent testbed.

Technical implementation direction:

- Add pinned Dear ImGui sources through vcpkg and keep all ImGui dependencies inside `physics_sim_lab`.
- Add `--mode lab` and panels for scenario/profile selection, run controls, solver parameters, metrics, plots, field views, replay, capture, and balanced/quality comparison.
- Track a run identity digest from scene, config, profile, and replay; deterministic parameter changes reset or explicitly fork the run.
- Capture image, metrics JSON, state digest, config, and tick as one artifact bundle.

Acceptance criteria:

- Every canonical scenario launches and runs without editing files.
- Balanced and quality runs can be compared from identical initial state.
- Forked/different runs visibly display identity divergence.
- Core physics contains no SDL or ImGui dependencies.

Subtasks:

- Add ImGui dependency and lab target.
- Implement scenario/run and diagnostics panels.
- Add plots, field views, comparison, and capture.
- Add lab smoke and deterministic identity tests.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Standard`
- Lab-mode smoke and capture replay.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0093.
- PSIM-0095.

Implementation notes:

- Added pinned Dear ImGui 1.92.8 through the vcpkg manifest and confined it to `physics_sim_lab`; core/content remain platform-independent and the SDL sandbox target has no ImGui dependency.
- Added `--mode lab` composition dispatch, SDL2-hosted ImGui rendering, all 10 canonical scenarios, run/pause/step/replay controls, gravity/timestep/emission controls, metrics, plots, complete particle/velocity/pressure/divergence/density/volume/solid views, matched balanced/quality comparison, and run identity/fork status.
- Added command-based particle seeding for deterministic initial states. Capture bundles record scenario, tick, digest, fixed timestep, gravity, profile, metrics JSON, and image.
- Reconciled Lab's duplicated scenario configurations with the validated canonical grids, emitters, obstacles, and checkpoints. Canonical Lab capture now exercises all 10 scenarios at their acceptance ticks; U-container, narrow-channel, and 6000-tick long-run semantic/energy gates prevent the previously observed floating or fragmented results.
- Verification on 2026-07-10: `build.ps1` passed with `/W4 /WX`; `run-lab-smoke.ps1 -SkipBuild` launched and captured all 10 scenarios; simulation API tests and dependency validation passed.

## Epic 17: Verification And Release

### PSIM-0098: Introduce tiered structured verification

Status: Done

Priority: P1

Linked roadmap IDs: R18.01, R18.02, R18.03

Problem:
The default test command takes about four minutes, mixes benchmarks with unit tests, and can delete the only failed visual capture.

Technical implementation direction:

- Add Fast, Standard, and Full script tiers with 30-second, 90-second, and 8-minute budgets; make verify-all the Full alias.
- Label CTest tests unit, integration, solver, visual, benchmark, smoke, or release and run safe independent tests in parallel.
- Remove benchmark/stress/long visual cases from Fast and preserve structured failure artifacts.
- Print scenario, profile, tick, seed, actual, threshold, duration, and artifact paths on failure.

Acceptance criteria:

- Fast, Standard, and Full tiers meet their budgets on the reference workstation.
- Developers can select tests by label.
- Failed artifacts remain available and successful temporary captures are cleaned.
- Full includes tracking, build, tests, smoke, solver, replay, visual, package, and hygiene checks.

Subtasks:

- Label CTest targets and parameterize scripts.
- Separate benchmarks and long scenarios.
- Standardize structured logs/artifacts.
- Measure and document each tier.

Verification:

- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\test.ps1 -Tier Full`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0093.

Implementation notes:

- Added Fast, Standard, and Full tiers, parallel label selection, Release prebuild for Full packaging, and CTest labels for unit, integration, solver, visual, benchmark, smoke, and release coverage.
- Fast passed 25/25 in 8.6 seconds and Standard passed 27/27 in 25.9 seconds on 2026-07-10 after the canonical 2400-tick suite moved to the explicit `stress` label.
- Full completed in 157.7 seconds on 2026-07-10: 30/33 tests passed, including the complete canonical solver suite, benchmark, package, smoke, and all non-visual coverage. The three expected legacy image-hash comparisons failed and preserved captures; their baselines remain unchanged pending PSIM-0095 named human acceptance.
- After solver and canonical Lab reconciliation, Fast passed 25/25 in 4.1 seconds and Full completed 31/34 in 71.8 seconds on 2026-07-10. The only failures remain the three intentionally stale visual hashes; Full now includes the 10-scenario acceptance-tick Lab matrix as a stress test.
- Fluid-quality, lab, and legacy visual failures now retain structured JSON/image/log artifacts; successful visual captures are cleaned only after their hashes match. The 2026-07-10 forced mismatch check preserved both the BMP and result JSON with baseline/capture hashes and artifact paths. This issue remains open until accepted recovered baselines make Full green.
- Registered the replay suite as a first-class visual CTest so Full cannot omit it. Quality benchmark failures now report actual average/maximum density errors and both limits.
- Final 2026-07-10 timing: Fast 25/25 in 4.1 seconds, Standard 27/27 in 25.7 seconds, and clean Full 35/35 in 76.0 seconds. All tiers are inside their 30/90/480-second budgets; exact recovered visuals, replay, package, smoke, canonical Lab, long-run solver, and structured failure contracts are part of Full.

### PSIM-0099: Add CI, hygiene, and prerelease packaging

Status: Done

Priority: P1

Linked roadmap IDs: R18.04, R18.05, R18.06

Problem:
The repository has no remote/CI and no current distributable, while hygiene covers only part of the intended release contract.

Technical implementation direction:

- Add Windows GitHub Actions for configure/build/tracking/Fast on pull requests, Standard on the default branch, and Full nightly/release.
- Add vcpkg caching and upload failure evidence without accepting cached regressions as truth.
- Enforce `/W4 /WX`, non-rewriting format checks, clang-tidy on compiled core, include direction, secrets, and absolute paths.
- Package version `0.2.0-alpha.1` with runtime dependencies, content, licenses, README, manifest, SHA-256 checksums, and verification logs; smoke from outside the build tree.

Acceptance criteria:

- CI definitions validate locally and run after a user-owned remote is configured.
- A clean CI checkout can build and run the applicable verification tier.
- The package has no build-tree dependency and passes packaged smoke.
- Release artifacts include checksums and evidence logs.

Subtasks:

- Add CI workflows and hygiene scripts/configuration.
- Harden warnings and static analysis.
- Rebuild package layout and manifest/checksum generation.
- Verify a clean packaged launch and document the external remote prerequisite.

Verification:

- `.\scripts\test.ps1 -Tier Full`
- `.\scripts\package-release.ps1`
- Packaged smoke from a temporary directory.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0096.
- PSIM-0097.
- PSIM-0098.

Implementation notes:

- Added Windows GitHub Actions for PR Fast, default-branch Standard, nightly/manual/tag Full, vcpkg caching, release packaging, and failure-evidence upload. Execution remains blocked on the external user-owned remote prerequisite.
- Enabled `/W4 /WX` for project targets and added non-rewriting clang-format, focused core clang-tidy, dependency direction, tracked secret, and absolute-path validation. `check-hygiene.ps1` passed on 2026-07-10.
- Rebuilt Release packaging as `0.2.0-alpha.1` with only the recovery basin/tutorial content, SDL runtime, ImGui/SDL licenses, version/commit manifest, per-file and ZIP SHA-256, verification evidence, and sandbox/lab smoke from the package directory.
- Verification on 2026-07-10: Release build and packaged sandbox/lab smoke passed in 83.6 seconds; archive SHA-256 was `46c5bfcaf2cc6da28bad48b88cf8cb73d8c5f655f931e1933e8ad7116de95fc6`. Final Full/CI evidence and remote protection remain open.
- Replaced machine-specific vcpkg configuration with `VCPKG_ROOT` plus a Visual Studio local fallback after the first clean GitHub runner exposed the absolute toolchain path. Fixed the versioned pre-push hook to accept Git's remote-name/URL arguments and added that invocation to the hook contract test.
- Hardened `verify-all.ps1` parameter/exit-code forwarding and made it the intended Full alias instead of redundantly rerunning every long suite. The final local bundle passed in 108.3 seconds and regenerated the package ZIP with SHA-256 `d1e0d9e1b2c139d54849eb3149e811b8853b68c24dcc01bc2aff0a7e85abdefa`.
- Created public repository `nicpadilla/physics-sim`, set `main` as the default, and protected it with strict required `verify` status, required pull requests, admin enforcement, conversation resolution, and force-push/deletion bans. Pull request #1 merged only after Windows run `29141676905` passed clean configure/build, tracking/hygiene, and Fast in 3m36s.
- Clean CI exposed and verified fixes for CRLF tracking input and excluding untracked dependency checkouts from repository path hygiene. The initial failing runs remain available as evidence rather than being hidden.
- Tag Full exposed that legacy `powershell.exe` on the runner lacked automatically loaded utility commands. Release scripts now explicitly import their standard module and CMake discovers `pwsh.exe` first with a Windows PowerShell fallback. Pull request #4 passed before the corrected tag was published.

## Epic 18: Selective Restoration

### PSIM-0100: Audit and gate deferred features

Status: Done

Priority: P2

Linked roadmap IDs: R19.01, R19.02, R19.03

Problem:
Legacy devices, objectives, progression, gallery breadth, tutorials, audio, and save-management features are nominally complete without recovery product decisions or acceptance.

Technical implementation direction:

- Review directional/omni emitters, gallery, devices, objectives, tutorial expansion, audio/animation, and broader save/progression in that order.
- For each, record restore, redesign, or remove based on sandbox/lab value.
- Restore only behind isolated APIs with automated state tests, persistence representation, performance budget, and human acceptance.
- Run the vertical-slice regression after every restored feature.

Acceptance criteria:

- Every deferred feature has a recorded disposition and rationale.
- Restored features meet the full recovery evidence gate.
- Removed features leave no unreachable UI or stale documentation.
- The accepted vertical slice remains unchanged unless an intentional reviewed issue expands it.

Subtasks:

- Produce the feature disposition table.
- Create separate restoration issues for accepted features.
- Remove rejected runtime/docs paths.
- Re-run vertical-slice acceptance.

Verification:

- `.\scripts\test.ps1 -Tier Standard`
- Manual vertical-slice regression.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0099.

Implementation notes:

- Recorded explicit redesign, defer, or removal decisions for emitters, gallery, devices, objectives, tutorial expansion, decorative polish, and broader saves/progression in `docs/deferred-feature-disposition.md`.
- The package contains only the starter basin and narrow tutorial; sandbox menus/cycling/shortcuts cannot reach legacy devices or gallery breadth. No deferred feature was restored, so no speculative restoration issue was opened.
- Verification on 2026-07-10: `test.ps1 -Tier Standard` passed 27/27 in 64.1 seconds, sandbox and lab smoke passed, package-directory smoke passed, and `check-tracking.ps1` passed. Named vertical-slice human acceptance remains correctly owned by PSIM-0095/0096 rather than this disposition audit.

## Epic 19: Recovery Acceptance

### PSIM-0101: Complete recovery release acceptance

Status: Done

Priority: P0

Linked roadmap IDs: R19.04

Problem:
The recovery needs a final evidence reconciliation that cannot be satisfied by feature existence or stale issue notes.

Technical implementation direction:

- Audit every recovery dependency, roadmap row, ADR, threshold, capture, manual review, package, and release note against current commit evidence.
- Run Full verification, package smoke, sandbox manual matrix, lab scenario/capture matrix, and final visual review.
- Close only after no P0/P1 recovery issue remains open and CI/package evidence agrees with local evidence.

Acceptance criteria:

- All recovery dependencies are Done with dated implementation and verification notes.
- Every active roadmap marker is Automated or Human Accepted as appropriate.
- Full verification completes within eight minutes and the prerelease package launches cleanly.
- Release notes state remaining limitations and deferred features honestly.

Subtasks:

- Reconcile tracking and evidence.
- Run final automated and manual acceptance.
- Produce release notes and package.
- Commit/tag the accepted recovery release.

Verification:

- `.\scripts\verify-all.ps1`
- Packaged smoke from a clean temporary directory.
- Sandbox and lab manual acceptance matrices.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0094.
- PSIM-0095.
- PSIM-0096.
- PSIM-0097.
- PSIM-0098.
- PSIM-0099.
- PSIM-0100.

Implementation notes:

- Reconciled the recovery dependencies: PSIM-0094 through PSIM-0100 are Done; every active roadmap marker except this final release marker is Automated or Human Accepted.
- `verify-all.ps1` passed locally in 108.3 seconds with Full 35/35 in 76.0 seconds; the Release package ZIP SHA-256 is `d1e0d9e1b2c139d54849eb3149e811b8853b68c24dcc01bc2aff0a7e85abdefa`.
- Owner-delegated sandbox acceptance and visual acceptance are recorded in the dated review files. Protected pull requests and required Windows CI passed.
- Annotated tag `v0.2.0-alpha.1` resolves to protected main commit `279a5fb726457a4ed58a249939bef21dc500ce96`. Tag workflow `29142733405` passed clean build, tracking/hygiene, Full, explicit prerelease packaging, and artifact upload in 9m10s. The explicit package-step ZIP SHA-256 is `6dd48e2e7155636657ed55ca0d556bf6201c6f9ecb671a31447ab536745de3a9`.
- No P0 or P1 recovery issue remains open. Deferred breadth remains governed by PSIM-0100 rather than being misrepresented as part of this alpha.

## Epic 20: Convincing Water Motion

### PSIM-0102: Add deterministic water-feel validation

Status: Done

Priority: P0

Linked roadmap IDs: R20.01

Problem:
Current regression gates prove containment, conservation, stability, and repeatability but can accept mound-like, overdamped water that does not flow or settle convincingly.

Technical implementation direction:

- Add deterministic scenarios and structured metrics under the existing solver-test and capture infrastructure for asymmetric leveling, fixed-rate pouring, sloshing, wall-sheet flow, two-stream merging, and obstacle breakup/rejoin.
- Extend `SimulationMetrics` only where a generally useful core quantity is missing; derive scenario-specific surface slope, footprint, component lifetime, stream-width variation, vorticity, and particle-distribution measures in validation code.
- Record scenario, profile, seed, tick, actual value, threshold, and artifact path on failure. Preserve existing hard conservation, pressure, penetration, finiteness, and determinism gates.
- Establish provisional thresholds from the current baseline and physically acceptable candidate runs. Threshold promotion requires recorded capture review; do not tune the solver or regenerate accepted goldens in this issue.
- Add real-time or fixed-frame-sequence evidence for motion review rather than relying exclusively on a final still image.

Acceptance criteria:

- Fast tests cover the metric algorithms with small deterministic fixtures.
- Standard verification runs representative leveling and pour-feel scenarios with structured output.
- Full verification covers every new scenario in balanced and quality profiles within the existing eight-minute budget.
- A baseline report identifies which feel gates the current solver fails without weakening existing numeric contracts.

Subtasks:

- Define manifests and metric schemas.
- Implement deterministic scenario runners and metric tests.
- Capture current balanced/quality baselines and frame sequences.
- Record provisional calibration ranges and observed failures.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0093 and PSIM-0094 are completed foundations.

Implementation notes:

- Added compiled `water_feel_metrics` core measurement for horizontal footprint, occupied columns, surface RMS/maximum slope, particle-count coefficient of variation, eight-neighbor particle support components, largest-component fraction, and grid vorticity RMS.
- Added deterministic unit fixtures for level, stepped, separated, uniformly sampled, and rotating fields and integrated every metric into repeated scenario snapshot comparison and non-finite checks.
- Added asymmetric leveling, steady pour, slosh decay, wall sheet, two-stream merge, and obstacle breakup/rejoin scenarios. Standard includes representative leveling and pour cases; the structured suite covers every new case in balanced and quality.
- Extended fluid-quality logs with checkpoint `sample=` records and the JSON summary with all new final metrics and artifact paths.
- Recorded pre-tuning measurements, provisional calibration ranges, and classified numerical, sampling, rendering, and acceptance debt in `docs/water-feel-baseline-2026-07-11.md`.
- Verification on 2026-07-11: `.\scripts\build.ps1` passed; Fast passed 26/26 in 4.277 seconds; Standard passed 28/28 in 26.450 seconds; `.\scripts\verify-fluid-quality-suite.ps1` passed all 30 balanced/quality runs in 278.7 seconds with zero hard-gate failures.

### PSIM-0103: Build an area-faithful particle free surface

Status: Done

Priority: P0

Linked roadmap IDs: R20.02

Problem:
Assigning each particle volume to one cell and averaging cell fractions at grid vertices expands tiny volumes, bridges holes, and produces coarse polygonal blobs.

Technical implementation direction:

- Introduce a deterministic compact-kernel particle volume rasterization that distributes each particle's volume to neighboring cells with normalized weights and stable iteration order.
- Add a narrow-band particle signed-distance or equivalent sub-cell scalar field for player rendering; keep lab volume-fraction and particle views truthful and independently selectable.
- Reconstruct the player surface at configurable higher sampling resolution without mutating solver state. Preserve total represented area within an explicit tolerance and clip against solids without leaks.
- Add fixtures for isolated droplets, adjacent particles, shallow pools, thin streams, solid boundaries, translation continuity, and deterministic triangle output.
- Do not use rendering dilation, minimum blob sizes, or temporal accumulation to conceal missing physical volume.

Acceptance criteria:

- A one-particle droplet renders within 15% of its conserved area and changes continuously under sub-cell translation.
- Settled-pool reconstruction contains no internal holes larger than the configured particle support while preserving occupied area within 2%.
- Thin streams remain connected when supported by particles and separate when the underlying particle support separates.
- Existing state digests remain physics-only and deterministic; Fast, Standard, smoke, and recovery visual checks pass before reviewed golden replacement.

Subtasks:

- Add kernel volume rasterization and area accounting tests.
- Add the player-facing sub-cell surface field and reconstruction.
- Integrate interpolation and solid clipping.
- Capture semantic comparisons without replacing accepted goldens prematurely.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-recovery-basin.ps1`

Dependencies:

- PSIM-0102 supplies area, continuity, and motion evidence.

Implementation notes:

- Added deterministic normalized bilinear particle-volume rasterization for smooth diagnostics while retaining center-occupied pressure topology so low-volume tails do not become full pressure cells.
- Added a four-times-resolution quadratic particle surface field with automatic area-calibrated contour selection, solid-aware visibility, sub-cell translation, and explicit particle/reconstructed area metrics.
- Sandbox rendering now interpolates the high-resolution field and draws continuous depth-colored water with a restrained top-surface highlight; lab captures use the same truthful particle-derived surface.
- Added isolated-droplet, area, translation, shallow-pool, solid-wall, and deterministic geometry tests. The accepted U-container surface is one component, has no isolated/escaped cells, and reconstructs within about `0.045%` of particle area.
- Replaced the three basin baselines only after numeric and named visual review. Full verification passed 36/36 in 169.6 seconds on 2026-07-11.

### PSIM-0104: Restore believable transfer and free-surface flow

Status: Done

Priority: P0

Linked roadmap IDs: R20.03

Problem:
The balanced and quality profiles lose shear and lateral motion too quickly, while coarse partial-cell pressure behavior permits stable stair-stepped mounds.

Technical implementation direction:

- Exercise the existing affine particle state through a measured APIC/FLIP transfer path with deterministic clamps and explicit energy diagnostics; tune profile values only against the new feel and invariant scenarios.
- Reduce PIC dissipation incrementally while bounding grid-scale velocity noise, kinetic-energy growth, divergence, and repeated-digest equality.
- Make pressure activation and free-surface gradients volume-aware so partial cells cannot support artificial hydrostatic steps; validate changes first on small known grids and leveling/hydrostatic scenarios.
- Keep timestep, gravity, and units explicit. Do not compensate for pressure or transfer failures through density correction, viscosity, rendering, or relaxed accounting thresholds.

Acceptance criteria:

- An asymmetric pile spreads toward a level free surface with monotonically decreasing fitted surface slope after the pour stops.
- Pour, slosh, wall-impact, and obstacle scenarios retain visibly useful shear and lateral motion without non-finite values, penetration, unexplained mass, or unbounded energy.
- Balanced and quality retain their pressure residual, conservation, determinism, performance, and long-run gates.
- Profile changes and calibration evidence are recorded in the scenario manifest and review artifacts.

Subtasks:

- Add APIC transfer coverage and energy/vorticity diagnostics.
- Calibrate transfer blends in bounded sweeps.
- Implement and validate volume-aware free-surface pressure treatment.
- Re-run the complete canonical and feel scenario matrices.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Full`
- `.\scripts\measure-water-solver.ps1`

Dependencies:

- PSIM-0102 metrics are required; PSIM-0103 may proceed in parallel but reviewed acceptance requires both.

Implementation notes:

- Corrected gravity integration to convert world acceleration by `cell_size`, fixing the 16-times-too-slow sandbox-scale fall while preserving cell-relative behavior across grid scales with a focused regression test.
- Calibrated balanced to FLIP `0.78` / APIC `0.10` and quality to FLIP `0.71` / APIC `0.10` through bounded scenario sweeps; added command-line calibration overrides for repeatable experiments.
- Kept pressure activation center-occupied while exposing conservatively smoothed volume, avoiding artificial pressure support from raster tails.
- The balanced asymmetric-leveling RMS slope improved from `0.365` to `0.117`; the settled U-container spreads roughly 34 cells with zero penetration, mass error, or non-finite values.
- `verify-fluid-quality-suite.ps1` passed all 30 balanced/quality runs in 299.1 seconds; Full passed the profile matrix, 6000-tick stress, benchmark, determinism, and package gates on 2026-07-11.

### PSIM-0105: Regularize particles and remove artificial cohesion

Status: Done

Priority: P1

Linked roadmap IDs: R20.04

Problem:
Per-cell split/merge and over-density-only position correction allow sparse holes and clumps, while the dormant surface-force path contains nonlocal center attraction that is incompatible with water.

Technical implementation direction:

- Remove global center-of-mass cohesion from the surface-force implementation; any restored surface tension must be a local curvature force derived from the validated free-surface field.
- Add conservative neighborhood-aware particle shifting or redistribution with deterministic neighbor order and momentum-consistent velocity handling.
- Extend resampling decisions across adjacent fluid cells while preserving mass, linear momentum, bounded angular-momentum error, center of mass, material identity, and lifecycle accounting.
- Calibrate viscosity in explicit simulation units and introduce local surface tension only after transfer, pressure, and sampling gates pass; default coefficients must remain subtle at basin scale.

Acceptance criteria:

- Particle-distribution variation and unsupported surface holes decrease without measurable center-of-mass drift or unexplained lifecycle changes.
- No force depends on the global fluid center of mass.
- Droplets round locally while basin-scale water still spreads and levels; steady streams do not collapse into ropes or beads.
- All invariant, feel, long-run, determinism, and performance gates pass in balanced and quality.

Subtasks:

- Remove and test against nonlocal cohesion.
- Add deterministic particle-distribution metrics and regularization.
- Improve neighborhood-aware resampling.
- Calibrate unit-aware viscosity and optional local surface tension.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Full`
- `.\scripts\verify-recovery-basin.ps1`

Dependencies:

- PSIM-0103 and PSIM-0104.

Implementation notes:

- Removed the nonlocal center-of-mass surface-tension attraction path. Default surface tension remains zero; any future restoration is restricted to the local curvature force.
- Raised the balanced/quality minimum local sampling target to two particles per occupied fluid cell while keeping split/merge mass, momentum, volume, lifecycle, and bounded-operation contracts.
- Reworked focused viscosity, surface-force, drain, and finite-wall tests to assert physical invariants rather than fragile particle counts or an invalid global-contraction expectation.
- Sampling CV improved to `0.325` for balanced slosh and `0.303` for quality; obstacle largest-component fraction improved from `0.765` to `0.895` in balanced without recursive splitting or particle-count explosion.
- All invariant, feel, long-run, determinism, recovery visual, and Full gates passed on 2026-07-11. Remaining active-pour sampling variation and small separated impact droplets are documented limitations, not hidden by rendering cohesion.

### PSIM-0106: Tune, review, and accept real-time water feel

Status: Done

Priority: P0

Linked roadmap IDs: R20.05

Problem:
The recovered release was accepted from stable still captures but has not passed a real-time review of pouring, spreading, grouping, splashing, sloshing, merging, and settling.

Technical implementation direction:

- Tune only within the validated solver and rendering controls delivered by PSIM-0102 through PSIM-0105; every candidate must retain hard numeric gates.
- Capture fixed-camera frame sequences and real-time video-equivalent contact sheets for standard sandbox actions in balanced and quality profiles.
- Add restrained normals, highlights, and temporal rendering stability only where they improve legibility without changing represented area or hiding discontinuities.
- Require a named review that evaluates stream weight, lateral spread, surface leveling, grouping strength, splash scale, wall behavior, slosh decay, tiny droplets, and fifteen-minute interactive stability.
- Replace goldens only after numeric and semantic gates pass and the review records exact artifacts and hashes.

Acceptance criteria:

- A user can pour, redirect, pool, split, merge, slosh, and settle water without it reading as gel, sand, smoke, or disconnected grid cells.
- Balanced is responsive and convincing in the sandbox; quality is at least as credible in lab comparisons.
- Fast, Standard, Full, smoke, recovery-basin, package-directory, and tracking verification pass within established budgets.
- Named human acceptance records exact build, profile, scenarios, duration, artifacts, observed limitations, and result.

Subtasks:

- Run bounded candidate sweeps and select validated defaults.
- Add final water rendering legibility changes.
- Capture and review the complete motion matrix.
- Regenerate accepted baselines, package, release notes, and launch the verified build.

Verification:

- `.\scripts\verify-all.ps1`
- `.\scripts\verify-recovery-basin.ps1`
- `.\scripts\capture-recovery-contact-sheet.ps1 -Tick 2400`
- Packaged sandbox and lab manual acceptance.
- `.\scripts\check-tracking.ps1`

Dependencies:

- PSIM-0102, PSIM-0103, PSIM-0104, and PSIM-0105.

Implementation notes:

- Numeric, semantic, canonical-capture, visual-baseline, replay, package, and named review gates pass. Evidence and accepted limitations are recorded in `docs/reviews/water-feel-acceptance-2026-07-11.md`.
- Fixed large-surface lab corruption by advertising the renderer's `ImDrawCmd::VtxOffset` support; automated capture now advances only the selected run, reducing the 10-scenario canonical matrix to 18.4 seconds and eliminating the prior parallel timeout.
- `verify-all.ps1` passed 36/36 plus tracking, hygiene, packaging, replay, and visual verification in 169.6 seconds on 2026-07-11.
- A populated balanced sandbox replay soak ran from 10:27:06 through 10:42:06 on 2026-07-11, emitted through tick 1200, continuously rendered the settled surface, saved settings, and exited normally at the configured 900 seconds. Log: `build/windows-x64/water-feel-soak.log`.

## Epic 21: Complete Water Sandbox

### PSIM-0107: Regularize active pours and refine droplets

Status: Done

Priority: P0

Linked roadmap IDs: R21.01

Problem:
Active pours still have uneven per-cell sampling and energetic interactions can leave tiny unsupported fragments that read as solver debris rather than water.

Technical implementation direction:

- Extend `WaterSimulation2D` regularization with a deterministic neighborhood pass after collision/density correction and before metrics collection. Use stable cell and particle ordering, compact support, zero-net correction, and a per-step displacement cap expressed in cell units.
- Improve split/merge selection across immediate fluid neighbors without teleporting isolated particles. Any merge must conserve mass, volume, linear momentum, and center of mass; physically separated supported droplets remain valid.
- Add particle-support age or equivalent deterministic evidence only if needed to distinguish transient under-sampling from stable droplets; include it in state digests and commands only if it becomes solver state.
- Promote active-pour sampling, unsupported-fragment lifetime, center-of-mass drift, angular error, particle-count growth, and performance thresholds in the feel suite. Do not relax mass, pressure, penetration, determinism, or settled-energy gates.

Acceptance criteria:

- Settled-pool and slosh sampling CV are `<= 0.35`; steady-pour CV is `<= 0.40` in balanced and quality without particle-count explosion.
- Unsupported one-particle fragments created by standard wall/obstacle impacts rejoin or remain physically supported within 120 ticks; supported separated droplets are preserved.
- Mass error stays `<= 1e-5`, force-free momentum error `<= 1e-4`, center-of-mass drift from regularization `<= 1e-5` cells, particles in solids and unexplained lifecycle changes remain zero.
- Fast, Standard, the 30-run matrix, 6000-tick stress, and deterministic repeated digests pass within existing budgets.

Subtasks:

- Add regularization and fragment-lifetime metrics/tests.
- Implement conservative neighborhood shifting and neighbor-aware split/merge.
- Run bounded parameter sweeps and record selected defaults.
- Capture before/after active-pour and impact evidence.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\verify-fluid-quality-suite.ps1`
- `.\scripts\measure-water-solver.ps1 -Profile All`

Dependencies:

- PSIM-0102 through PSIM-0106 provide the invariant, feel, rendering, and review baseline.

Implementation notes:

- Added deterministic mass-weighted pair regularization with compact support, one globally capped correction field, unchanged velocities, solid projection, configurable cadence, and an eight-particle small-component guard so supported droplets are not cosmetically merged.
- Balanced uses one iteration every four ticks at support `0.50` and max displacement `0.010` cells; quality uses two iterations every tick at support `0.45` and the same cap. Both use strength `0.25`.
- Added focused control-vs-regularized separation, mass, momentum, and center-of-mass tests plus calibration-only CLI overrides for iterations, support, strength, displacement, and cadence.
- Promoted leveling/slosh CV `<= 0.35`, steady-pour CV `<= 0.40`, obstacle main-component fraction `>= 0.90`, and remaining components `<= 3` into scenario assertions without weakening existing hard gates.
- Rejected per-tick balanced shifting after it exceeded the U-container energy gate; selected cadence passes U energy at `11,554` balanced and `10,408` quality. Full calibration is in `docs/particle-regularization-2026-07-11.md`.
- Verification on 2026-07-11: build passed; Fast 26/26 in 4.164 seconds; Standard 28/28 in 33.83 seconds; all 30 fluid-quality cases passed; all solver benchmark profiles passed (`0.4302 ms/step` balanced stress, `1.6074 ms/step` quality stress).

### PSIM-0108: Add deterministic foam and spray presentation

Status: Open

Priority: P1

Linked roadmap IDs: R21.02

Problem:
The continuous surface reads as water but lacks small-scale impact, aeration, and breakup cues that communicate speed and energy.

Technical implementation direction:

- Add a compiled presentation-only `WaterVisualEffects` builder derived from `SimulationSnapshot`, the reconstructed field, velocity, vorticity, curvature, connectivity, and the deterministic tick. It must never mutate solver state or enter physics digests.
- Generate bounded foam bands at exposed high-curvature/high-vorticity surface samples, spray points only from fast small supported components above the main body, and short impact accents near solid contacts. Use stable spatial hashing/tick phases rather than frame-time randomness.
- Render effects through SDL geometry with fixed maximum counts and explicit color/alpha budgets. Reduced motion freezes phase and disables ballistic accent motion while preserving static legibility; high contrast remains readable.
- Add semantic counts/bounds, deterministic hashes, performance tests, fixed-tick captures, and named human review before changing visual baselines.

Acceptance criteria:

- Identical snapshot/tick/settings produce identical effect primitives and hashes; interpolation/frame rate do not alter selection.
- Still pools produce no spray and minimal foam; wall impacts, pours, and obstacle breakup produce bounded visible accents with zero effects inside solids or outside the domain.
- Effects add no represented water area or physics mass and stay below 1.5 ms CPU presentation time at the 80x45 target with at most 512 foam and 256 spray primitives.
- Reduced-motion, high-contrast, sandbox, lab, semantic, and named visual acceptance pass.

Subtasks:

- Implement deterministic effect extraction and unit fixtures.
- Integrate sandbox/lab rendering and accessibility settings.
- Add capture metrics and performance budgets.
- Review and accept new visual baselines.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\run-smoke.ps1`
- `.\scripts\capture-recovery-contact-sheet.ps1`
- `.\scripts\verify-recovery-basin.ps1`

Dependencies:

- PSIM-0107 must settle final particle/component behavior before visual acceptance.

Implementation notes:

- None yet.

### PSIM-0109: Restore advanced water tools and devices

Status: Open

Priority: P0

Linked roadmap IDs: R21.03

Problem:
Directional/omni emitters, gates, sensors, drains, pumps, and valves are implemented but intentionally unreachable in the recovered sandbox.

Technical implementation direction:

- Restore tools `3` through `9` behind an explicit keyboard-accessible Advanced Tools palette in the custom SDL sandbox; keep pour/draw/erase as the default compact palette and keep ImGui out of sandbox mode.
- Route every placement, selection, toggle, parameter edit, undo/redo, replay, and load through existing `SimulationCommand`/`SceneController` boundaries. Add missing commands rather than mutating solver internals from UI.
- Re-enable scene v2 persistence and replay v2 coverage for each device family, with malformed bounds/parameter validation and deterministic ordering. Keep player saves separate from bundled scenes.
- Provide visible selection, enabled/open state, direction/strength feedback, keyboard alternatives, short help text, and per-device performance/state tests. Package only scenes accepted under PSIM-0111.

Acceptance criteria:

- A keyboard-only user can place, select, configure, toggle, undo, redo, save, reload, and replay every advanced tool without debug overlays.
- Device state round-trips exactly; malformed device records fail safely; identical command replays produce identical state digests and metrics.
- Drain/outflow accounting remains explicit, pumps/valves/gates do not create mass, sensors do not mutate physics, and all existing canonical device scenarios pass.
- Advanced tools add less than 10% median solver cost in their accepted scenes and do not regress the core compact palette or first-run tutorial.

Subtasks:

- Restore advanced palette and tool feedback.
- Complete command/undo/replay/persistence contracts.
- Add device-family UI and malformed-input tests.
- Run keyboard, save/load, performance, and named usability review.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- `.\scripts\run-smoke.ps1`
- `.\scripts\verify-fluid-quality-suite.ps1`

Dependencies:

- PSIM-0107 is required for final water/device captures; implementation may begin after its solver defaults are stable.

Implementation notes:

- None yet.

### PSIM-0110: Restore challenge objectives

Status: Open

Priority: P1

Linked roadmap IDs: R21.04

Problem:
Objective sensors and completion metrics exist, but the release has no intentional challenge contract or player flow.

Technical implementation direction:

- Define explicit scene-v2 challenge metadata: objective title, required objective-sensor count, hold duration in ticks, optional maximum emitted/outflow mass, and completion state that is runtime-only unless a separate profile progression record is justified.
- Drive progress exclusively from sensor readings and tick-indexed commands; objective logic must not mutate solver state or depend on frame timing. Expose progress, restart, completion, and failure feedback in custom SDL UI.
- Create three bounded challenges teaching redirection, gating, and pump/valve control. Each must remain solvable through keyboard-accessible tools and replayable deterministically.
- Keep challenge selection optional; creative sandbox remains unconstrained and never displays hidden objectives.

Acceptance criteria:

- Three curated challenges have deterministic success replays, visible progress/criteria, restart, and completion feedback.
- Objective state round-trips where scene-authored, runtime completion resets predictably, and repeated replays complete on the same tick with identical digests/metrics.
- No objective changes mass, forces, devices, or solver ordering; missing/malformed criteria fail safely.
- Named usability review confirms criteria are understandable without README or debug overlays.

Subtasks:

- Extend challenge metadata/parser/validation.
- Implement deterministic objective evaluator and UI.
- Author three challenge scenes and success replays.
- Add automated and named player acceptance.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- Challenge replay verification script.
- Packaged keyboard-only manual matrix.

Dependencies:

- PSIM-0109 supplies the restored device interactions.

Implementation notes:

- None yet.

### PSIM-0111: Ship the curated scene gallery

Status: Open

Priority: P1

Linked roadmap IDs: R21.05

Problem:
Gallery code and legacy thumbnails exist, but the release lacks a reviewed, coherent set of creative and challenge scenes.

Technical implementation direction:

- Replace broad filesystem exposure with a versioned curated manifest containing stable ID, title, description, category, scene path, thumbnail path, required feature set, and sort order.
- Restore custom SDL gallery navigation with categories for Learn, Sandbox, and Challenges, keyboard/pointer parity, load-error recovery, current-scene confirmation, and no build-tree paths.
- Review/regenerate scenes as v2 and deterministic thumbnails using current rendering. Include starter/tutorial, three creative advanced-device scenes, and the three PSIM-0110 challenges; exclude legacy scenes that fail current quality gates.
- Package the manifest, scenes, thumbnails, replay evidence, and licenses. Add manifest completeness, duplicate-ID, missing-file, and clean-directory tests.

Acceptance criteria:

- At least eight reviewed scenes are discoverable with valid thumbnails and useful descriptions; every entry loads from the package directory.
- Keyboard-only navigation, category filtering, load confirmation, malformed/missing entry recovery, and return to the current session pass.
- Gallery scenes meet numeric/semantic gates appropriate to their devices/objectives and do not expose unreviewed legacy content.
- Named review accepts ordering, thumbnails, readability, and product coherence.

Subtasks:

- Add curated manifest contract and validation.
- Restore gallery UI and navigation.
- Review/author scenes and regenerate thumbnails.
- Verify and package the complete gallery.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\test.ps1 -Tier Standard`
- Gallery capture/validation script.
- `.\scripts\package-release.ps1`

Dependencies:

- PSIM-0109 and PSIM-0110 define the accepted feature and challenge content.

Implementation notes:

- None yet.

### PSIM-0112: Polish water and device audio

Status: Open

Priority: P1

Linked roadmap IDs: R21.06

Problem:
Short functional synthesized cues exist, but water motion and restored devices lack layered, continuous, and reviewed audio feedback.

Technical implementation direction:

- Extend the compiled audio model with bounded loop layers for pour, flow, impact, pump, drain, gate/valve, and objective ambience derived from smoothed simulation metrics, never frame-random state.
- Use deterministic procedural synthesis or repository-owned assets with documented licenses. Mix through a fixed-size callback/queue path with attack/release envelopes, voice caps, soft limiting, master/effects/music controls, and zero allocation in the audio callback.
- Handle lost/reopened devices without crashing or blocking simulation. Mute produces silence immediately; reduced motion must not disable necessary audio feedback, and all interactions retain visual equivalents.
- Add waveform, clipping, envelope, transition, mute/volume, device-loss, CPU, and named listening-review evidence.

Acceptance criteria:

- Pour/flow/impact/device layers respond within 100 ms, release without clicks, remain below `-1 dBFS`, and never exceed eight simultaneous voices.
- Muting and zero volume produce exact silence; missing/lost devices degrade safely and can recover without restarting the simulation.
- Audio processing remains below 1% CPU at 48 kHz on the recorded recovery environment and does not change simulation determinism.
- Named review accepts clarity, restraint, repetition, balance, and accessibility across sandbox/challenge/gallery flows.

Subtasks:

- Implement continuous layer synthesis/mixer state.
- Connect deterministic simulation/device signals.
- Add resilience, performance, and waveform tests.
- Run named listening and packaged-device review.

Verification:

- `.\scripts\build.ps1`
- `.\scripts\test.ps1 -Tier Fast`
- `.\scripts\run-smoke.ps1`
- Audio render/performance capture.
- Packaged lost-device manual matrix.

Dependencies:

- PSIM-0109 supplies final device signals; PSIM-0110 supplies objective transitions.

Implementation notes:

- None yet.

### PSIM-0113: Accept and publish the complete prerelease

Status: Open

Priority: P0

Linked roadmap IDs: R21.07, R21.08

Problem:
The complete feature set needs integrated acceptance and a protected downloadable release rather than isolated local implementation.

Technical implementation direction:

- Reconcile PSIM-0107 through PSIM-0112, update README/limitations/disposition/release notes, choose the next semantic prerelease version, and ensure no P0/P1 issue remains incomplete.
- Extend Full verification for visual effects, advanced tools, challenges, curated gallery, audio, package launch, and failed-artifact retention while keeping the eight-minute target or documenting/approving a tier split before release.
- Run named live review covering core pour/build, every advanced tool, all challenges, gallery navigation, audio/mute/lost-device, reduced motion, high contrast, keyboard-only use, and a populated fifteen-minute soak.
- Commit focused completed issues, push through protected `main` via the repository's accepted workflow, create an annotated prerelease tag, wait for tag CI, publish/check the GitHub release and downloadable ZIP, verify checksums, then launch that downloaded-equivalent package locally.

Acceptance criteria:

- All linked roadmap rows are Automated or Human Accepted with current dated evidence and no open P0/P1 issue.
- Fast/Standard/Full, tracking, hygiene, replay/visual/challenge/gallery/audio/package checks, and clean-directory launch pass within recorded budgets.
- Protected GitHub CI succeeds for the accepted commit and tag; the published ZIP checksum matches the verified local/downloaded artifact and contains the full curated product.
- A fresh user can complete core sandbox, one advanced device workflow, one challenge, gallery navigation, mute, save/load, and return to sandbox without external instructions.

Subtasks:

- Complete integrated automated and human acceptance.
- Update versioning, package manifest, docs, and release notes.
- Push protected commit and annotated prerelease tag.
- Verify CI, GitHub release/artifact, checksum, clean launch, and final local game window.

Verification:

- `.\scripts\verify-all.ps1`
- Integrated packaged acceptance matrix and 900-second soak.
- `.\scripts\check-tracking.ps1`
- GitHub branch/tag workflow and release artifact inspection.

Dependencies:

- PSIM-0107, PSIM-0108, PSIM-0109, PSIM-0110, PSIM-0111, and PSIM-0112 must be Done.

Implementation notes:

- None yet.
