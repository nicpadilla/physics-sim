# Scene Gallery

Physics Sim ships with a small curated gallery of scene files under `scenes\`.

Browse the gallery in the app with `PgUp` and `PgDn`. The window title shows the current scene title, and the debug overlay flashes a short `MSG` line when a scene loads.

Gallery scenes:

| Scene | Purpose |
| --- | --- |
| `scenes\demo_scene.pscene` | U-container demo for the default regression and creative-flow baseline. |
| `scenes\free_fall.pscene` | A single hose drops water through open space. |
| `scenes\hose_wall_impact.pscene` | A horizontal hose strikes a wall and throws spray back. |
| `scenes\omni_spray.pscene` | An omni emitter fills a shallow basin. |
| `scenes\objective_fill.pscene` | An objective scene that fills the basin until a sensor triggers. |
| `scenes\future_device.pscene` | A placeholder slot for the next device type. |

Each scene carries its own metadata and purpose notes in the scene file itself.
The gallery scenes also ship with matching `.thumb.bmp` sidecars so future browser tooling can show previews using the same thumbnail policy as the demo scene.
