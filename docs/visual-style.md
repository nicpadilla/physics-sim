# Visual Style Note

This note records the current visual direction for the sandbox so future polish work can stay coherent.

## Palette

- Water: cool cyan-blue with clear depth contrast.
- Solids and walls: dark neutral slate so water reads first.
- Selected fixtures: bright cyan outline with high contrast.
- Invalid placement: red/orange warning tint.
- Objective states: warm gold for goal feedback.
- Debug overlay: white and amber text on a dark translucent panel.

## Readability Rules

- Water, walls, and devices must remain readable in mixed, density, and particle views.
- Debug diagnostics should stay visually separated from the player-facing state summary.
- Hover, selection, disabled, active, and triggered states should be distinct without relying on motion alone.
- Panels, tool labels, and scene-state text should stay legible at the supported 80 x 45, 16 px demo scale.

## Rendering Boundaries

- The default view should look like a deliberate sandbox, not a debug capture.
- Particle and density modes remain available for diagnostics and regression work.
- UI feedback should not change simulation behavior or regression outputs.
