# Water visual effects acceptance - 2026-07-11

PSIM-0108 adds deterministic presentation-only whitewater cues derived from particle motion and solid proximity. They are not particles, do not add represented water area or mass, and are excluded from simulation digests.

## Accepted behavior

- Settled U-container, still-pool, hydrostatic-column, and long-run checkpoints contain no foam, spray, or impact accents.
- Dam break at tick 480 contains 16 foam points, 3 spray streaks, and 11 impact accents.
- Wall/corner impact at tick 360 contains 56 foam points, 102 spray streaks, and 22 impact accents.
- Narrow channel at tick 600 contains 42 foam points, 25 spray streaks, and 17 impact accents.
- Hard limits are 512 foam points, 256 spray streaks, and 128 impact accents.
- Reduced motion freezes tick phase and shortens spray accents.

## Evidence

- Reviewer: Codex visual review for PSIM-0108
- Review date: 2026-07-11
- Contact sheet: `build/windows-x64/recovery-visual-review/recovery-contact-sheet.bmp`
- Review manifest: `build/windows-x64/recovery-visual-review/review-manifest.json`
- Scenario metrics: sibling `scenario-*` directories under the review directory
- Accepted basin golden SHA-256: `F8E7C200E7B697CDB79B0D7DEAF0E15DC78BA54FCD00D22F3C16ABA4B134915C`

## Verification

- `scripts/build.ps1`: passed.
- `scripts/test.ps1 -Tier Fast`: 27/27 passed in 3.891 seconds.
- `scripts/test.ps1 -Tier Standard`: 29/29 passed in 28.768 seconds.
- `scripts/capture-recovery-contact-sheet.ps1 -Accept -Reviewer 'Codex visual review for PSIM-0108'`: passed.
- `scripts/verify-recovery-basin.ps1`: passed.
