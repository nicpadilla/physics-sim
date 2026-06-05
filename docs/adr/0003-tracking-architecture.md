# ADR 0003: Tracking Architecture

Status: Accepted

Context:

The project needs a durable, machine-readable workflow for issue tracking, roadmap progress, verification, and agent handoff.

Decision:

Treat `ROADMAP.md`, `PROGRESS.md`, `ISSUES.md`, and `AGENTS.md` as the source of truth for product goals, live status, implementation queue, and agent workflow rules. Keep validation rules in `docs/TRACKING.md` and require named evidence for verified status.

Consequences:

- Tracking decisions stay local to the repository and do not depend on external task systems.
- Progress rows can be audited against concrete commands, tests, scenes, and manual checks.
- Agents can follow the same closure workflow without re-deriving conventions from prior conversation.
