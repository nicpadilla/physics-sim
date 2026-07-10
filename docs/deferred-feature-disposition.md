# Deferred Feature Disposition

Decision date: 2026-07-10
Recovery target: `0.2.0-alpha.1`

The recovery package exposes one basin sandbox and the laboratory. Legacy implementation may remain in source for reference, but it is not release scope unless a new issue independently satisfies architecture, automated state, persistence, performance, and named player-review gates.

| Feature | Decision | Recovery use | Conditions for reconsideration |
| --- | --- | --- | --- |
| Directional and omni emitters | Redesign later | Lab-only scenario inputs; pointer pouring remains the sandbox source. | Command-based core API, deterministic replay v2 coverage, parameter budget, and lab acceptance. |
| Scene browser and broad gallery | Defer | No first-release sandbox navigation. Canonical scenarios belong in lab selection. | Curated basin value, thumbnail/accessibility review, load-failure coverage, and no dilution of the core loop. |
| Drains, pumps, valves, gates, and sensors | Defer as experimental lab devices | Useful for controlled engineering scenarios, not the first sandbox. | One issue per device family, isolated commands/metrics, v2 persistence, deterministic state tests, and performance evidence. |
| Objectives and challenge mode | Remove from recovery release | No current value to the creative basin loop or numeric lab. | A separately validated challenge design with measurable player value and no hidden solver coupling. |
| Tutorial expansion | Defer | The release keeps only pour, draw, erase, pause, reset, and save/load onboarding. | Named usability evidence showing a specific missing learning step. |
| Decorative audio and animation | Defer; keep functional feedback | Mute/volume, error, reset, save, and navigation feedback remain accessibility/clarity features. | Explicit frame/audio budget, lost-device behavior, reduced-motion compliance, and human acceptance. |
| Broader save management and progression | Remove from recovery release | One atomic current-scene save/load path is sufficient. | A demonstrated multi-scene workflow, conflict/recovery design, v2 schema, and keyboard-accessible acceptance. |

## Protection Rules

- The packaged scene set is limited to `starter_basin.pscene` and `tutorial_intro.pscene`; lab scenarios are programmatic and deterministic.
- Sandbox menus and tool cycling do not expose legacy gallery or device features, and shortcuts `3` through `9` plus gallery paging are rejected.
- A restoration issue must name its sandbox or lab value, command/state boundary, test surfaces, persistence representation, performance limit, and human review artifact before implementation.
- Restored work must rerun the sandbox smoke, lab smoke, Standard tier, package smoke, and the accepted vertical-slice visual check.
