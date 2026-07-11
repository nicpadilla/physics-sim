# Challenge acceptance - 2026-07-11

PSIM-0110 adds optional, scene-authored challenges without placing objectives in ordinary creative sandbox scenes.

## Contract

- Scene v2 stores title, required objective sensors, hold duration, and optional emitted/outflow mass budgets.
- Runtime progress is derived only from sensor metrics and simulation ticks.
- Completion/failure never changes mass, forces, devices, or solver ordering.
- F10 reloads the authored state and resets progress.
- Missing targets, zero criteria, invalid budgets, and malformed regions fail during parsing.

## Deterministic success evidence

| Challenge | Teaching goal | Completion tick | State digest |
|---|---|---:|---|
| Fill Goal | Redirect/fill | 273 | `D18349166749EEDB` |
| Open Gate | Gate selection/toggle | 553 | `7EE30B971D89ADCD` |
| Power Crossing | Pump/valve flow | 679 | `E826B7B93FAAD25E` |

`scripts/verify-challenges.ps1` ran each replay twice and matched both completion tick and state digest. Summary: `build/windows-x64/challenge-review/summary.json`.

## Named review

- Reviewer: Codex challenge usability review for PSIM-0110
- Date: 2026-07-11
- Artifacts: `build/windows-x64/challenge-review/fill-run1.bmp`, `gate-run1.bmp`, and `pump-valve-run1.bmp`
- Result: accepted. Criteria, live hold progress, budgets, target regions, completion/restart feedback, and the relevant device are legible without debug overlays or README help.

Fast passed 28/28 in 4.083 seconds. Standard passed 30/30 in 29.293 seconds. The Release challenge verifier passed in 6.1 seconds, and a clean packaged Open Gate replay completed at tick 553 with the accepted digest.
