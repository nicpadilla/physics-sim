$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
$baseline = Join-Path $repoRoot 'regression\demo_scene_density_golden.bmp'
$capture = Join-Path $repoRoot 'build\windows-x64\demo_scene_density_regression_capture.bmp'
$log = Join-Path $repoRoot 'build\windows-x64\demo_scene_density_regression.log'
$appLog = Join-Path $repoRoot 'build\windows-x64\demo_scene_density_regression-app.log'
$settings = Join-Path $repoRoot 'build\windows-x64\demo_scene_density_regression-settings.txt'

function Write-Log
{
    param([string]$Message)
    Add-Content -LiteralPath $log -Value $Message
}

Set-Content -LiteralPath $log -Value "[demo-regression-density] starting"
Write-Log "[demo-regression-density] baseline: $baseline"
Write-Log "[demo-regression-density] capture:  $capture"

if (-not (Test-Path $exe))
{
    Write-Log "[demo-regression-density] missing exe"
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

if (-not (Test-Path $baseline))
{
    Write-Log "[demo-regression-density] missing baseline"
    throw "Missing regression baseline: $baseline"
}

if (Test-Path $capture)
{
    Remove-Item -LiteralPath $capture -Force
}

if (Test-Path -LiteralPath $settings)
{
    Remove-Item -LiteralPath $settings -Force
}

Write-Log "[demo-regression-density] launching app"
$process = Start-Process -FilePath $exe -ArgumentList @(
    '--log-file', $appLog,
    '--settings-file', $settings,
    '--skip-session-shell',
    '--visual-mode', 'density',
    '--dump-frame', $capture,
    '--dump-frame-after-ticks', '960',
    '--auto-exit-ms', '30000'
) -Wait -PassThru -WorkingDirectory $repoRoot
Write-Log "[demo-regression-density] app exit code: $($process.ExitCode)"

if ($process.ExitCode -ne 0)
{
    throw "physics-sim.exe returned exit code $($process.ExitCode)"
}

if (-not (Test-Path $capture))
{
    Write-Log "[demo-regression-density] capture missing"
    throw "Regression capture was not written to $capture"
}

Write-Log "[demo-regression-density] capture written"

$baselineHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $baseline).Hash
$captureHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $capture).Hash
Write-Log "[demo-regression-density] baseline hash: $baselineHash"
Write-Log "[demo-regression-density] capture hash:  $captureHash"

try
{
    if ($baselineHash -ne $captureHash)
    {
        Write-Log "[demo-regression-density] hash mismatch"
        throw "Density demo scene regression mismatch.`nBaseline: $baselineHash`nCapture:  $captureHash"
    }

    Write-Log "[demo-regression-density] hash match"
}
finally
{
    if (Test-Path $capture)
    {
        Remove-Item -LiteralPath $capture -Force
        Write-Log "[demo-regression-density] capture removed"
    }
}

Write-Log "[demo-regression-density] success"
