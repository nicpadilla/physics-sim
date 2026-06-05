param(
    [ValidateSet('Live', 'Offline', 'All')]
    [string]$Tier
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics_sim_solver_benchmark.exe'

if (-not (Test-Path -LiteralPath $exe))
{
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

if ($Tier)
{
    & $exe --tier $Tier.ToLowerInvariant()
}
else
{
    & $exe
}
