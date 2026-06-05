# Replay Regression Harness

The replay harness drives deterministic, fixed-step actions through the app and captures frames for comparison.

## Command

Use the replay suite command:

```powershell
.\scripts\verify-replay-suite.ps1
```

The suite currently runs two replay cases:

- `demo-add-directional`
- `demo-add-omni`

Each case loads the committed demo scene, applies scripted actions at fixed simulation ticks, captures a frame, and compares the result with a golden BMP in `regression\`.

## Replay file format

Replay files use a simple text format:

```text
physics-sim-replay 1
tick 0 tool directional
tick 0 speed 8
tick 0 rate 192
tick 0 place 880 120
```

- The header declares the file format version.
- Each `tick` line schedules one command at a fixed-step tick count.
- Supported commands are `action`, `tool`, `direction`, `speed`, `rate`, and `place`.
- Blank lines and `#` comments are ignored.

## App option

Pass `--replay-file <path>` to load a replay script at startup.
Actions are applied in tick order during the fixed-step loop, which keeps the replay deterministic across runs.
