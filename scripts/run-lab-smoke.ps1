param(
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$bundle = Join-Path $repoRoot 'build\windows-x64\lab-smoke'
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
if (-not $SkipBuild)
{
    & (Join-Path $PSScriptRoot 'build.ps1')
}
if (-not (Test-Path -LiteralPath $exe))
{
    throw "[lab-smoke] executable missing: $exe"
}
if (Test-Path -LiteralPath $bundle)
{
    Remove-Item -LiteralPath $bundle -Recurse -Force
}
$arguments = @('--mode', 'lab', '--capture-bundle', $bundle, '--auto-exit-ms', '5000')
$process = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru -WindowStyle Hidden
if (-not $process.WaitForExit(15000))
{
    Stop-Process -Id $process.Id -Force
    throw "[lab-smoke] timed out; retained artifact directory: $bundle"
}
if ($process.ExitCode -ne 0)
{
    throw "[lab-smoke] exit=$($process.ExitCode); retained artifact directory: $bundle"
}
$metricsPath = Join-Path $bundle 'metrics.json'
$imagePath = Join-Path $bundle 'frame.bmp'
if (-not (Test-Path -LiteralPath $metricsPath) -or -not (Test-Path -LiteralPath $imagePath))
{
    throw "[lab-smoke] incomplete capture bundle: $bundle"
}
$metrics = Get-Content -LiteralPath $metricsPath -Raw | ConvertFrom-Json
if ($metrics.tick -lt 10 -or -not $metrics.state_digest)
{
    throw "[lab-smoke] invalid metrics bundle: $metricsPath"
}
Write-Host "[lab-smoke] tick=$($metrics.tick) digest=$($metrics.state_digest) artifacts=$bundle"
