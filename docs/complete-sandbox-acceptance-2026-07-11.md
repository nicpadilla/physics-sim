# Complete Sandbox Acceptance — 2026-07-11

Named review: **Codex integrated complete-water-sandbox review for PSIM-0113**

Decision: Accepted for the `0.2.0-alpha.2` prerelease candidate.

## Product flow

The complete custom-SDL experience was reviewed as one product rather than isolated features. A fresh session presents the tutorial and core pour/build/reset/save loop without engineering overlays. The compact tool palette keeps pouring and wall editing primary; Advanced Tools explicitly expands to every emitter and flow-control device. Challenges show targets, hold progress, budgets, restart, completion, and failure. The curated gallery separates Learn, Sandbox, and Challenges and confirms scene loads. Laboratory mode remains a deliberate engineering surface over the same deterministic core.

Keyboard and pointer paths cover menus, gallery, core tools, advanced tools, device selection/configuration, pause/step/reset, undo/redo, save/load, and return navigation. Reduced-motion, high-contrast, mute, missing-audio, resize/fullscreen, malformed save, and package-directory behavior have visible or silent-safe fallbacks. Status messages, selection outlines, target regions, and challenge panels preserve necessary feedback without relying on sound.

## Integrated evidence

- Clean `verify-all.ps1`: tracking, hygiene, Debug/Release builds, Full 40/40 in 242.618 seconds, package creation, and the complete clean-package matrix passed on 2026-07-11. Evidence: `build/windows-x64/verification-bundle.log`.
- Full covers the 40-case fluid suite, 6000-tick stress, balanced/quality physics, visual/replay goldens, all three challenges, benchmarks, sandbox/lab smoke, and package launch. Portable software-renderer hashes are `0F0E76225C1838F5E25522C113DB2186D18072F7A03A0C3DC3DAB515A14A4D9D` (early surface), `833E62C4E77607D8BE196C22143F876051F8DE226C177845DAAF4AE253C3AA5D` (surface), and `BD07D6D324F8E3F0245D07C16829CF024A30B34DB62DAABE61EED8DAD34B47EB` (density).
- The packaged matrix loaded all eight gallery scenes, reproduced challenge completion at ticks 273/553/679 with exact digests, applied reduced motion + high contrast + mute, degraded safely without an audio device, and passed a fresh launch.
- The populated shipped omni scene ran continuously for 900.695 seconds and shut down normally at tick 26,542 with state digest `E663918632928B66`. Evidence: `build/windows-x64/integrated-acceptance/populated-soak.log` and `summary.json`.
- Component reviews remain recorded in `docs/particle-regularization-2026-07-11.md`, `docs/gallery-acceptance-2026-07-11.md`, `docs/challenge-acceptance-2026-07-11.md`, `docs/device-restoration-acceptance-2026-07-11.md`, and `docs/audio-acceptance-2026-07-11.md`.

No accepted feature bypasses the deterministic simulation command/snapshot/metrics boundaries, and audio/visual derivation does not alter state digests. No local P0/P1 implementation defect remains after this review.
