# ADR 0008: Compiled Module Boundaries

Status: Accepted

Date: 2026-07-10

## Context

Substantial runtime behavior lives in large headers and a monolithic executable source, allowing client code to depend on solver internals.

## Decision

Use compiled `physics_sim_core`, `physics_sim_content`, `physics_sim_app`, and `physics_sim_lab` targets behind one executable. Clients interact through simulation config, command, snapshot, and metrics interfaces. Core has no SDL or ImGui dependency; content depends on core; app depends on content/core/SDL; lab additionally depends on ImGui.

## Consequences

- Refactoring is staged behind deterministic regression gates.
- Public headers are declarative and algorithms move to implementation units.
- UI and rendering mutate simulation only through commands.
