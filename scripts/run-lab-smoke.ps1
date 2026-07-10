param([switch]$SkipBuild)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$rootBundle = Join-Path $repoRoot 'build\windows-x64\lab-smoke'
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }
if (-not (Test-Path -LiteralPath $exe)) { throw "[lab-smoke] executable missing: $exe" }
if (Test-Path -LiteralPath $rootBundle) { Remove-Item -LiteralPath $rootBundle -Recurse -Force }
New-Item -ItemType Directory -Path $rootBundle -Force | Out-Null

$results = @()
for ($scenario = 0; $scenario -lt 10; $scenario++)
{
    $bundle = Join-Path $rootBundle ("scenario-{0:D2}" -f $scenario)
    $arguments = @('--mode', 'lab', '--scenario', $scenario, '--capture-bundle', $bundle, '--auto-exit-ms', '5000')
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru -WindowStyle Hidden
    if (-not $process.WaitForExit(15000))
    {
        Stop-Process -Id $process.Id -Force
        throw "[lab-smoke] scenario=$scenario timed out; artifacts=$bundle"
    }
    if ($process.ExitCode -ne 0) { throw "[lab-smoke] scenario=$scenario exit=$($process.ExitCode); artifacts=$bundle" }
    $metricsPath = Join-Path $bundle 'metrics.json'
    $imagePath = Join-Path $bundle 'frame.bmp'
    if (-not (Test-Path -LiteralPath $metricsPath) -or -not (Test-Path -LiteralPath $imagePath))
    {
        throw "[lab-smoke] scenario=$scenario incomplete bundle: $bundle"
    }
    $metrics = Get-Content -LiteralPath $metricsPath -Raw | ConvertFrom-Json
    if ($metrics.tick -lt 10 -or -not $metrics.state_digest -or -not $metrics.scenario)
    {
        throw "[lab-smoke] scenario=$scenario invalid metrics: $metricsPath"
    }
    if ($scenario -eq 1 -and $metrics.surface_cells_outside_allowed_region -ne 0)
    {
        throw "[lab-smoke] U-container surface escaped its allowed region: $metricsPath"
    }
    if ($scenario -eq 2 -and $metrics.surface_components -ne 1)
    {
        throw "[lab-smoke] still-pool surface was not one component: $metricsPath"
    }
    $results += [ordered]@{
        scenario_index = $scenario
        scenario = $metrics.scenario
        tick = $metrics.tick
        state_digest = $metrics.state_digest
        metrics = $metricsPath
        image = $imagePath
    }
    Write-Host "[lab-smoke] scenario=$scenario name=$($metrics.scenario) tick=$($metrics.tick) digest=$($metrics.state_digest)"
}
$results | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $rootBundle 'summary.json') -Encoding utf8
Write-Host "[lab-smoke] passed=10 artifacts=$rootBundle"
