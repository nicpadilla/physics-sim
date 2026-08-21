# Active Work Tracking

GitHub Issues are the source of truth for active implementation work after the completed recovery program.

## Repository records

- GitHub Issues and milestones: current work, priority, dependencies, and completion state.
- Pull requests: implementation, review, and exact verification evidence.
- `ISSUES.md`: completed recovery issue ledger.
- `PROGRESS.md`: completed recovery evidence and acceptance ledger.
- `ROADMAP.md`: completed recovery product contract and retained direction record.
- `docs/TRACKING.md`: evidence and status rules that continue to govern historical records and new pull requests.

New issue status must not be duplicated into the completed Markdown ledgers. Corrections to those ledgers should only fix their historical record.

## Evidence states

Automated checks, agent review, and human acceptance are distinct:

- **Automated:** a current command and retained result prove a machine-checkable contract.
- **Agent reviewed:** an identified agent inspected the specified evidence; this is not human acceptance.
- **Human Accepted:** a named or approved anonymized human reviewer completed a dated visual or usability review.

## Completion rule

Use `Fixes #<number>` only when the pull request meets every acceptance criterion. Use `Refs #<number>` for partial or enabling work. Closing an issue should include the merged PR and the required evidence.
