# Scene Format Policy

The current on-disk scene format version is `1`.

Version 1 stores:

- grid width, height, and cell size
- optional descriptive metadata fields for title, description, author, tags, and notes
- solid wall cells
- water emitters with kind, position, direction, speed, emission rate, and enabled state
- gates and valves with open/closed state
- sensors with enabled, active, objective, and label fields
- drains with rectangular footprints and enabled state
- pumps with rectangular footprints, enabled state, direction, and strength

Thumbnail policy:

- Thumbnails are stored as sibling BMP files next to the scene rather than embedded in the scene text.
- See [scene-metadata.md](scene-metadata.md) for the metadata field list and thumbnail naming example.

Compatibility policy:

- The loader accepts version `1`.
- Unknown, future, malformed, or missing versions are rejected.
- `save_scene` always writes version `1`.
- If the format changes in the future, the parser should be updated with an explicit compatibility or migration rule instead of silently accepting unknown data.

Failure behavior:

- `load_scene(path, simulation)` returns `false` if the file cannot be opened or the scene text is invalid.
- `save_scene(path, document)` returns `false` if the file cannot be created or written.
