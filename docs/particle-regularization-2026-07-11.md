# Particle Regularization Calibration - 2026-07-11

## Contract

PSIM-0107 adds a deterministic mass-weighted pair regularizer for under-spaced neighboring particles. Pair displacements are equal and opposite in mass-weighted space, the complete correction field receives one global displacement scale, velocities are untouched, and particles separated beyond compact support are never attracted or merged.

Small supported components are protected by a minimum eight-particle population. Balanced runs one iteration every four ticks at `0.50`-cell support and a `0.010`-cell displacement cap. Quality runs two iterations every tick at `0.45`-cell support and the same cap. Both use strength `0.25`.

## Rejected candidates

- Per-tick balanced shifting with a `0.012`-cell cap raised the settled U-container energy to `60,352`, above its `50,000` gate, and was rejected.
- Reducing displacement without a cadence contract preserved density but left the steady-pour sampling CV above `0.40` or produced an unnecessarily energetic surface.
- Applying regularization to the four-particle overcrowding fixture lowered its density estimate to `0.449740` and missed the existing error gate by `0.000260`; small-component protection fixed the cause without changing the gate.
- Wider quality support (`0.50` to `0.60` cells) failed the promoted leveling-CV gate in most cadence combinations and was rejected.

## Selected measurements

| Profile/scenario | Sampling CV | Surface RMS slope | Kinetic energy | Main component |
| --- | ---: | ---: | ---: | ---: |
| Balanced asymmetric leveling | `0.298` | `0.208` | `0.049` | `1.000` |
| Balanced steady pour | `0.387` | `0.271` | `7.687` | `1.000` |
| Balanced U-container | `0.446` | `0.333` | `11,554` | `1.000` |
| Quality asymmetric leveling | `0.311` | `0.125` | `0.035` | `1.000` |
| Quality steady pour | `0.348` | `0.338` | `13.802` | `1.000` |
| Quality U-container | `0.423` | `0.348` | `10,408` | `1.000` |

Obstacle breakup retains at least `0.90` of particles in the main component and no more than three components at the accepted checkpoint. The renderer continues to show physically separated supported droplets; regularization does not add cohesion or delete mass.

## Verification

- `.\scripts\test.ps1 -Tier Fast`: 26/26 passed in 4.164 seconds on 2026-07-11.
- `.\scripts\test.ps1 -Tier Standard`: 28/28 passed in 33.83 seconds on 2026-07-11.
- `.\scripts\verify-fluid-quality-suite.ps1`: all 30 balanced/quality manifest cases passed on 2026-07-11 with the promoted sampling/rejoin gates and zero hard-gate failures.
- `.\scripts\measure-water-solver.ps1 -Profile All`: all budgets passed. Balanced stress averaged `0.4302 ms/step`; quality `1.6074 ms/step`; every reported mass error was `0.000000`.

Visual baseline replacement remains PSIM-0108 work because that issue intentionally changes the presentation layer and requires a new named review.
