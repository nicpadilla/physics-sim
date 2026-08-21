# Tracking Workflow

GitHub Issues and pull requests are the source of truth for active work. The Markdown recovery files remain the dated evidence record for the completed recovery program.

## Sources Of Truth

- GitHub Issues hold active implementation work, priorities, dependencies, milestones, and closure state.
- GitHub pull requests hold the reviewable implementation, linked issue references, and current verification evidence.
- `ROADMAP.md` preserves the completed recovery product contract and may hold broad product direction, but it is not an active task board.
- `PROGRESS.md` preserves the completed recovery status and acceptance ledger.
- `ISSUES.md` preserves the completed `PSIM-*` recovery queue and implementation notes.
- `AGENTS.md` is the operating guide for coding agents.

New active work must not be copied into `ISSUES.md` or `PROGRESS.md` as a second status system. Use `Fixes #<number>` only when a pull request satisfies the full GitHub issue. Use `Refs #<number>` for partial work. **Human Accepted** requires a named, dated human review; automated and agent review remain separate evidence classes.

## Recovery Roadmap IDs

Every completion marker in the completed recovery `ROADMAP.md` has a stable ID in `PROGRESS.md`.

Format:

- `R<stage>.<marker>`
- Example: `R5.05` means stage 5, marker 5.
- Use two digits for marker numbers.
- Do not renumber existing IDs unless the corresponding recovery marker is intentionally corrected.

## Recovery Progress Statuses

Allowed historical `PROGRESS.md` statuses:

- `Missing`: Required behavior does not exist.
- `Partial`: Some behavior exists, but the requirement is not fully satisfied.
- `Implemented`: Behavior exists, but current automated evidence is incomplete.
- `Automated`: Current automated evidence passes with command, date, duration, and scenario/profile where applicable.
- `Human Accepted`: A named, dated visual or usability review passed in addition to required automation.
- `Blocked`: Work cannot proceed until a concrete dependency is resolved.

Rules:

- Do not mark a progress item `Automated` without current command evidence in the row.
- Do not mark a progress item `Human Accepted` without reviewer, date, artifact, and review scope.
- Prefer `Implemented` when the only evidence is code inspection.
- A matching golden-image hash proves determinism only; it cannot establish visual or usability acceptance.
- Link incomplete historical recovery requirements to one or more `PSIM-*` issues in the next-task column.
- If a recovery marker changes, update `PROGRESS.md` in the same change.

## Recovery Issue IDs And Lifecycle

The historical recovery issue IDs use `PSIM-0001`, `PSIM-0002`, and so on. New active work uses normal GitHub issue numbers instead of adding new `PSIM-*` entries.

Historical issues live under `## Epic <number>: <name>` sections in `ISSUES.md`. Each epic contains one or more `### PSIM-####: <title>` issue entries.

Allowed priorities:

- `P0`: V1 blocker.
- `P1`: Near-term foundation.
- `P2`: Future sandbox polish.

Allowed historical issue statuses:

- `Open`: Ready for implementation planning or work.
- `In Progress`: Currently being implemented.
- `Blocked`: Waiting on a dependency or decision.
- `Done`: Implemented and verified.
- `Deferred`: Intentionally postponed without blocking current completion.

Rules:

- Do not reuse issue IDs.
- Every historical issue must link to at least one valid roadmap ID unless it is pure maintenance.
- Every historical issue must live under an epic section.
- Every historical issue must include problem, acceptance criteria, subtasks, verification, dependencies, and implementation notes.
- Do not mark an issue `Done` without implementation notes and verification results.
- Do not add new active issues to `ISSUES.md`; create a GitHub issue instead.
- Use `P0` only for issues that block the first complete testable product.

## Canonical Historical Issue Template

The following structure remains documented for interpreting or correcting a recovery `PSIM-*` record. New active work uses `.github/ISSUE_TEMPLATE/implementation.md`.

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

The roadmap IDs must be valid `R*.**` entries, and the historical issue must include every required field from the template. The tracking checker enforces `Technical implementation direction:` for every retained `Open` and `In Progress` recovery issue.

## Active Issue Standard

An active GitHub issue is implementation-ready when another contributor can identify the intended design without relying on hidden conversation context.

Required for active implementation work:

- The problem and desired contract are explicit.
- Scope names affected files, APIs, commands, or docs surfaces.
- Acceptance criteria define observable behavior, not only intent.
- Verification lists the smallest meaningful command set for the work.
- Dependencies state whether upstream issues are required, optional, superseded, or intentionally bypassed.
- Out-of-scope work prevents unrelated expansion.
- Evidence is added during implementation rather than inferred after the fact.

Recommended for complex solver, rendering, persistence, or tooling work:

- Include proposed structs, functions, settings, script parameters, or output fields.
- Include numeric thresholds or an explicit process for measuring and then recording thresholds.
- Include deterministic ordering and tie-break rules.
- Include baseline-regeneration policy if visuals or replay captures may change.
- Include non-goals so lower-capability agents do not expand scope.

Direct user exception:

- A direct user instruction may bypass creating a GitHub issue only for docs/process-only audits.
- The change must stay out of product/runtime code.
- The final response must state that the issue-link rule was explicitly bypassed by user instruction.

## Implementation Handoff Checklist

Use this checklist when starting or closing active work:

1. Identify the linked GitHub issue and any retained recovery records it affects.
2. Confirm the issue meets the implementation-ready standard above; refine it first if it does not.
3. Add or update tests, scenes, or manual checks that prove the change.
4. Run the smallest named verification command set that covers the issue.
5. Record exact commands, results, timings, digests, benchmark values, and artifact paths in the pull request.
6. Use `Refs #<number>` while acceptance criteria remain incomplete.
7. Use `Fixes #<number>` only when the pull request satisfies the entire issue.
8. Re-run `.\scripts\check-tracking.ps1` before final review.
9. Do not close the issue until the implementation is merged or an explicit non-code resolution is recorded.
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

Review open GitHub issues on a regular cadence so the backlog does not drift:

- Reassess open issues at each release checkpoint.
- Revisit stale issues when a dependency lands, a roadmap goal changes, or new evidence makes the scope clearer.
- If an issue is no longer needed, close it with `not planned` and a short explanation.
- Record new blocking or major findings as linked GitHub issues.

## Evidence Rules

Use the strongest applicable evidence:

- Automated tests: `.\scripts\test.ps1`, named CTest target, or specific test executable.
- Build evidence: `.\scripts\build.ps1`.
- Smoke evidence: `.\scripts\run-smoke.ps1`.
- Visual regression: `.\scripts\verify-recovery-basin.ps1` or a named regression command.
- Manual evidence: a dated, reproducible manual check with exact actions and expected outcome.

Every verification entry records the command or review, date, result, duration, and relevant profile/scenario. Visual and usability reviews also record the reviewer and artifact path. Evidence from the pre-recovery snapshot is historical context, not current acceptance.

Regression baselines:

- Only update baseline images when an issue explicitly requires the change.
- Record the command used to create the baseline.
- Record why the visual change is expected.
- Record semantic checks and a named human review before treating a visual baseline as accepted.

## Agent Workflow

Before implementation:

- Read `AGENTS.md`, the linked GitHub issue, and relevant recovery or architecture documents.
- Identify which active issue and retained recovery records can change.

During implementation:

- Keep changes scoped to the issue.
- Prefer tests or scripted evidence for behavior changes.
- Do not downgrade existing evidence without explaining why.

After implementation:

- Update the pull request with verification and evidence.
- Leave unrelated recovery rows untouched.
- Commit the completed, verified change using the commit workflow above.
