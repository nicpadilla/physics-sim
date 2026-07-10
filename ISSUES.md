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
| PSIM-0094 | In Progress | P0 | Validated Water | R15.05 | Correct solver behavior against recovery gates |
| PSIM-0095 | In Progress | P0 | Water Presentation | R16.01, R16.02, R16.04, R16.05 | Reconstruct cohesive water and semantic visual gates |
| PSIM-0096 | In Progress | P0 | Product Experiences | R17.01, R17.02, R17.03, R17.06 | Deliver narrow sandbox vertical slice |
| PSIM-0097 | Done | P0 | Product Experiences | R16.03, R17.04, R17.05 | Deliver Dear ImGui laboratory mode |
| PSIM-0098 | In Progress | P1 | Verification And Release | R18.01, R18.02, R18.03 | Introduce tiered structured verification |
| PSIM-0099 | In Progress | P1 | Verification And Release | R18.04, R18.05, R18.06 | Add CI, hygiene, and prerelease packaging |
| PSIM-0100 | Done | P2 | Selective Restoration | R19.01, R19.02, R19.03 | Audit and gate deferred features |
| PSIM-0101 | Open | P0 | Recovery Acceptance | R19.04 | Complete recovery release acceptance |

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

Status: In Progress

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
- Verification on 2026-07-10: `test.ps1 -Tier Standard` passed 25/25 in 78.2 seconds; `verify-fluid-quality-suite.ps1` passed 18/18 in about 303 seconds; `measure-water-solver.ps1 -Profile All` passed in Release with balanced and quality demo averages of 0.5028 ms and 0.6951 ms; `check-tracking.ps1` passed after tracking reconciliation.

## Epic 15: Water Presentation

### PSIM-0095: Reconstruct cohesive water and semantic visual gates

Status: In Progress

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
- `.\scripts\verify-demo-scene-surface.ps1`
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

## Epic 16: Product Experiences

### PSIM-0096: Deliver narrow sandbox vertical slice

Status: In Progress

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
- Verification on 2026-07-10: `build.ps1` passed with `/W4 /WX`; `run-lab-smoke.ps1 -SkipBuild` launched and captured all 10 scenarios; simulation API tests and dependency validation passed.

## Epic 17: Verification And Release

### PSIM-0098: Introduce tiered structured verification

Status: In Progress

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
- Fast passed 25/25 in 3.3 seconds and Standard passed 25/25 in 78.2 seconds on 2026-07-10. Full is intentionally still open because recovered visual/replay baselines have not passed numeric-plus-human acceptance.
- Fluid-quality and lab failures now retain structured JSON/image/log artifacts. Legacy visual/replay scripts still require uniform result JSON and success-only cleanup before this issue can close.

### PSIM-0099: Add CI, hygiene, and prerelease packaging

Status: In Progress

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

Status: Open

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

- None yet.
