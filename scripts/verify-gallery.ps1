param(
    [string]$PackageRoot = '',
    [int]$CaptureTick = 60
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$runtimeRoot = if ($PackageRoot) { [System.IO.Path]::GetFullPath($PackageRoot) } else { $repoRoot }
$manifestPath = Join-Path $runtimeRoot 'gallery\gallery.manifest'
if (-not (Test-Path -LiteralPath $manifestPath)) { throw "Missing gallery manifest: $manifestPath" }

$header, $entryLines = @(Get-Content -LiteralPath $manifestPath)
if ($header -ne "physics-sim-gallery`t1") { throw 'Gallery manifest version must be 1.' }
$entries = @()
foreach ($line in $entryLines)
{
    if (-not $line -or $line.StartsWith('#')) { continue }
    $fields = $line.Split("`t")
    if ($fields.Count -ne 9 -or $fields[0] -ne 'entry') { throw "Malformed gallery entry: $line" }
    $entries += [pscustomobject]@{
        Id = $fields[1]
        Category = $fields[2]
        Order = [int]$fields[3]
        Scene = $fields[4]
        Thumbnail = $fields[5]
        Features = $fields[6]
        Title = $fields[7]
        Description = $fields[8]
    }
}
if ($entries.Count -lt 8) { throw "Gallery contains only $($entries.Count) entries." }
if (($entries.Id | Sort-Object -Unique).Count -ne $entries.Count) { throw 'Gallery contains duplicate IDs.' }
foreach ($category in @('learn', 'sandbox', 'challenges'))
{
    if ($category -notin $entries.Category) { throw "Gallery category is empty: $category" }
}

$exe = if ($PackageRoot) { Join-Path $runtimeRoot 'physics-sim.exe' } else { Join-Path $repoRoot 'build\windows-x64\Release\physics-sim.exe' }
if (-not (Test-Path -LiteralPath $exe)) { throw "Missing gallery runtime: $exe" }
$captureRoot = Join-Path $repoRoot 'build\windows-x64\gallery-validation'
New-Item -ItemType Directory -Path $captureRoot -Force | Out-Null
foreach ($entry in ($entries | Sort-Object Order))
{
    $scenePath = Join-Path $runtimeRoot $entry.Scene
    $thumbnailPath = Join-Path $runtimeRoot $entry.Thumbnail
    if (-not (Test-Path -LiteralPath $scenePath)) { throw "Missing gallery scene: $scenePath" }
    if (-not (Test-Path -LiteralPath $thumbnailPath)) { throw "Missing gallery thumbnail: $thumbnailPath" }
    if ((Get-Item -LiteralPath $thumbnailPath).Length -lt 1024) { throw "Gallery thumbnail is invalid: $thumbnailPath" }
    if (-not $entry.Description -or -not $entry.Features) { throw "Gallery metadata is incomplete: $($entry.Id)" }
    $capture = Join-Path $captureRoot "$($entry.Id).bmp"
    $log = Join-Path $captureRoot "$($entry.Id).log"
    $arguments = @('--mode', 'sandbox', '--skip-session-shell', '--scene-path', $scenePath,
        '--dump-frame', $capture, '--dump-frame-after-ticks', "$CaptureTick", '--auto-exit-ms', '1', '--log-file', $log)
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $runtimeRoot -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(30000)) { Stop-Process -Id $process.Id -Force; throw "Gallery load timed out: $($entry.Id)" }
    if ($process.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $capture)) { throw "Gallery load failed: $($entry.Id)" }
    Write-Host "[gallery] pass id=$($entry.Id) category=$($entry.Category)"
}
Write-Host "[gallery] passed entries=$($entries.Count) runtime=$runtimeRoot"
