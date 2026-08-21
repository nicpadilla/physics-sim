# Physics Sim 0.2.0-alpha.3

Status: Unreleased

This prerelease follows the complete-water-sandbox recovery release and prepares the next correctness and maintainability increment.

## Release engineering changes

- `VERSION.txt` is the authoritative full version string.
- CMake derives its numeric project version and generated runtime version header from that file.
- The vcpkg manifest, README, package name, release manifest, and release-note selection are validated against the same version.
- Curated packaged scenes and thumbnails derive from `gallery/gallery.manifest`.
- Packaged challenge replays derive from `regression/replays/package.manifest`.
- Tag builds reject a tag that does not exactly match `v<VERSION.txt>`.

## Compatibility

- Scene format remains version 2.
- Replay format remains version 2.
- Recovered settings remain version 1.
- Windows x64 remains the packaged platform.

## Verification

The release package remains subject to Full verification, packaged Sandbox and Lab smoke tests, manifest completeness checks, and SHA-256 generation before publication.
