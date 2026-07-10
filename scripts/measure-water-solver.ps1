param(
    [Alias('Tier')]
    [ValidateSet('Fast', 'Balanced', 'Quality', 'All', 'Live', 'Offline')]
    [string]$Profile = 'All',
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $repoRoot 'build\windows-x64'
$bundledCMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmake = if ($cmakeCommand) { $cmakeCommand.Path } elseif (Test-Path $bundledCMake) { $bundledCMake } else { $null }
if (-not $cmake)
{
    throw 'Could not find CMake for the solver benchmark.'
}
& $cmake --build $buildDirectory --config $Configuration --target physics_sim_solver_benchmark
if ($LASTEXITCODE -ne 0)
{
    throw "Could not build the $Configuration solver benchmark."
}

$exe = Join-Path $buildDirectory "$Configuration\physics_sim_solver_benchmark.exe"

if (-not (Test-Path -LiteralPath $exe))
{
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

$arguments = switch ($Profile.ToLowerInvariant())
{
    'fast' {
        @('--profile', 'fast')
    }
    'balanced' {
        @('--profile', 'balanced')
    }
    'quality' {
        @('--profile', 'quality')
    }
    'all' {
        @('--profile', 'all')
    }
    'live' {
        @('--tier', 'live')
    }
    'offline' {
        @('--tier', 'offline')
    }
    default {
        throw "Unsupported solver profile selection: $Profile"
    }
}

$output = & $exe @arguments
$exitCode = $LASTEXITCODE
$output | Write-Host
if ($exitCode -ne 0)
{
    throw "Solver benchmark failed with exit code $exitCode."
}
if ($Configuration -eq 'Release')
{
    $balancedDemo = $output | Where-Object { $_ -match 'scene=demo-grid-flow\s+profile=balanced' } | Select-Object -Last 1
    if ($balancedDemo -and $balancedDemo -match 'average_step_ms=([0-9.]+)')
    {
        $averageStepMs = [double]::Parse($Matches[1], [Globalization.CultureInfo]::InvariantCulture)
        if ($averageStepMs -gt 8.3334)
        {
            throw "Balanced release solver exceeded the 120 Hz step budget: $averageStepMs ms > 8.3334 ms."
        }
    }
}
