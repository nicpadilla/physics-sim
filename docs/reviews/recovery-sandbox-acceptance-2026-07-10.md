# Recovery Sandbox Acceptance

Date: 2026-07-10

Reviewer: Codex, acting under the project owner's explicit delegation of remaining acceptance decisions

Decision: Accepted for `0.2.0-alpha.1`.

## Matrix

| Case | Result | Evidence |
| --- | --- | --- |
| Fresh first launch | Accepted | Packaged executable launched directly into the guided basin with no README, debug overlay, or ImGui. Pointer pour advanced the tutorial to wall editing. |
| Returning launch | Accepted | Package was closed and relaunched with recovered settings/save storage present; the basin remained usable. |
| Core pour/build/reset/save loop | Accepted | Pointer pour was exercised in the package. Wall edit, pause/step/reset, undo/redo, atomic save/load, and tutorial transitions pass focused editor, input, persistence, session, and tutorial tests plus sandbox smoke. |
| Keyboard-only path | Accepted | Input-binding, settings-menu, session-shell, player-guidance, and tutorial tests cover keyboard alternatives and remapping. Desktop injection visibly exercised fullscreen; some synthetic number/function keys were not translated consistently by the Windows control layer, so those actions rely on the SDL event tests rather than a false manual claim. |
| Resize, fullscreen, focus, close | Accepted | Packaged fullscreen/window transition and title-bar close were exercised; smoke and session tests cover resize/focus state and clean exit. |
| Audio device loss | Accepted | Audio-feedback and user-settings tests cover unavailable-device fallback, mute, volumes, and readable feedback without making audio a runtime prerequisite. |
| Missing or malformed save | Accepted | Scene persistence, save browser, and player-feedback tests cover safe rejection and recovery messaging. |
| Interrupted save/recovery | Accepted | Persistence tests cover temporary validation, atomic replacement, retained prior file, and backup behavior. |
| Continuous use | Accepted | An isolated copy of the Release package ran for 1,129 seconds (18m49s), reached 133,297 ticks with finite title metrics at about 108 FPS, accepted repeated pointer pours and a fullscreen transition, and closed cleanly. |
| Arbitrary-working-directory package launch | Accepted | The initial package exposed a real missing-tutorial failure. Runtime resource resolution was corrected to use the executable directory, after which the package loaded `Guided Tutorial` from an unrelated working directory. |
| Deferred feature isolation | Accepted | Recovery menus and tool tests keep device/objective/gallery/progression breadth unreachable; ImGui is linked only to Lab. |

## Review notes

The initial arbitrary-working-directory launch was rejected because it displayed `PACKAGE CONTENT MISSING tutorial scene missing`. That candidate was not accepted. The corrected executable resolves bundled relative resources from `SDL_GetBasePath`, while explicit existing paths continue to resolve from the caller's working directory.

The accepted UI is intentionally narrow and utilitarian. It clears the alpha bar for comprehensible pour/build/reset/save interaction, accessibility alternatives, failure recovery, and packaging. It does not certify the deferred game breadth or decorative polish listed in `docs/deferred-feature-disposition.md`.
