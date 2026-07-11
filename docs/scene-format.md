# Recovery Scene Format

Recovery scenes use text format version 2 and begin with:

```text
physics-sim-scene 2
solver-profile balanced
grid 80 45 16
```

Version 1 and unknown versions are intentionally rejected. There is no migration path in the recovery executable; legacy content remains available through the pre-recovery Git tag.

Required records:

- exactly one `physics-sim-scene 2` header;
- one `solver-profile fast|balanced|quality` record;
- one positive `grid <width> <height> <cell-size>` record.

Optional metadata records are `title`, `description`, `author`, repeated `tag`, repeated `note`, and an optional `challenge` contract. Authored state records are `wall`, `emitter`, `gate`, `sensor`, `drain`, `pump`, and `valve`; all are available in the recovered sandbox Advanced Tools palette.

Player saves use atomic replacement:

1. write `<path>.tmp`;
2. parse and validate the temporary scene;
3. move the previous valid scene to `<path>.bak`;
4. rename the validated temporary scene to the requested path;
5. restore the backup if publication fails.

Malformed headers, missing required records, invalid profiles, non-positive grids, invalid devices, and unknown records are rejected without applying partial state.
