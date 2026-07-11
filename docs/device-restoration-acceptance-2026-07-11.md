# Device restoration acceptance - 2026-07-11

PSIM-0109 restores directional and omni emitters, gates, sensors, drains, pumps, and valves to the recovered sandbox while preserving the compact pour/wall/erase first-run loop.

## Interaction acceptance

- Compact palette is the default and expands through `A` or its pointer header.
- Direct shortcuts 3-9 and Tab cycling select every advanced tool.
- Pointer placement selects the new device; existing devices can be reselected.
- `T` toggles enabled/open state, Delete removes, Q/E rotate directional devices, and +/- adjusts speed or pump strength.
- Undo, redo, scene-v2 save/load, and replay-v2 placement/toggle/delete cover every family.
- Drain and pump selection, editing, and deletion—previously missing—are now first-class.

- Reviewer: Codex device usability review for PSIM-0109
- Review date: 2026-07-11
- Artifact: `build/windows-x64/device-ui-review/advanced-tools.bmp`

## Numeric evidence

- Fast: 27/27 in 4.118 seconds.
- Standard: 29/29 in 30.736 seconds.
- Full fluid-quality suite: all 40 balanced/quality cases in 369.4 seconds.
- Device scenarios: drain-basin, pumped-loop, and valve-basin passed with zero mass-accounting error, zero particles in solids, and zero unexplained lifecycle changes.
- Release device overhead: 3.368246 ms baseline, 3.375441 ms with devices, 0.214% overhead; limit 10%.
- Identical device runs produce identical state digests and particle/mass metrics.
