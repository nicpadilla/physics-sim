param(
    [switch]$SkipBuild,
    [switch]$Canonical
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$rootBundle = Join-Path $repoRoot $(if ($Canonical) { 'build\windows-x64\lab-canonical' } else { 'build\windows-x64\lab-smoke' })
$configuration = if ($Canonical) { 'Release' } else { 'Debug' }
$exe = Join-Path $repoRoot "build\windows-x64\$configuration\physics-sim.exe"
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -Configuration $configuration }
if (-not (Test-Path -LiteralPath $exe)) { throw "[lab-smoke] executable missing: $exe" }
if (Test-Path -LiteralPath $rootBundle) { Remove-Item -LiteralPath $rootBundle -Recurse -Force }
New-Item -ItemType Directory -Path $rootBundle -Force | Out-Null

$results = @()
$canonicalTicks = @(10, 2400, 1200, 1200, 480, 360, 360, 8, 6000, 2400)
for ($scenario = 0; $scenario -lt 10; $scenario++)
{
    $bundle = Join-Path $rootBundle ("scenario-{0:D2}" -f $scenario)
    $captureTick = if ($Canonical) { $canonicalTicks[$scenario] } else { 10 }
    $arguments = @('--mode', 'lab', '--scenario', $scenario, '--capture-tick', $captureTick, '--capture-bundle', $bundle, '--auto-exit-ms', '30000')
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru -WindowStyle Hidden
    if (-not $process.WaitForExit($(if ($Canonical) { 60000 } else { 15000 })))
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
    if ($metrics.tick -ne $captureTick -or -not $metrics.state_digest -or -not $metrics.scenario)
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
    if ($Canonical -and $scenario -eq 1 -and
        ($metrics.surface_components -ne 1 -or $metrics.isolated_surface_cells -ne 0 -or [double]$metrics.kinetic_energy -gt 50000.0))
    {
        throw "[lab-smoke] settled U-container failed semantic/energy gates: $metricsPath"
    }
    if ($Canonical -and $scenario -eq 6 -and
        ($metrics.surface_components -gt 2 -or $metrics.isolated_surface_cells -ne 0))
    {
        throw "[lab-smoke] narrow-channel surface fragmented: $metricsPath"
    }
    if ($Canonical -and $scenario -eq 8 -and
        ($metrics.surface_components -ne 1 -or $metrics.isolated_surface_cells -ne 0 -or [double]$metrics.kinetic_energy -gt 100.0))
    {
        throw "[lab-smoke] long-run surface/energy gate failed: $metricsPath"
    }
    foreach ($metricName in @('pressure_residual', 'average_density_error', 'kinetic_energy'))
    {
        $value = [double]$metrics.$metricName
        if ([double]::IsNaN($value) -or [double]::IsInfinity($value))
        {
            throw "[lab-smoke] scenario=$scenario non-finite $metricName=$value artifacts=$bundle"
        }
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
Write-Host "[lab-smoke] passed=10 tier=$(if ($Canonical) { 'canonical' } else { 'quick' }) artifacts=$rootBundle"
