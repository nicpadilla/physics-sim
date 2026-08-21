# Contributing to Physics Sim

## Active work

GitHub Issues are the source of truth for active work. `ISSUES.md`, `PROGRESS.md`, and the recovery sections of `ROADMAP.md` preserve the completed recovery program; do not add new task status to those ledgers.

Before starting code, make the issue state the problem, desired contract, acceptance criteria, verification commands, dependencies, and explicit out-of-scope work.

## Branches and pull requests

- Create a focused branch from `main`.
- Keep one coherent issue, or one tightly coupled dependency group, in a pull request.
- Link issues with `Fixes #<number>` only when every acceptance criterion is met. Use `Refs #<number>` for partial work.
- Do not mix solver tuning, refactoring, baseline regeneration, and product changes without an issue that requires the combined change.
- Explain every changed threshold, replay digest, or visual baseline. A changed hash alone is not acceptance evidence.

## Verification tiers

- **Fast:** documentation helpers and narrow logic changes.
- **Standard:** runtime, scene, replay, UI, content, device, challenge, Lab, and ordinary solver changes.
- **Full:** solver behavior, digest contracts, baselines, packaging, release workflow, or broad architecture changes.

Common commands:

```powershell
.\scripts\check-hygiene.ps1
.\scripts\test.ps1 -Tier Fast
.\scripts\test.ps1 -Tier Standard
.\scripts\verify-all.ps1
```

## Commit and evidence rules

- Keep commits reviewable and state the issue or contract they implement.
- Record exact commands, results, timings, digests, benchmark values, and artifact paths in the pull request.
- Preserve failure artifacts before changing code or baselines.
- Reserve **Human Accepted** for a named, dated human review. Record automated and agent review as separate evidence.
- Never weaken a gate merely to make CI pass. Explain the physical or product reason for every gate change.
