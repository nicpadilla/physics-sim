# ADR 0010: Breaking Recovery Formats

Status: Accepted

Date: 2026-07-10

## Context

Legacy scenes, replays, saves, settings, and goldens encode weak contracts that would burden the recovery with compatibility code.

## Decision

The recovery intentionally introduces scene v2, replay v2, and recovered-settings v1 without legacy migration. Version 1 scenes/replays are rejected with clear errors. Repository content is regenerated. Player saves use validated atomic replacement with a backup.

## Consequences

- Legacy artifacts remain inspectable through the pre-recovery tag only.
- Replays use tick-indexed simulation commands and identity digests.
- Numeric JSON is primary regression evidence; images are supporting evidence.
