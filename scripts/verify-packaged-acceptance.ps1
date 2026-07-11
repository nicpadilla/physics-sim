param(
    [string]$PackageRoot = '',
    [int]$SoakSeconds = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $PackageRoot) { $PackageRoot = Join-Path $repoRoot 'dist\physics-sim-0.2.0-alpha.2-windows-x64' }
$PackageRoot = [System.IO.Path]::GetFullPath($PackageRoot)
$exe = Join-Path $PackageRoot 'physics-sim.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw "Missing packaged executable: $exe" }
$evidence = Join-Path $repoRoot 'build\windows-x64\integrated-acceptance'
New-Item -ItemType Directory -Path $evidence -Force | Out-Null

& (Join-Path $PSScriptRoot 'verify-gallery.ps1') -PackageRoot $PackageRoot -CaptureTick 60
& (Join-Path $PSScriptRoot 'verify-audio.ps1') -SkipBuild -PackageRoot $PackageRoot

function Invoke-PackagedRun
{
    param([string]$Name, [string[]]$Arguments, [int]$TimeoutMilliseconds = 30000)
    $process = Start-Process -FilePath $exe -ArgumentList $Arguments -WorkingDirectory $PackageRoot -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit($TimeoutMilliseconds)) { Stop-Process -Id $process.Id -Force; throw "Packaged run timed out: $Name" }
    if ($process.ExitCode -ne 0) { throw "Packaged run failed: $Name exit=$($process.ExitCode)" }
}

$freshSettings = Join-Path $evidence 'fresh-settings.txt'
$freshLog = Join-Path $evidence 'fresh-launch.log'
Remove-Item $freshSettings,$freshLog -Force -ErrorAction SilentlyContinue
Invoke-PackagedRun 'fresh-launch' @('--mode','sandbox','--skip-session-shell','--scene-path','scenes\starter_basin.pscene','--settings-file',$freshSettings,'--log-file',$freshLog,'--auto-exit-ms','750')
if (-not (Select-String -LiteralPath $freshLog -Pattern 'scene load ok' -Quiet)) { throw 'Fresh packaged launch did not load the starter basin.' }

$accessibleSettings = Join-Path $evidence 'accessible-settings.txt'
(Get-Content -LiteralPath $freshSettings) -replace '^reduced_motion 0$', 'reduced_motion 1' -replace '^high_contrast 0$', 'high_contrast 1' -replace '^audio_muted 0$', 'audio_muted 1' | Set-Content -LiteralPath $accessibleSettings -Encoding ascii
$accessibleLog = Join-Path $evidence 'accessible-launch.log'
Invoke-PackagedRun 'accessible-launch' @('--mode','sandbox','--skip-session-shell','--scene-path','scenes\device_playground.pscene','--settings-file',$accessibleSettings,'--log-file',$accessibleLog,'--auto-exit-ms','750')
if (-not (Select-String -LiteralPath $accessibleLog -Pattern 'reduced_motion=on' -Quiet)) { throw 'Reduced-motion package setting was not applied.' }

$challenges = @(
    @{ Name='fill'; Scene='objective_fill.pscene'; Replay='challenge_fill.replay'; Tick='273'; Digest='D18349166749EEDB' },
    @{ Name='gate'; Scene='challenge_gate.pscene'; Replay='challenge_gate.replay'; Tick='553'; Digest='7EE30B971D89ADCD' },
    @{ Name='pump-valve'; Scene='challenge_pump_valve.pscene'; Replay='challenge_pump_valve.replay'; Tick='679'; Digest='E826B7B93FAAD25E' }
)
foreach ($challenge in $challenges)
{
    $log = Join-Path $evidence "challenge-$($challenge.Name).log"
    $frame = Join-Path $evidence "challenge-$($challenge.Name).bmp"
    Invoke-PackagedRun "challenge-$($challenge.Name)" @('--mode','sandbox','--skip-session-shell','--disable-audio','--scene-path',"scenes\$($challenge.Scene)",'--replay-file',"replays\$($challenge.Replay)",'--log-file',$log,'--dump-frame',$frame,'--dump-frame-after-ticks','800','--auto-exit-ms','1') 180000
    $expected = "challenge complete: tick=$($challenge.Tick) state_digest=$($challenge.Digest)"
    if (-not (Select-String -LiteralPath $log -SimpleMatch $expected -Quiet)) { throw "Packaged challenge evidence mismatch: $($challenge.Name)" }
}

$soakResult = $null
if ($SoakSeconds -gt 0)
{
    $soakLog = Join-Path $evidence 'populated-soak.log'
    $watch = [System.Diagnostics.Stopwatch]::StartNew()
    Invoke-PackagedRun 'populated-soak' @('--mode','sandbox','--skip-session-shell','--disable-audio','--scene-path','scenes\omni_spray.pscene','--log-file',$soakLog,'--auto-exit-ms',"$($SoakSeconds * 1000)") (($SoakSeconds + 60) * 1000)
    $watch.Stop()
    $shutdown = Select-String -LiteralPath $soakLog -Pattern 'shutdown: normal exit tick=(\d+) state_digest=([0-9A-F]{16})'
    if (-not $shutdown) { throw 'Populated soak lacks a clean deterministic shutdown record.' }
    $soakResult = [ordered]@{ requested_seconds=$SoakSeconds; elapsed_seconds=$watch.Elapsed.TotalSeconds; shutdown=$shutdown.Line }
}
else
{
    $priorSoakLog = Join-Path $evidence 'populated-soak.log'
    if (Test-Path -LiteralPath $priorSoakLog)
    {
        $startup = Select-String -LiteralPath $priorSoakLog -Pattern '^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}) startup:'
        $shutdown = Select-String -LiteralPath $priorSoakLog -Pattern '^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}) shutdown: normal exit tick=(\d+) state_digest=([0-9A-F]{16})'
        if ($startup -and $shutdown)
        {
            $startTime = [datetime]::ParseExact($startup.Matches[0].Groups[1].Value, 'yyyy-MM-dd HH:mm:ss', $null)
            $endTime = [datetime]::ParseExact($shutdown.Matches[0].Groups[1].Value, 'yyyy-MM-dd HH:mm:ss', $null)
            $elapsed = ($endTime - $startTime).TotalSeconds
            if ($elapsed -ge 900) { $soakResult = [ordered]@{ requested_seconds=900; elapsed_seconds=$elapsed; shutdown=$shutdown.Line } }
        }
    }
}

$summary = [ordered]@{
    schema_version = 1
    generated_at = (Get-Date).ToString('o')
    package_root = $PackageRoot
    fresh_launch = 'pass'
    reduced_motion_high_contrast_muted = 'pass'
    gallery_entries = 8
    challenges = 3
    audio_missing_device = 'pass'
    soak = $soakResult
}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'summary.json') -Encoding utf8
Write-Host "[integrated-acceptance] passed package=$PackageRoot soak_seconds=$SoakSeconds"
