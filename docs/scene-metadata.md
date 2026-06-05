# Scene Metadata And Thumbnails

Scene files may include optional descriptive metadata alongside the physics data.

## Metadata fields

Version `1` scene files support these optional fields:

- `title`
- `description`
- `author`
- `tag`
- `note`

The fields are stored in the scene document and survive save/load round-trips.
Multiple `tag` and `note` lines are allowed.

## Thumbnail policy

Thumbnails are not embedded in the scene file.
Instead, gallery or browser tooling should treat a sibling BMP with the same scene stem as the thumbnail source.

Example:

- Scene: `scenes\demo_scene.pscene`
- Thumbnail: `scenes\demo_scene.thumb.bmp`

The repository includes a matching thumbnail sidecar for the committed demo scene.

This keeps the scene format simple while still leaving room for a gallery workflow later.
