param([switch]$SkipBuild)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
$log = Join-Path $repoRoot 'build\windows-x64\physics-sim-smoke.log'
$settings = Join-Path $repoRoot 'build\windows-x64\physics-sim-smoke-settings.txt'

if (-not $SkipBuild)
{
    & (Join-Path $PSScriptRoot 'build.ps1')
}

if (Test-Path -LiteralPath $settings)
{
    Remove-Item -LiteralPath $settings -Force
}

$previousLocation = Get-Location
try
{
    Set-Location -LiteralPath $repoRoot
    & $exe --log-file $log --settings-file $settings --skip-session-shell --auto-exit-ms 1500
}
finally
{
    Set-Location -LiteralPath $previousLocation
}
