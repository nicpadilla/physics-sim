# Tracking Workflow

This repo uses Markdown as the source of truth for roadmap progress and implementation work.

## Sources Of Truth

- `ROADMAP.md` is the product contract. Edit it only when stage goals or completion criteria change.
- `PROGRESS.md` is the live status dashboard for every roadmap completion marker.
- `ISSUES.md` is the implementation queue for concrete work.
- `AGENTS.md` is the operating guide for coding agents.

## Roadmap IDs

Every completion marker in `ROADMAP.md` has a stable ID in `PROGRESS.md`.

Format:

- `R<stage>.<marker>`
- Example: `R5.05` means stage 5, marker 5.
- Use two digits for marker numbers.
- Do not renumber existing IDs unless the corresponding roadmap marker is intentionally replaced.

## Progress Statuses

Allowed `PROGRESS.md` statuses:

- `Missing`: Required behavior does not exist.
- `Partial`: Some behavior exists, but the requirement is not fully satisfied.
- `Done`: Behavior appears implemented in code or docs, but verification evidence is incomplete.
- `Verified`: Requirement is backed by a named command, test, scene, regression check, or documented manual check.
- `Blocked`: Work cannot proceed until a concrete dependency is resolved.

Rules:

- Do not mark a progress item `Verified` without evidence in the row.
- Prefer `Done` over `Verified` when the only evidence is code inspection.
- Link incomplete requirements to one or more `PSIM-*` issues in the next-task column.
- If a roadmap marker changes, update `PROGRESS.md` in the same change.

## Issue IDs And Lifecycle

Issue IDs use `PSIM-0001`, `PSIM-0002`, and so on.

Issues live under `## Epic <number>: <name>` sections in `ISSUES.md`. Each epic should contain one or more `### PSIM-####: <title>` issue entries.

Allowed priorities:

- `P0`: V1 blocker.
- `P1`: Near-term foundation.
- `P2`: Future sandbox polish.

Allowed issue statuses:

- `Open`: Ready for implementation planning or work.
- `In Progress`: Currently being implemented.
- `Blocked`: Waiting on a dependency or decision.
- `Done`: Implemented and verified.
- `Deferred`: Intentionally postponed without blocking current completion.

Rules:

- Do not reuse issue IDs.
- Every issue must link to at least one valid roadmap ID unless it is pure maintenance.
- Every issue must live under an epic section.
- Every issue must include problem, acceptance criteria, subtasks, verification, dependencies, and implementation notes.
- Do not mark an issue `Done` without implementation notes and verification results.
- If work is started without a matching issue, create a new issue entry first.
- Use `P0` only for issues that block the first complete testable product.

## Canonical Issue Template

Use this structure when creating a new `PSIM-*` issue:

```markdown
### PSIM-0000: Short title

Status: Open

Priority: P1

Linked roadmap IDs: R0.00

Problem:
One sentence describing the gap.

Technical implementation direction:

- Name the affected source, test, scene, script, or docs surfaces.
- Describe the intended data flow, API shape, sequencing, thresholds, and edge cases enough for another agent to implement without inventing architecture.
- State what must not change, such as deterministic behavior, baselines, or public file formats.

Acceptance criteria:

- Criterion one.
- Criterion two.

Subtasks:

- Task one.
- Task two.

Verification:

- `.\scripts\check-tracking.ps1`
- `.\scripts\test.ps1`

Dependencies:

- PSIM-0001.

Implementation notes:

- None yet.
```

The roadmap IDs must be valid `R*.**` entries, and the issue must include every required field from the template. The tracking checker enforces `Technical implementation direction:` for every `Open` and `In Progress` issue.

## Implementation-Ready Issue Standard

An open issue is implementation-ready when another agent can identify the intended design without relying on hidden conversation context.

Required for `Open` and `In Progress` issues:

- `Technical implementation direction:` names affected files, APIs, commands, or docs surfaces.
- Acceptance criteria define observable behavior, not only intent.
- Subtasks are ordered enough to guide a safe implementation.
- Verification lists the smallest meaningful command set for the work.
- Dependencies state whether upstream issues are required, optional, superseded, or intentionally bypassed.

Recommended for complex solver, rendering, persistence, or tooling work:

- Include proposed structs, functions, settings, script parameters, or output fields.
- Include numeric thresholds or an explicit process for measuring and then recording thresholds.
- Include deterministic ordering and tie-break rules.
- Include baseline-regeneration policy if visuals or replay captures may change.
- Include non-goals so lower-capability agents do not expand scope.

Direct user exception:

- A direct user instruction may bypass creating a `PSIM-*` issue only for docs/process-only audits.
- The change must stay out of product/runtime code.
- The final response must state that the issue-link rule was explicitly bypassed by user instruction.

## Implementation Handoff Checklist

Use this checklist when starting or closing an issue:

1. Identify the linked `PSIM-*` issue and the `R*.**` rows it affects.
2. Confirm the issue meets the implementation-ready standard above; refine it first if it does not.
3. Move the issue to `In Progress` before editing code or docs.
4. Add or update tests, scenes, or manual checks that prove the change.
5. Run the smallest named verification command set that covers the issue.
6. Update `PROGRESS.md` with `Verified` only when evidence is recorded in the row.
7. Update `ISSUES.md` implementation notes and verification results.
8. Re-run `.\scripts\check-tracking.ps1` before closing the issue.
9. Mark the issue `Done` only after the tracking files and evidence agree.
10. Stage only the files intentionally changed for the issue.
11. Commit the completed work after verification passes.
12. Report the commit hash and any commands not run.

## Commit Workflow

Agents should leave completed repository work in a commit, not only as working-tree edits.

Default sequence:

1. Inspect `git status --short` before staging.
2. Verify that every changed file is related to the active issue or explicit user request.
3. Run the required verification command set.
4. Stage only intentional files, using explicit paths.
5. Review `git diff --cached` for unrelated edits, generated noise, secrets, machine-specific paths, or accidental baseline changes.
6. Create one focused commit for the completed unit of work.
7. Report the commit hash in the final response.

Commit boundaries:

- Use one commit per completed issue or explicit docs/process task.
- Do not combine unrelated product, tracking, generated, and cleanup changes.
- Do not commit broad untracked files just because they are present.
- Do not commit failed work as complete.
- A checkpoint commit is allowed only when the user asks for one or when a long task has a useful verified intermediate state.

Dirty worktree handling:

- Treat pre-existing dirty files as user or prior-agent work.
- Stage only files touched for the current task.
- If a file contains both current-task edits and unrelated edits, either isolate the intended hunk safely or report the overlap.
- If safe staging is not possible, leave the work uncommitted and explain the exact blocker.

## Git Hook Workflow

The repo keeps workflow hooks versioned under `scripts\git-hooks`.

Install or verify local hook wiring:

```powershell
.\scripts\install-git-hooks.ps1
.\scripts\install-git-hooks.ps1 -Check
```

Test hook behavior without making a commit:

```powershell
.\scripts\test-git-hooks.ps1
```

Hook responsibilities:

- `pre-commit` runs `git diff --cached --check`, runs `.\scripts\check-tracking.ps1`, and rejects staged generated/local-output paths such as `build/` and `dist/`.
- `commit-msg` requires either a `PSIM-####` reference or an approved workflow prefix: `docs`, `chore`, `build`, `test`, `release`, `ci`, or `refactor`.
- `pre-push` runs `.\scripts\check-tracking.ps1` and `.\scripts\test.ps1` by default.

Pre-push escape hatches:

- Set environment variable `PSIM_SKIP_PRE_PUSH_TESTS=1` for a one-off push that should skip `.\scripts\test.ps1`.
- Set `git config hooks.physicsSim.skipPrePushTests true` for a local persistent skip.
- Skips should be reported in the final response or handoff note.

## Backlog Review Cadence

Review open issues on a regular cadence so the backlog does not drift:

- Reassess open `PSIM-*` issues at each stage or release checkpoint.
- Revisit stale issues when a dependency lands, a roadmap marker changes, or new evidence makes the scope clearer.
- If an issue is no longer needed, mark it `Deferred` with a short note explaining why.
- Record the next planned backlog review in the issue notes or a planning note when the review uncovers new work.

## Evidence Rules

Use the strongest applicable evidence:

- Automated tests: `.\scripts\test.ps1`, named CTest target, or specific test executable.
- Build evidence: `.\scripts\build.ps1`.
- Smoke evidence: `.\scripts\run-smoke.ps1`.
- Visual regression: `.\scripts\verify-demo-scene.ps1` or a named regression command.
- Manual evidence: a dated, reproducible manual check with exact actions and expected outcome.

Regression baselines:

- Only update baseline images when an issue explicitly requires the change.
- Record the command used to create the baseline.
- Record why the visual change is expected.

## Agent Workflow

Before implementation:

- Read `AGENTS.md`, `ROADMAP.md`, `PROGRESS.md`, and `ISSUES.md`.
- Identify the linked `PSIM-*` issue or create one.
- Identify which `R*.**` progress IDs can change.

During implementation:

- Keep changes scoped to the issue.
- Prefer tests or scripted evidence for behavior changes.
- Do not downgrade existing evidence without explaining why.

After implementation:

- Update the issue status and implementation notes.
- Update linked progress rows.
- Record verification commands and results.
- Leave unrelated roadmap rows untouched.
- Commit the completed, verified change using the commit workflow above.
