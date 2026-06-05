$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
$appLog = Join-Path $repoRoot 'build\windows-x64\tutorial-mode-app.log'
$settings = Join-Path $repoRoot 'build\windows-x64\tutorial-mode-settings.txt'

& (Join-Path $PSScriptRoot 'build.ps1')

foreach ($path in @($appLog, $settings))
{
    if (Test-Path -LiteralPath $path)
    {
        Remove-Item -LiteralPath $path -Force
    }
}

$previousLocation = Get-Location
try
{
    Set-Location -LiteralPath $repoRoot
    & $exe --log-file $appLog --settings-file $settings --tutorial-mode --auto-exit-ms 1500
}
finally
{
    Set-Location -LiteralPath $previousLocation
}

if ($LASTEXITCODE -ne 0)
{
    throw "physics-sim.exe returned exit code $LASTEXITCODE"
}

Start-Sleep -Milliseconds 500

for ($attempt = 0; $attempt -lt 20 -and -not (Test-Path -LiteralPath $appLog); $attempt++)
{
    Start-Sleep -Milliseconds 100
}

if (-not (Test-Path -LiteralPath $appLog))
{
    throw "Tutorial mode log was not written to $appLog"
}

$expectedLine = 'scene load ok: scenes/tutorial_intro.pscene'
$logText = Get-Content -LiteralPath $appLog -Raw
if ($logText -notmatch [regex]::Escape($expectedLine))
{
    throw "Tutorial mode did not log the expected scene load: $expectedLine"
}

Write-Host '[tutorial-mode] success'
