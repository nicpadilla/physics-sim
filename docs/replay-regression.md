# Recovery Replay Contract

Replay version 2 stores semantic, tick-indexed commands rather than SDL events or render-frame timing.

Required header:

```text
physics-sim-replay 2
scene-digest 0E70DEFE66E332D7
fixed-timestep 0.008333333333333333
solver-profile balanced
```

An optional `expected-final-digest` records the accepted terminal simulation identity. Each event uses `tick <tick> <command> [arguments...]`; current recovery parsing supports semantic action, tool, direction, speed, rate, and placement commands while the application is migrated to the stable simulation facade.

The scene digest is deterministic FNV-1a 64 over scene bytes after normalizing CRLF to LF, so Git line-ending conversion does not invalidate identity. Replay startup fails before simulation if scene digest, fixed timestep, or solver profile differs. Version 1, missing identity, malformed commands, and unknown versions are rejected.

Image hashes remain determinism evidence only. Recovery acceptance additionally requires structured numeric and semantic visual evidence.
