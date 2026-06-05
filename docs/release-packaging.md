# Release Packaging

The current release package is a runnable folder created by `scripts\package-release.ps1`.

Package layout:

- `physics-sim.exe`
- `SDL2d.dll`
- `README.md`
- `package-manifest.txt`
- `scenes\demo_scene.pscene`

Command:

```powershell
.\scripts\package-release.ps1
```

Validation:

- The script runs `build.ps1` first.
- It copies the runtime DLLs from `build\windows-x64\Debug`.
- It validates that the expected runtime files exist in `dist\physics-sim-release`.
- `package-manifest.txt` lists the packaged files and their SHA-256 hashes.
