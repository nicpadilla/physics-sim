# Boundary Interaction

This document records the `PSIM-0072` wall and material interaction contract for the current 2D water solver.

## Default Wall Material

The default wall material is a near-free-slip water boundary:

- No penetration: `u dot n = u_solid dot n`.
- Restitution: `e = 0`, so water does not bounce off solid walls.
- Tangential damping: `mu = 0` by default, so tangential motion is preserved unless a later material issue adds friction.
- Static world walls, closed gates, and closed valves use the same no-penetration rule.

This matches the current axis-separated particle collision behavior: when a particle crosses a solid cell along one axis, the solver projects it to the solid face, zeroes the normal velocity component, and preserves the tangential component. If a particle reaches a corner or remains embedded after axis separation, the solver restores the previous safe position and clears velocity to preserve no-penetration determinism.

## Equations

Solid boundary condition:

```text
u dot n = u_solid dot n
```

Free-slip tangential condition:

```text
d(u_t) / dn = 0
```

Optional no-slip condition for future material work:

```text
u_t = u_solid,t
```

Particle contact projection:

```text
if phi(x_p) < r_p:
    x_p = x_p + (r_p - phi(x_p)) n
    v_n = min(v dot n, 0)
    v = v - (1 + e) v_n n - mu v_t
```

The current grid-cell implementation uses axis-separated cell-face contact instead of a continuous signed-distance field. That is an approximation, but it is deterministic and covered by wall impact, corner, narrow-channel, U-container, gate, and valve tests.

## Current Limitations

- Friction and no-slip material parameters are documented but not exposed as editable scene data.
- Moving solid velocity `u_solid` is effectively zero for current gates and valves because they toggle between open and closed states rather than moving continuously.
- The collision model is cell-face based, not a smooth signed-distance field.
- Foam, splashes, turbulence, and 3D wall wetting are out of scope for this 2D solver.

## Verification

The boundary contract is verified by:

- `physics_sim_water_tests` for solid collision, U-container retention, and finite stress behavior.
- `physics_sim_device_runtime_tests` for gate and valve runtime behavior.
- `physics_sim_fluid_quality_tests` scenarios for hose-wall pooling, wall impact, corner impact, narrow channel, and U-container fill.
- `scripts\verify-replay-suite.ps1` for replay determinism across scene/device interactions.
