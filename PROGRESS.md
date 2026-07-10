# Recovery Progress

Allowed statuses: `Missing`, `Partial`, `Implemented`, `Automated`, `Human Accepted`, `Blocked`.

`Automated` requires current command evidence. `Human Accepted` requires a named, dated visual or usability review. Pre-recovery evidence does not automatically satisfy recovery markers.

| ID | Requirement | Status | Evidence | Next task or blocker |
| --- | --- | --- | --- | --- |
| R14.01 | Preserve the pre-recovery project snapshot. | Automated | Annotated tag `pre-recovery-2026-07-10` resolves to legacy commit `647153c`; direct inspection is documented. Verified 2026-07-10. | None. |
| R14.02 | Use evidence-aware recovery tracking statuses. | Automated | `.\scripts\check-tracking.ps1` passed in 0.6 seconds on 2026-07-10 with the recovery status contract. | None. |
| R14.03 | Keep recovery issues implementation-ready and summary/detail consistent. | Automated | `.\scripts\check-tracking.ps1` passed in 0.6 seconds on 2026-07-10 for 13 recovery issues and exact summary/detail validation. | None. |
| R14.04 | Record recovery architecture and workflow ADRs. | Automated | ADRs 0007-0011 are accepted and `.\scripts\check-tracking.ps1` plus `.\scripts\test.ps1` (26/26 in 170.7 seconds) passed on 2026-07-10. | None. |
| R14.05 | Capture a trusted baseline report. | Automated | `.\scripts\capture-recovery-baseline.ps1` completed in 570.5 seconds on 2026-07-10 and published environment, command, timing, hash, visual-review, and classified-failure evidence. | None. |
| R14.06 | Decompose the application into compiled module targets. | Automated | Compiled core/content/app/lab targets, composition-only main, 28/28 CTest, smoke, and dependency checks passed on 2026-07-10; full internal extraction remains staged behind these boundaries. | None. |
| R15.01 | Expose stable simulation client interfaces. | Automated | `SimulationConfig`, command, snapshot, metrics, and facade tests passed on 2026-07-10; new recovery clients have no mutable solver access. | None. |
| R15.02 | Cover core physics invariants with focused tests. | Automated | `test.ps1 -Tier Fast` passed 23/23 in 4.1 seconds on 2026-07-10, including transfer, pressure, equilibrium, gravity, collision, density/resampling, accounting, and repeated-digest tests. | None. |
| R15.03 | Define canonical machine-readable validation scenarios. | Automated | `verify-fluid-quality-suite.ps1` passed all 18 balanced/quality scenario runs in 303 seconds on 2026-07-10 and wrote the structured summary under `build/windows-x64/fluid-quality-suite`. | None. |
| R15.04 | Enforce hard accounting, convergence, penetration, finiteness, and determinism gates. | Automated | The 2026-07-10 fluid suite enforced mass `1e-5`, momentum `1e-4`, pressure `5e-5`/`1e-5`, and zero penetration, non-finite, and unexplained-lifecycle gates with 18/18 passing. | None. |
| R15.05 | Pass balanced and quality numeric/performance budgets. | Automated | `test.ps1 -Tier Standard` passed 25/25 in 78.2 seconds and the 2026-07-10 Release all-profile benchmark passed; balanced and quality demo steps averaged 0.5028 ms and 0.6951 ms. | None. |
| R16.01 | Render continuous player-facing water. | Missing | Current surface baseline shows disconnected filled-cell output. | PSIM-0095. |
| R16.02 | Interpolate rendering without mutating simulation state. | Missing | No recovery interpolation contract exists. | PSIM-0095. |
| R16.03 | Expose complete laboratory field views. | Partial | Existing debug modes are incomplete and coupled to the app renderer. | PSIM-0097. |
| R16.04 | Run semantic visual checks. | Missing | Existing image regressions use hash equality only. | PSIM-0095. |
| R16.05 | Require numeric plus human visual acceptance. | Missing | No recovered capture has human acceptance evidence. | PSIM-0095. |
| R17.01 | Deliver the narrow sandbox interaction loop. | Partial | Recovery scene v2 and atomic save/load contracts passed automated tests on 2026-07-10; player-loop scope and acceptance remain PSIM-0096. | PSIM-0096. |
| R17.02 | Deliver accessible player-facing SDL UI. | Partial | Existing UI needs scope reduction and recovered acceptance. | PSIM-0096. |
| R17.03 | Teach the loop through first-run interaction. | Partial | Existing tutorial is not accepted against the narrow recovery loop. | PSIM-0096. |
| R17.04 | Deliver Dear ImGui laboratory mode. | Missing | Dear ImGui is not a dependency and lab mode does not exist. | PSIM-0097. |
| R17.05 | Make lab forks and digest divergence explicit. | Missing | No lab run identity contract exists. | PSIM-0097. |
| R17.06 | Keep deferred breadth out of the first recovery release UI. | Missing | Existing runtime exposes secondary devices and progression. | PSIM-0096. |
| R18.01 | Meet fast, standard, and full verification time budgets. | Missing | Current default test run takes about four minutes. | PSIM-0098. |
| R18.02 | Label CTest coverage by purpose and cost. | Missing | Tests are currently unlabeled. | PSIM-0098. |
| R18.03 | Retain structured failure artifacts. | Missing | Several regression scripts delete captures and emit unstructured logs. | PSIM-0098. |
| R18.04 | Run Windows CI with scheduled and release verification. | Missing | No remote or CI workflow is configured. | PSIM-0099. |
| R18.05 | Enforce static analysis and hygiene. | Partial | `/W4` and absolute-path checks exist; remaining gates are absent. | PSIM-0099. |
| R18.06 | Produce and verify the recovery prerelease package. | Missing | No current package exists. | PSIM-0099. |
| R19.01 | Decide every deferred feature deliberately. | Missing | Feature restoration audit has not occurred. | PSIM-0100. |
| R19.02 | Require full evidence for restored features. | Missing | No recovery feature has passed the new gate. | PSIM-0100. |
| R19.03 | Protect the vertical slice from restored-feature regressions. | Missing | Vertical slice is not yet accepted. | PSIM-0100. |
| R19.04 | Complete final recovery acceptance with no open P0/P1 work. | Missing | Recovery program is active. | PSIM-0101. |
