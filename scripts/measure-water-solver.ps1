param(
    [Alias('Tier')]
    [ValidateSet('Fast', 'Balanced', 'Quality', 'All', 'Live', 'Offline')]
    [string]$Profile = 'All'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics_sim_solver_benchmark.exe'

if (-not (Test-Path -LiteralPath $exe))
{
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

switch ($Profile.ToLowerInvariant())
{
    'fast' {
        & $exe --profile fast
    }
    'balanced' {
        & $exe --profile balanced
    }
    'quality' {
        & $exe --profile quality
    }
    'all' {
        & $exe --profile all
    }
    'live' {
        & $exe --tier live
    }
    'offline' {
        & $exe --tier offline
    }
    default {
        throw "Unsupported solver profile selection: $Profile"
    }
}
