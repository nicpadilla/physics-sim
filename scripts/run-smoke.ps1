$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'

& (Join-Path $PSScriptRoot 'build.ps1')

& $exe --auto-exit-ms 1500
