$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
$baseline = Join-Path $repoRoot 'regression\demo_scene_surface_golden.bmp'
$capture = Join-Path $repoRoot 'build\windows-x64\demo_scene_surface_regression_capture.bmp'
$log = Join-Path $repoRoot 'build\windows-x64\demo_scene_surface_regression.log'
$appLog = Join-Path $repoRoot 'build\windows-x64\demo_scene_surface_regression-app.log'
$settings = Join-Path $repoRoot 'build\windows-x64\demo_scene_surface_regression-settings.txt'
$toolReplay = Join-Path $repoRoot 'regression\replays\demo-tool-wall.replay'

function Write-Log
{
    param([string]$Message)
    Add-Content -LiteralPath $log -Value $Message
}

Set-Content -LiteralPath $log -Value "[demo-regression-surface] starting"
Write-Log "[demo-regression-surface] baseline: $baseline"
Write-Log "[demo-regression-surface] capture:  $capture"

if (-not (Test-Path $exe))
{
    Write-Log "[demo-regression-surface] missing exe"
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

if (-not (Test-Path $baseline))
{
    Write-Log "[demo-regression-surface] missing baseline"
    throw "Missing regression baseline: $baseline"
}

if (-not (Test-Path $toolReplay))
{
    Write-Log "[demo-regression-surface] missing tool replay"
    throw "Missing regression tool replay: $toolReplay"
}

if (Test-Path $capture)
{
    Remove-Item -LiteralPath $capture -Force
}

if (Test-Path -LiteralPath $settings)
{
    Remove-Item -LiteralPath $settings -Force
}

Write-Log "[demo-regression-surface] launching app"
$process = Start-Process -FilePath $exe -ArgumentList @(
    '--log-file', $appLog,
    '--settings-file', $settings,
    '--skip-session-shell',
    '--scene-path', 'scenes/demo_scene.pscene',
    '--replay-file', $toolReplay,
    '--visual-mode', 'surface',
    '--dump-frame', $capture,
    '--dump-frame-after-ticks', '2400',
    '--auto-exit-ms', '60000'
) -Wait -PassThru -WorkingDirectory $repoRoot
Write-Log "[demo-regression-surface] app exit code: $($process.ExitCode)"

if ($process.ExitCode -ne 0)
{
    throw "physics-sim.exe returned exit code $($process.ExitCode)"
}

if (-not (Test-Path $capture))
{
    Write-Log "[demo-regression-surface] capture missing"
    throw "Regression capture was not written to $capture"
}

Write-Log "[demo-regression-surface] capture written"

$baselineHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $baseline).Hash
$captureHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $capture).Hash
Write-Log "[demo-regression-surface] baseline hash: $baselineHash"
Write-Log "[demo-regression-surface] capture hash:  $captureHash"

try
{
    if ($baselineHash -ne $captureHash)
    {
        Write-Log "[demo-regression-surface] hash mismatch"
        throw "Surface demo scene regression mismatch.`nBaseline: $baselineHash`nCapture:  $captureHash"
    }

    Write-Log "[demo-regression-surface] hash match"
}
finally
{
    if (Test-Path $capture)
    {
        Remove-Item -LiteralPath $capture -Force
        Write-Log "[demo-regression-surface] capture removed"
    }
}

Write-Log "[demo-regression-surface] success"
