# PSIM-0112 Audio Acceptance — 2026-07-11

Named review: **Codex water and device audio listening review for PSIM-0112**

Result: Accepted.

The one-second deterministic render at `build/windows-x64/audio-review/all-layers.wav` combines the maximum seven simultaneous continuous voices: pour, bulk flow, impact, pump, drain, open fixture, and active objective. Listening review accepted the low, restrained water bed; distinct pump tone; brief turbulent texture; unobtrusive objective harmony; click-free starts and releases; and the absence of obvious short-loop repetition. Existing interaction cues remain short and visually duplicated by status messages, highlights, and challenge progress.

The procedural source is repository-owned code and requires no third-party sound asset license. Layer intensity is derived from tick-indexed emitted/outflow counts, kinetic energy, divergence, enabled device state, and active sensors. Audio state never enters or mutates the simulation core. Reduced-motion mode leaves necessary sound feedback enabled; mute and zero master/effects volume clear queued audio and produce exact silence.

Automated evidence in `build/windows-x64/audio-review/summary.json` records peak, RMS, hash, voice count, and sample format. `audio-test.txt` records Release CPU cost and validates attack time, release continuity, deterministic output, mute, clipping, and the voice cap. `missing-device.log` confirms an unavailable SDL output driver degrades to silent operation with a readable warning and exit code 0. SDL device-added/device-removed events close and reopen output without stopping simulation.
