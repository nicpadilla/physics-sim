param(
    [switch]$SkipBuild,
    [string]$PackageRoot = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $repoRoot 'build\windows-x64'
$releaseDir = Join-Path $buildRoot 'Release'
$evidenceDir = Join-Path $buildRoot 'audio-review'
New-Item -ItemType Directory -Path $evidenceDir -Force | Out-Null
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -Configuration Release }

$testExe = Join-Path $releaseDir 'physics_sim_audio_feedback_tests.exe'
$rawPath = Join-Path $evidenceDir 'all-layers.f32'
$wavPath = Join-Path $evidenceDir 'all-layers.wav'
$testOutput = Join-Path $evidenceDir 'audio-test.txt'
& $testExe --dump-raw-f32 $rawPath | Tee-Object -FilePath $testOutput
if ($LASTEXITCODE -ne 0) { throw "Audio test failed with exit $LASTEXITCODE." }

$raw = [System.IO.File]::ReadAllBytes($rawPath)
if (($raw.Length % 4) -ne 0) { throw 'Raw audio evidence has an invalid length.' }
$sampleCount = [int]($raw.Length / 4)
$peak = 0.0
$sumSquares = 0.0
$pcm = New-Object byte[] ($sampleCount * 2)
for ($i = 0; $i -lt $sampleCount; $i++)
{
    $sample = [BitConverter]::ToSingle($raw, $i * 4)
    $peak = [Math]::Max($peak, [Math]::Abs($sample))
    $sumSquares += $sample * $sample
    $integer = [int16][Math]::Round([Math]::Max(-1.0, [Math]::Min(1.0, $sample)) * 32767.0)
    $bytes = [BitConverter]::GetBytes($integer)
    $pcm[$i * 2] = $bytes[0]
    $pcm[$i * 2 + 1] = $bytes[1]
}
$stream = [System.IO.File]::Open($wavPath, [System.IO.FileMode]::Create)
$writer = [System.IO.BinaryWriter]::new($stream)
try
{
    $writer.Write([Text.Encoding]::ASCII.GetBytes('RIFF'))
    $writer.Write([int](36 + $pcm.Length))
    $writer.Write([Text.Encoding]::ASCII.GetBytes('WAVEfmt '))
    $writer.Write([int]16); $writer.Write([int16]1); $writer.Write([int16]1)
    $writer.Write([int]48000); $writer.Write([int]96000); $writer.Write([int16]2); $writer.Write([int16]16)
    $writer.Write([Text.Encoding]::ASCII.GetBytes('data'))
    $writer.Write([int]$pcm.Length); $writer.Write($pcm)
}
finally
{
    $writer.Dispose()
    $stream.Dispose()
}
Remove-Item -LiteralPath $rawPath -Force

$runtimeRoot = if ($PackageRoot) { [System.IO.Path]::GetFullPath($PackageRoot) } else { $repoRoot }
$gameExe = if ($PackageRoot) { Join-Path $runtimeRoot 'physics-sim.exe' } else { Join-Path $releaseDir 'physics-sim.exe' }
$silentLog = Join-Path $evidenceDir 'missing-device.log'
$previousDriver = $env:SDL_AUDIODRIVER
try
{
    $env:SDL_AUDIODRIVER = 'physics-sim-intentionally-missing'
    $process = Start-Process -FilePath $gameExe -ArgumentList @('--mode','sandbox','--skip-session-shell','--auto-exit-ms','500','--log-file',$silentLog) -WorkingDirectory $runtimeRoot -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(15000)) { Stop-Process -Id $process.Id -Force; throw 'Missing-device run timed out.' }
    if ($process.ExitCode -ne 0) { throw "Missing-device run failed with exit $($process.ExitCode)." }
}
finally
{
    $env:SDL_AUDIODRIVER = $previousDriver
}
if (-not (Select-String -LiteralPath $silentLog -Pattern 'audio unavailable' -Quiet)) { throw 'Missing-device recovery was not logged.' }

$summary = [ordered]@{
    schema_version = 1
    generated_at = (Get-Date).ToString('o')
    sample_rate = 48000
    channels = 1
    duration_seconds = $sampleCount / 48000.0
    peak = $peak
    peak_dbfs = 20.0 * [Math]::Log10([Math]::Max($peak, 1e-12))
    rms = [Math]::Sqrt($sumSquares / $sampleCount)
    voice_cap = 8
    active_test_voices = 7
    deterministic_procedural_source = $true
    wav_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $wavPath).Hash.ToLowerInvariant()
    missing_device_exit = 0
    runtime_root = $runtimeRoot
}
$summary | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath (Join-Path $evidenceDir 'summary.json') -Encoding utf8
Write-Host "[audio] passed peak_dbfs=$([Math]::Round($summary.peak_dbfs, 3)) wav=$wavPath"
