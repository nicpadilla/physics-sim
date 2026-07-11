# Recovery Visual Acceptance — 2026-07-10

Reviewer: Codex, acting under the project owner's explicit instruction to reach independent conclusions and continue without further input.

Decision: Accepted for the `0.2.0-alpha.1` recovery prerelease.

## Evidence reviewed

- Complete balanced/quality fluid-quality suite passed after free-surface, transfer, density-correction, and scenario-contract fixes.
- Canonical Lab captures passed at scenario-specific checkpoints, including U-container tick 2400, still pool tick 1200, narrow channel tick 600, and long run tick 6000.
- Surface views include solid geometry so containment and interaction are visually reviewable.
- U-container is a single connected surface with no isolated cells or escaped-region cells and remains below its settled-energy gate.
- Narrow-channel water visibly crosses both constrictions and the numeric test requires at least one downstream particle.
- Long-run water remains finite, connected, contained, and below its energy gate.
- Wall/corner impact visibly deflects at the internal corner with no solid penetration.

## Accepted release baselines

| Artifact | SHA-256 | Purpose |
|---|---|---|
| `regression/recovery_basin_early_surface_golden.bmp` | `ECD34453BDDCC0AB21ABCAAFC6C22DFB95936C704B2424AA955031612D9FF18A` | Tick-240 filling behavior. |
| `regression/recovery_basin_density_golden.bmp` | `0CC51A72D10682277AE63B9D450E048F7636B63C35AAB72BB78F9A7628AEF31B` | Tick-2400 settled density field. |
| `regression/recovery_basin_surface_golden.bmp` | `D712F77D205C0738535CE20201CA0DF2FAC310E13E91619A591BEC99C733296F` | Tick-2400 settled continuous surface. |

The accepted baselines use `scenes/starter_basin.pscene` and `regression/replays/recovery-basin.replay`. The replay emits from tick 0 through tick 1200, then settles through tick 2400.

## Rejected evidence

The legacy demo captures were explicitly rejected because they showed fragmented density and extensive water outside the basin. They were moved to `regression/legacy/pre-recovery-2026-07-10` and are not referenced by active verification.

## Scope

Acceptance applies to the recovery basin vertical slice and canonical Lab validation surfaces. It does not restore deferred emitters, objectives, progression, broad gallery content, or decorative polish.
