# Release Packaging

The recovery prerelease is a runnable Windows x64 folder and matching ZIP created by `scripts\package-release.ps1`.

Package layout:

- `physics-sim.exe`
- `SDL2.dll`
- `README.md`
- `release-manifest.json`
- `SHA256SUMS.txt`
- `gallery\gallery.manifest` and eight thumbnails
- dependency license files
- eight curated scene-v2 files and three challenge replay-v2 files

Command:

```powershell
.\scripts\package-release.ps1
```

Validation:

- The script builds Release unless `-SkipBuild` is explicitly supplied.
- It copies the reviewed runtime, curated gallery/challenge content, release notes, verification summaries, and dependency licenses.
- Sandbox and lab smoke run with the package directory as the working directory.
- The executable also resolves bundled relative resources from its own directory, so Explorer and arbitrary-working-directory launches work.
- `release-manifest.json` records version, commit, platform, modes, and format versions; `SHA256SUMS.txt` records per-file hashes.
- The ZIP receives a separate `.sha256` sidecar in `dist`.
