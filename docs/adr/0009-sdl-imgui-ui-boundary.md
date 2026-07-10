# ADR 0009: SDL And Dear ImGui UI Boundary

Status: Accepted

Date: 2026-07-10

## Context

The sandbox needs deliberate player UX while the testbed needs dense engineering controls and plots.

## Decision

Keep custom SDL rendering and UI for sandbox mode. Use pinned Dear ImGui sources only in lab mode for scenario selection, controls, metrics, field inspection, plots, comparison, and capture. ImGui must never appear in sandbox mode or enter core/content dependencies.

## Consequences

- The first release remains one executable with `--mode sandbox|lab`.
- Lab usability can evolve without turning debug panels into the player interface.
- Dependency and smoke checks enforce the boundary.
