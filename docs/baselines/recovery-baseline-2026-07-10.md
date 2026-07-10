# Recovery Baseline: 2026-07-10

## Identity And Environment

- Product source commit: `ce371b804c47185e89a924325575437a3a745539`
- Branch: `codex/physics-sim-bootstrap`
- Capture window: 2026-07-10 14:40:17-14:49:47 CDT
- Host: AMD Ryzen 7 9800X3D, Windows 11 Pro 10.0.26200
- Toolchain observed during configure/build: CMake 3.31.6-msvc6, MSBuild 17.14.23, MSVC 14.44.35207, Windows SDK 10.0.26100.0, SDL2 2.32.10
- Configuration: Debug, `build/windows-x64`
- Structured generated summary: `build/windows-x64/recovery-baseline/baseline-summary.json`

This is reproducibility and diagnostic evidence. It grants no human acceptance and does not authorize refreshing a golden or relaxing a threshold.

## Command Results

| Step | Result | Duration | Profile/scenario | Finding |
| --- | --- | ---: | --- | --- |
| Tracking | Pass | 0.379 s | Recovery tracker | 32 markers, 13 issues, and 8 epics validated. |
| Build | Pass | 10.781 s | Debug | Configure and all targets built successfully. |
| All CTest targets | Pass, 26/26 | 169.078 s | Legacy aggregate | The aggregate does not include the dedicated replay manifest and does not exercise every dedicated fluid-suite invocation. |
| Smoke | Pass | 11.441 s | Balanced startup | Executable launched and exited successfully. |
| Replay suite | **Fail** | 2.161 s | Balanced, `demo-add-directional` | Golden SHA-256 `0A939310...70D83`; capture `DE40E02A...3F5B2`. The failed capture is retained under `build/windows-x64/replay-suite`. |
| Fluid-quality suite | **Fail** | 241.639 s | Quality, `long-run-stress`, tick 6000 | Average density error expected `<= 1.0`, actual `1.311553`. |
| All solver profiles | Pass legacy budgets | 56.899 s | Fast/balanced/quality; two benchmark scenes | Legacy profile tolerances are substantially looser than recovery pressure gates; passing them is not recovered physics acceptance. |
| Clean demo regression | Pass hash | 2.057 s | Balanced U-container, tick 240 | Deterministic only. |
| Density demo regression | Pass hash | 9.181 s | Balanced U-container, tick 960 | Deterministic only. |
| Surface demo regression | Pass hash | 53.643 s | Balanced U-container, tick 2400 | Deterministic only; visual review fails below. |
| Legacy package | Pass creation | 11.256 s | Balanced | Package created with executable, SDL DLL, README, and legacy scenes; clean-directory smoke is not part of this result. |

Total capture duration was 570.5 seconds. The work is highly duplicated and serial; this is test-quality and feedback-performance debt for PSIM-0098.

## Numeric Baseline Findings

- The dedicated quality long-run scenario fails its existing density gate. This is a numerical defect for PSIM-0094, not a candidate for threshold relaxation.
- Balanced `demo-grid-flow` reports pressure relative residual `0.007156` against its legacy target `0.01`; quality reports `0.037154` against `0.05`. Both are far above the recovery targets of `5e-5` and `1e-5`, so legacy convergence success is not recovery success.
- All benchmark rows report zero legacy mass error, but recovery validation must independently prove the ledger and unexplained lifecycle gates through PSIM-0093.
- The benchmark passes elapsed-time budgets, with balanced demo flow averaging 5.3022 ms per step and quality averaging 7.0361 ms per step on this host.

## Visual Review Sheet

Review: Codex artifact inspection, 2026-07-10. This is a failure classification, not human acceptance.

| Evidence | Review | Classification | Recovery owner |
| --- | --- | --- | --- |
| `regression/demo_scene_golden.bmp` | Water appears as disconnected square/cell fragments rather than a coherent stream or pool. | Rendering and numerical presentation debt. | PSIM-0094, PSIM-0095. |
| `regression/demo_scene_density_golden.bmp` | Occupancy remains sparse and fragmented; density view provides no player-quality surface evidence. | Rendering/tooling debt. | PSIM-0095. |
| `regression/demo_scene_surface_golden.bmp` | Most of the domain becomes occupied by disconnected cyan cells, including extensive regions outside the basin. | Numerical and rendering blocker. | PSIM-0094, PSIM-0095. |
| Directional replay golden versus retained failed capture | Large state/render difference is visible; current replay output does not reproduce its committed golden. | Determinism/regression blocker. | PSIM-0092, PSIM-0093. |
| Still pool, hydrostatic column, dam break, wall/corner impact, narrow channel, overcrowding, long run | Legacy harness produces numeric logs but no committed fixed-tick image bundle for each case. | Baseline tooling gap. | PSIM-0093, PSIM-0095, PSIM-0097. |

## Failure Inventory

1. **Numerical:** quality long-run density error exceeds the existing limit; legacy pressure convergence tolerances do not meet recovery gates; visible basin behavior suggests over-spread/fragmentation.
2. **Determinism/regression:** dedicated directional replay does not match its committed golden even though aggregate CTest passes.
3. **Rendering:** filled-cell surface output is disconnected and unsuitable for player acceptance.
4. **Interaction/product:** the active executable still exposes broad legacy tools and shell behavior instead of the narrow recovery loop.
5. **Performance:** balanced solver timing is inside the legacy live budget, but verification takes roughly nine minutes because expensive cases are repeated serially.
6. **Test quality:** aggregate CTest can be green while dedicated replay and fluid suites fail; failure evidence is split across script-specific logs.
7. **Documentation/release:** the generated legacy package exists, but it is not a recovered release and has no clean-directory smoke evidence.

## Legacy Golden Boundary

The five committed BMP goldens and their hashes remain preserved in the pre-recovery tag and are listed in the generated JSON summary. They are comparison inputs only. New accepted goldens require recovery numeric gates, semantic checks, generation commands, reviewer, date, and review artifacts.
