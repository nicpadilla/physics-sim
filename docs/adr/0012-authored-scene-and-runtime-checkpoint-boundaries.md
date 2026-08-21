# ADR 0012: Authored Scene And Runtime Checkpoint Boundaries

- Status: Accepted
- Date: 2026-08-20
- Decision owners: Physics Sim maintainers
- Related issues: #10, #11, #12, #13, #25

## Context

The current implementation uses `SceneDocument` for authored persistence and for `SceneController` undo history. That type describes walls and devices but does not contain all state needed to continue an active deterministic run. Applying it can clear particles and some counters while leaving other runtime values, such as a tick or derived sensor flag, inconsistent.

Scene loading, retry, clear-fluid, clear-scene, undo, redo, replay, and Sandbox-to-Lab handoff need separate and explicit state contracts.

## Decision

Physics Sim has three state domains. They must use distinct types or APIs and must not be substituted for one another.

### Authored scene definition

An authored scene definition contains only durable author intent:

- grid width, grid height, and cell size;
- selected solver profile;
- walls and authored device definitions;
- emitter configured kind, position, direction, speed, rate, and enabled state;
- gates, sensor regions and labels, drains, pumps, and valves;
- title, description, author, tags, notes, and challenge definition.

It does **not** contain:

- particles or grid velocity/pressure workspaces;
- simulation tick or elapsed simulation time;
- emitted, removed, or outflow counters;
- emitter fractional accumulation or emitted-particle phase;
- derived sensor activity;
- challenge progress or completion/failure state;
- pause or pending single-step state;
- current selection, hover, open menus, camera, or status messages;
- replay cursor or undo/redo history.

`SceneDocument` remains the authored-scene type. Scene v2 may continue parsing its legacy sensor-active token for compatibility, but loading must ignore that token and recompute activity from the new empty runtime.

### Deterministic runtime checkpoint

A runtime checkpoint contains every value required to resume the same deterministic run:

- the authored scene definition in effect;
- all particles, including mass, volume, density, affine velocity, and neighbor data;
- solid state and all future-affecting grid and solver fields;
- solver settings and fixed timestep;
- simulation tick and elapsed simulation time;
- lifecycle counters and metrics required by future challenge or device decisions;
- all devices and their runtime state;
- emitter fractional accumulation and emitted-particle phase;
- derived sensor state;
- challenge progress;
- pause, pending single step, and fixed-step accumulator;
- deterministic seed or equivalent deterministic identity, if exposed.

A checkpoint is an internal, versioned in-memory state type. It is not an authored scene file and has no cross-version persistence guarantee unless a later ADR defines one.

### Presentation and session state

Presentation state is not part of authored scene persistence or physics identity unless it changes future commands. It includes:

- selected tool and selected entity identifier;
- camera pan/zoom;
- hover state and open menus;
- tutorial presentation state;
- status messages and derived audio/visual effects;
- save-browser and gallery UI state.

Selection must use stable identifiers or validated indices. Raw pointers into mutable containers must not be stored in checkpoints.

## State transition contract

| Operation | Authored scene | Particles and solver fields | Tick/time and counters | Emitter runtime | Sensors | Challenge | Pause/step driver | Selection/UI | Replay cursor | Undo/redo history |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Load authored scene | Replace after full validation | Clear and initialize clean workspaces | Reset to zero | Reset accumulation and phase | Ignore serialized activity, recompute inactive/derived state | Reset | Reset accumulator and pending step; caller selects initial paused/running state | Clear invalid selection, preserve only safe global preferences | Reset | Replace with one initial runtime checkpoint |
| Failed scene load | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve | Preserve |
| Retry current scene | Restore the exact initially loaded authored definition | Clear and initialize | Reset to zero | Reset | Recompute | Reset | Reset accumulator/pending step | Clear selection and transient messages | Reset | Replace with one initial checkpoint |
| Clear fluid | Preserve | Remove particles and clear fluid fields while preserving walls/devices | Reset fluid lifecycle accounting and simulation tick | Reset accumulation and phase | Recompute from empty fluid | Reset scene-local challenge progress | Preserve paused/running choice, clear pending step and accumulator | Preserve safe selection only if its entity still exists | Preserve script definition but reset playback position | Preserve edit history only if checkpoints are updated to the cleared runtime |
| Clear scene | Replace with an empty definition | Clear | Reset | Remove | Remove | Reset/remove | Reset | Clear | Reset | Replace with one empty checkpoint |
| Undo edit | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Clear or validate selection after restore | Restore checkpoint when replay affects future commands | Move backward within bounded history |
| Redo edit | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Restore checkpoint | Clear or validate selection after restore | Restore checkpoint when replay affects future commands | Move forward within bounded history |
| Enter Lab for inspection | Preserve source definition | Copy/fork checkpoint; do not alias mutable source state | Preserve copied identity | Preserve copied identity | Preserve copied identity | Preserve copied identity | Preserve copied identity | Lab owns separate presentation state | Preserve identity metadata | Sandbox history remains owned by Sandbox |
| Return from Lab without applying fork | Preserve original | Restore or retain original exact checkpoint | Preserve original | Preserve original | Preserve original | Preserve original | Preserve original | Restore Sandbox presentation state where safe | Preserve original | Preserve original |

## Undo and redo policy

Undo and redo operate on complete deterministic runtime checkpoints, not `SceneDocument` values.

History is bounded by both:

- a maximum entry count, initially 64; and
- an estimated memory budget, initially 64 MiB.

When adding a checkpoint exceeds a bound, evict the oldest non-current entries in deterministic order. Always retain the current entry, even when one checkpoint alone exceeds the estimate. Branching after undo drops redo entries before appending the new checkpoint.

After restore, identical future commands and fixed steps must produce identical versioned runtime digests at multiple later checkpoints.

## Atomicity and failure rules

- Parse and validate an authored scene before mutating the current runtime.
- Reject non-finite, out-of-range, overlapping, or unsupported authored state without partial mutation.
- Restore a checkpoint only after its version, dimensions, and internal sizes validate.
- Failed load or restore leaves the current valid runtime, driver, challenge state, replay cursor, and history unchanged.

## Compatibility

- Scene v2 remains intentionally incompatible with pre-recovery scene v1.
- Existing scene v2 files remain readable.
- The legacy serialized sensor-active field is accepted but treated as non-authoritative derived data.
- Replay v2 remains readable. Changes to scene or runtime digest domains require an explicit digest version and documented baseline migration.
- Runtime checkpoints are internal and may change without scene-format compatibility.

## Required implementation tests

Dependent implementation work must add tests for:

1. loading a scene after hundreds of ticks produces the same initial runtime digest as loading into a fresh simulation;
2. failed load leaves the prior runtime, driver, challenge, replay, and history unchanged;
3. sensor activity is recomputed instead of trusted from scene text;
4. retry resets tick, counters, particles, emitter accumulation/phase, sensors, challenge progress, driver accumulator, and replay cursor;
5. undo after moving water restores particles, grid history, tick, counters, devices, emitter accumulation, sensors, challenge progress, and driver state;
6. redo restores the exact post-edit runtime digest;
7. equal restored checkpoints with equal future commands produce equal digests over several later ticks;
8. branch-after-undo removes redo entries;
9. history count and byte-budget eviction are deterministic;
10. different grid sizes and solver profiles cannot retain stale workspaces;
11. Sandbox-to-Lab round-trip without applying a fork returns the exact original digest.

## Consequences

- Checkpoints cost more memory than scene-only history, so bounded history and size accounting are required.
- Authored files become easier to reason about because they no longer claim to preserve transient runtime facts.
- Digests, replay validation, Lab handoff, and undo/redo can share one explicit future-state boundary.
- The app must coordinate driver, challenge, replay, and solver restore rather than relying on `apply_scene()` alone.
