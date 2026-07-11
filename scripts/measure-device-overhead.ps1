param([ValidateSet('Debug', 'Release')] [string]$Configuration = 'Release')

$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1') -Configuration $Configuration
if ($LASTEXITCODE -ne 0) { throw '[device-overhead] build failed' }
$exe = Join-Path (Split-Path -Parent $PSScriptRoot) "build\windows-x64\$Configuration\physics_sim_device_overhead_benchmark.exe"
& $exe
if ($LASTEXITCODE -ne 0) { throw '[device-overhead] 10 percent budget exceeded' }
