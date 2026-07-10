# AGENTS.md

Instructions for Codex and other coding agents working in this repository.

## Read Order

Read these files before implementation work:

1. `AGENTS.md`
2. `ROADMAP.md`
3. `PROGRESS.md`
4. `ISSUES.md`
5. Relevant source, test, scene, regression, or script files
6. `docs/adr/README.md` and the linked decision records for major architecture choices

Use `docs/TRACKING.md` for the tracking rules and lifecycle details.

## Project Shape

This is a Windows C++20 / SDL2 2D physics simulator focused on water first.

Important commands:

- Configure: `.\scripts\configure.ps1`
- Build: `.\scripts\build.ps1`
- Test: `.\scripts\test.ps1`
- Smoke: `.\scripts\run-smoke.ps1`
- Verification bundle: `.\scripts\verify-all.ps1`
- Solver benchmark: `.\scripts\measure-water-solver.ps1`
- Release package: `.\scripts\package-release.ps1`
- Demo regression: `.\scripts\verify-demo-scene.ps1`
- Recovery contact sheet: `.\scripts\capture-recovery-contact-sheet.ps1 -Tick 2400`
- Tracking check: `.\scripts\check-tracking.ps1`
- Dependency boundary check: `.\scripts\check-dependencies.ps1`
- Install Git hooks: `.\scripts\install-git-hooks.ps1`
- Hook contract check: `.\scripts\test-git-hooks.ps1`

## Tracking Requirements

- Link every implementation change to an existing `PSIM-*` issue in `ISSUES.md`, or create a new issue before implementation.
- A direct user instruction may bypass issue creation only for docs/process-only audits. Keep that scope out of product/runtime code and report the explicit bypass in the final response.
- Update `PROGRESS.md` only for roadmap IDs directly affected by the change.
- Do not mark a progress item `Automated` without current command, date, duration, result, and scenario/profile evidence where applicable.
- Do not mark a progress item `Human Accepted` without a named, dated review and artifact path; a matching golden hash proves determinism only.
- Do not mark an issue `Done` without implementation notes and verification results.
- Keep `ROADMAP.md` stable unless the product goals or completion criteria are intentionally changing.
- Follow the canonical issue template and implementation handoff checklist in `docs\TRACKING.md` when opening, working, or closing `PSIM-*` issues.
- Use the backlog review cadence in `docs\TRACKING.md` when pruning, deferring, splitting, or reprioritizing open work.
- Open and in-progress issues should be implementation-ready: include concrete technical direction, affected code/test surfaces, acceptance thresholds where practical, dependency assumptions, and verification commands.

## Agentic Handoff Rules

- Before starting an issue, confirm it has enough technical direction for a lower-capability agent to make progress without inventing architecture.
- If an issue is too vague, refine `ISSUES.md` first rather than guessing in code.
- Treat `Technical implementation direction:` as the handoff contract for open and in-progress issues.
- Keep implementation notes factual: list what changed, where, and which verification commands passed or failed.
- Do not leave `Implementation notes: None yet.` on `Done` or `Deferred` issues.

## Commit Discipline

- After making repository changes and completing the required verification, commit the completed work before handing control back.
- Commit only files intentionally changed for the active task. Do not stage unrelated dirty or untracked files.
- If verification fails, do not commit a success-state change. Report the failure, keep the issue open, and leave the worktree for follow-up unless the user asks for a checkpoint commit.
- If a commit cannot be made because the worktree contains ambiguous overlapping user changes, report the exact blocker instead of staging broadly.
- Use concise commit messages that name the issue or workflow area, for example `PSIM-0073 add viscosity force pass` or `docs tighten agent handoff workflow`.
- Run `.\scripts\install-git-hooks.ps1` in a new checkout so Git uses the versioned hooks under `scripts\git-hooks`.

## Engineering Rules

- Keep changes scoped to the active issue.
- Prefer existing headers and patterns under `include\physics_sim`.
- Keep C++ code C++20-compatible and MSVC-friendly.
- Preserve deterministic demo/regression behavior unless the active issue explicitly changes it.
- Do not change regression baselines unless the issue requires it and the capture command is recorded.
- Avoid introducing machine-specific absolute paths outside documented helper scripts.

## Verification Defaults

Use the smallest meaningful verification set for the change:

- Docs/tracking-only change: `.\scripts\check-tracking.ps1` and `.\scripts\test.ps1`.
- Core C++ behavior change: `.\scripts\build.ps1` and `.\scripts\test.ps1`.
- Runtime/input change: add `.\scripts\run-smoke.ps1`.
- Visual/demo change: add `.\scripts\verify-demo-scene.ps1`.

Always report commands that were not run and why.
