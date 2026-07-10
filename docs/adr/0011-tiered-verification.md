# ADR 0011: Tiered Verification

Status: Accepted

Date: 2026-07-10

## Context

The default CTest run takes roughly four minutes because it mixes unit tests, long solver scenarios, benchmarks, and visual captures.

## Decision

Provide Fast, Standard, and Full tiers targeting 30 seconds, 90 seconds, and 8 minutes. Label CTest coverage by purpose and cost. Fast is the local feedback gate, Standard covers representative integration/solver/smoke behavior, and Full is the release gate. Failed structured evidence is retained.

## Consequences

- Benchmarks and long visual captures leave the fast path.
- CI runs tiers based on event risk.
- `verify-all.ps1` becomes the Full release alias.
