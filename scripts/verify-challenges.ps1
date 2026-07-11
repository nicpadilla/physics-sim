param(
    [ValidateSet('Debug', 'Release')] [string]$Configuration = 'Release',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -Configuration $Configuration }
$exe = Join-Path $repoRoot "build\windows-x64\$Configuration\physics-sim.exe"
$output = Join-Path $repoRoot 'build\windows-x64\challenge-review'
New-Item -ItemType Directory -Force $output | Out-Null
$cases = @(
    [ordered]@{ Name='fill'; Scene='objective_fill.pscene'; Replay='challenge_fill.replay'; Tick=273; Digest='D18349166749EEDB' },
    [ordered]@{ Name='gate'; Scene='challenge_gate.pscene'; Replay='challenge_gate.replay'; Tick=553; Digest='7EE30B971D89ADCD' },
    [ordered]@{ Name='pump-valve'; Scene='challenge_pump_valve.pscene'; Replay='challenge_pump_valve.replay'; Tick=679; Digest='E826B7B93FAAD25E' }
)
$results = @()
foreach ($case in $cases)
{
    $observed = @()
    foreach ($run in 1..2)
    {
        $log = Join-Path $output "$($case.Name)-run$run.log"
        $frame = Join-Path $output "$($case.Name)-run$run.bmp"
        Remove-Item $log,$frame -Force -ErrorAction SilentlyContinue
        $arguments = @('--skip-session-shell','--disable-audio','--scene-path',"scenes\$($case.Scene)",
            '--replay-file',"regression\replays\$($case.Replay)",'--log-file',$log,'--visual-mode','surface','--dump-frame',$frame,
            '--dump-frame-after-ticks','800','--auto-exit-ms','120000')
        $process = Start-Process $exe -ArgumentList $arguments -WorkingDirectory $repoRoot -PassThru
        if (-not $process.WaitForExit(180000)) { Stop-Process -Id $process.Id -Force; throw "challenge $($case.Name) timed out" }
        if ($process.ExitCode -ne 0) { throw "challenge $($case.Name) exited $($process.ExitCode)" }
        $match = [regex]::Match((Get-Content $log -Raw), 'challenge complete: tick=(\d+) state_digest=([0-9A-F]{16})')
        if (-not $match.Success) { throw "challenge $($case.Name) did not complete; log=$log" }
        $observed += [ordered]@{ tick=[uint64]$match.Groups[1].Value; digest=$match.Groups[2].Value; log=$log; frame=$frame }
    }
    $mismatch = ($observed[0].tick -ne $case.Tick) -or ($observed[0].digest -ne $case.Digest) -or
        ($observed[1].tick -ne $observed[0].tick) -or ($observed[1].digest -ne $observed[0].digest)
    if ($mismatch)
    { throw "challenge $($case.Name) determinism mismatch: $($observed | ConvertTo-Json -Compress)" }
    $results += [ordered]@{ name=$case.Name; completion_tick=$observed[0].tick; state_digest=$observed[0].digest; runs=$observed }
}
[ordered]@{ schema_version=1; generated_at=(Get-Date).ToString('o'); configuration=$Configuration; results=$results } |
    ConvertTo-Json -Depth 6 | Set-Content (Join-Path $output 'summary.json') -Encoding utf8
Write-Host "[challenges] success cases=$($results.Count) summary=$(Join-Path $output 'summary.json')"
