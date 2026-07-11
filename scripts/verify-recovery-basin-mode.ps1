param(
    [Parameter(Mandatory)] [string]$Name,
    [Parameter(Mandatory)] [string]$Baseline,
    [Parameter(Mandatory)] [ValidateSet('particles', 'density', 'surface')] [string]$VisualMode,
    [Parameter(Mandatory)] [ValidateRange(1, 10000)] [int]$Tick,
    [Parameter(Mandatory)] [ValidateRange(1000, 120000)] [int]$AutoExitMilliseconds
)

$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
$repoRoot = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $repoRoot 'build\windows-x64\Debug\physics-sim.exe'
$baselinePath = Join-Path $repoRoot $Baseline
$artifactRoot = Join-Path $repoRoot 'build\windows-x64'
$capture = Join-Path $artifactRoot "recovery_basin_$Name-capture.bmp"
$result = Join-Path $artifactRoot "recovery_basin_$Name-result.json"
$log = Join-Path $artifactRoot "recovery_basin_$Name.log"
$appLog = Join-Path $artifactRoot "recovery_basin_$Name-app.log"
$settings = Join-Path $artifactRoot "recovery_basin_$Name-settings.txt"
$replay = Join-Path $repoRoot 'regression\replays\recovery-basin.replay'

Set-Content -LiteralPath $log -Value "[recovery-basin] name=$Name tick=$Tick mode=$VisualMode"
foreach ($required in @($exe, $baselinePath, $replay))
{
    if (-not (Test-Path -LiteralPath $required))
    {
        throw "[recovery-basin] required file missing: $required"
    }
}
Remove-Item -LiteralPath $capture, $settings -Force -ErrorAction SilentlyContinue

$process = Start-Process -FilePath $exe -ArgumentList @(
    '--log-file', $appLog,
    '--settings-file', $settings,
    '--disable-audio',
    '--software-renderer',
    '--skip-session-shell',
    '--scene-path', 'scenes/starter_basin.pscene',
    '--replay-file', $replay,
    '--visual-mode', $VisualMode,
    '--dump-frame', $capture,
    '--dump-frame-after-ticks', $Tick,
    '--auto-exit-ms', $AutoExitMilliseconds
) -Wait -PassThru -WorkingDirectory $repoRoot

if ($process.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $capture))
{
    throw "[recovery-basin] capture failed: name=$Name exit=$($process.ExitCode) artifact=$capture"
}

$baselineHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $baselinePath).Hash
$captureHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $capture).Hash
$matched = $baselineHash -eq $captureHash
@{
    schema_version = 1
    scenario = 'recovery-basin'
    view = $VisualMode
    profile = 'balanced'
    tick = $Tick
    baseline = $baselinePath
    capture = $capture
    baseline_sha256 = $baselineHash
    capture_sha256 = $captureHash
    matched = $matched
    result = if ($matched) { 'passed' } else { 'failed' }
} | ConvertTo-Json | Set-Content -LiteralPath $result -Encoding utf8

if (-not $matched)
{
    throw "[recovery-basin] mismatch: name=$Name baseline=$baselineHash capture=$captureHash artifacts=$capture,$result"
}

Remove-Item -LiteralPath $capture -Force
Add-Content -LiteralPath $log -Value "[recovery-basin] success hash=$captureHash"
Write-Host "[recovery-basin] name=$Name tick=$Tick mode=$VisualMode hash=$captureHash passed"
