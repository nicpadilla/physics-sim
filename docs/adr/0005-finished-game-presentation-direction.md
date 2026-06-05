# ADR 0005: Finished Game Presentation Direction

Status: Accepted

Date: 2026-06-05

## Context

The simulator has completed the first testable water-sandbox milestones and now layers a finished-game shell around stronger fluid realism. The next product-quality layer is broader than physics: the app needs a finished-game shell, readable in-game UI, onboarding, visual and audio feedback, accessibility settings, player-facing error handling, and release packaging polish.

Without a durable direction, future polish issues could drift into isolated cosmetic changes. The project needs a standard that preserves deterministic simulation and verification while making the default player experience feel intentional.

## Decision

Future finished-game polish will treat the app as a playable sandbox game with a professional product shell around the existing deterministic simulator.

The default experience should prioritize player comprehension. Menus, HUD, scene browser, tutorial flow, save/load UX, and errors should be visible and understandable in the app. Debug overlays, solver metrics, replay tools, and regression scenes remain accessible but should not be the primary surface for normal play.

The implementation direction remains self-contained C++20 and SDL by default. New UI, rendering, audio, or physics runtime dependencies require a later ADR. Presentation effects must not become the source of truth for physics behavior; numeric tests, state tests, replays, and documented manual checks remain the evidence model.

## Consequences

- Finished-game polish work should create testable state transitions and scripted evidence, not only screenshots.
- Visual and audio improvements must preserve deterministic simulation and automated regression paths.
- Accessibility, settings, and error recovery are required parts of finish quality.
- Debug clarity remains available through explicit debug modes and overlays.
- Large platform features such as online sharing, store integration, cloud saves, and engine migration stay out of scope unless a later ADR changes direction.

## Alternatives Considered

- Keep the app as a developer-facing simulator only. This would preserve simplicity, but it conflicts with the sandbox-game roadmap and leaves the app feeling unfinished.
- Prioritize visual effects before menus and UX. This could make screenshots look better quickly, but it would not fix first-run comprehension, save/load confidence, accessibility, or release handoff.
- Adopt a full game engine or external UI framework now. This could speed some presentation work, but it would add dependency and determinism risk before the SDL shell has reached its limit.
