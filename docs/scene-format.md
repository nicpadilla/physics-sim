# Scene Format Policy

The current on-disk scene format version is `2`.

Version 1 stores:

- grid width, height, and cell size
- optional descriptive metadata fields for title, description, author, tags, and notes
- solid wall cells
- water emitters with kind, position, direction, speed, emission rate, and enabled state
- gates and valves with open/closed state
- sensors with enabled, active, objective, and label fields
- drains with rectangular footprints and enabled state
- pumps with rectangular footprints, enabled state, direction, and strength

Version 2 adds:

- optional `solver-profile <fast|balanced|quality>` scene-level solver profile

Thumbnail policy:

- Thumbnails are stored as sibling BMP files next to the scene rather than embedded in the scene text.
- See [scene-metadata.md](scene-metadata.md) for the metadata field list and thumbnail naming example.

Compatibility policy:

- The loader accepts version `1` and `2`.
- Version `1` scenes have no authored solver profile. Direct `load_scene` calls use the `balanced` fallback, while the app can pass the current user profile as the fallback.
- Version `2` scenes may specify `solver-profile`; if present, that profile overrides the user setting unless the app was launched with `--solver-profile`.
- Unknown, future, malformed, or missing versions are rejected.
- `save_scene` always writes version `2`.
- If the format changes in the future, the parser should be updated with an explicit compatibility or migration rule instead of silently accepting unknown data.

Failure behavior:

- `load_scene(path, simulation)` returns `false` if the file cannot be opened or the scene text is invalid.
- `save_scene(path, document)` returns `false` if the file cannot be created or written.
