$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$checker = Join-Path $PSScriptRoot 'check-version.ps1'
$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Raw).Trim()

& $checker -ExpectedTag "v$version"

$tagMismatchDetected = $false
try
{
    & $checker -ExpectedTag 'v999.999.999'
}
catch
{
    $tagMismatchDetected = $_.Exception.Message -match 'does not match authoritative version'
}
if (-not $tagMismatchDetected)
{
    throw '[version-test] mismatched tag was not rejected.'
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-version-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
try
{
    $missingReplayManifest = Join-Path $tempRoot 'missing-replay.manifest'
    Set-Content -LiteralPath $missingReplayManifest -Encoding utf8 -Value @(
        "physics-sim-replay-package`t1",
        "replay`tfill-goal`tmissing.replay",
        "replay`topen-gate`tchallenge_gate.replay",
        "replay`tpower-crossing`tchallenge_pump_valve.replay"
    )

    $missingReplayDetected = $false
    try
    {
        & $checker -ReplayManifest $missingReplayManifest
    }
    catch
    {
        $missingReplayDetected = $_.Exception.Message -match 'packaged replay is missing'
    }
    if (-not $missingReplayDetected)
    {
        throw '[version-test] missing manifest content was not rejected.'
    }

    $duplicateManifest = Join-Path $tempRoot 'duplicate-replay.manifest'
    Set-Content -LiteralPath $duplicateManifest -Encoding utf8 -Value @(
        "physics-sim-replay-package`t1",
        "replay`tfill-goal`tchallenge_fill.replay",
        "replay`tfill-goal`tchallenge_gate.replay",
        "replay`tpower-crossing`tchallenge_pump_valve.replay"
    )

    $duplicateDetected = $false
    try
    {
        & $checker -ReplayManifest $duplicateManifest
    }
    catch
    {
        $duplicateDetected = $_.Exception.Message -match 'duplicate packaged challenge id'
    }
    if (-not $duplicateDetected)
    {
        throw '[version-test] duplicate manifest identity was not rejected.'
    }
}
finally
{
    Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host '[version-test] matching metadata passed; tag mismatch, missing content, and duplicate identity were rejected'
