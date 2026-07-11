# Release Packaging

The recovery prerelease is a runnable Windows x64 folder and matching ZIP created by `scripts\package-release.ps1`.

Package layout:

- `physics-sim.exe`
- `SDL2.dll`
- `README.md`
- `package-manifest.txt`
- `SHA256SUMS.txt`
- `VERSION.txt`
- dependency license files
- `scenes\starter_basin.pscene`
- `scenes\tutorial_intro.pscene`

Command:

```powershell
.\scripts\package-release.ps1
```

Validation:

- The script builds Release unless `-SkipBuild` is explicitly supplied.
- It copies only the recovery runtime, basin/tutorial content, documentation, and dependency licenses.
- Sandbox and lab smoke run with the package directory as the working directory.
- The executable also resolves bundled relative resources from its own directory, so Explorer and arbitrary-working-directory launches work.
- `package-manifest.txt` records version, commit, toolchain evidence, and contents; `SHA256SUMS.txt` records per-file hashes.
- The ZIP receives a separate `.sha256` sidecar in `dist`.
