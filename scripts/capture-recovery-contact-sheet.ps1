param(
    [ValidateRange(10, 6000)]
    [int]$Tick = 60,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$repoRoot = Split-Path -Parent $PSScriptRoot
$outputRoot = Join-Path $repoRoot 'build\windows-x64\recovery-visual-review'
$exe = Join-Path $repoRoot 'build\windows-x64\Release\physics-sim.exe'
if (-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') -Configuration Release }
if (-not (Test-Path -LiteralPath $exe)) { throw "[contact-sheet] Release executable missing: $exe" }
if (Test-Path -LiteralPath $outputRoot) { Remove-Item -LiteralPath $outputRoot -Recurse -Force }
New-Item -ItemType Directory -Path $outputRoot -Force | Out-Null

$scenarios = @(
    [ordered]@{ Index = 1; Name = 'U-container' },
    [ordered]@{ Index = 2; Name = 'Still pool' },
    [ordered]@{ Index = 3; Name = 'Hydrostatic column' },
    [ordered]@{ Index = 4; Name = 'Dam break' },
    [ordered]@{ Index = 5; Name = 'Wall and corner impact' },
    [ordered]@{ Index = 6; Name = 'Narrow channel' },
    [ordered]@{ Index = 8; Name = 'Long run' }
)
$captures = @()
foreach ($scenario in $scenarios)
{
    $bundle = Join-Path $outputRoot ("scenario-{0:D2}" -f $scenario.Index)
    $arguments = @('--mode', 'lab', '--scenario', $scenario.Index, '--field', 'surface', '--capture-tick', $Tick,
        '--capture-bundle', $bundle, '--auto-exit-ms', '30000')
    $process = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru -WindowStyle Hidden
    if (-not $process.WaitForExit(60000))
    {
        Stop-Process -Id $process.Id -Force
        throw "[contact-sheet] $($scenario.Name) timed out; artifacts=$bundle"
    }
    if ($process.ExitCode -ne 0) { throw "[contact-sheet] $($scenario.Name) exit=$($process.ExitCode); artifacts=$bundle" }
    $metricsPath = Join-Path $bundle 'metrics.json'
    $imagePath = Join-Path $bundle 'frame.bmp'
    if (-not (Test-Path -LiteralPath $metricsPath) -or -not (Test-Path -LiteralPath $imagePath))
    {
        throw "[contact-sheet] incomplete capture for $($scenario.Name): $bundle"
    }
    $metrics = Get-Content -LiteralPath $metricsPath -Raw | ConvertFrom-Json
    $captures += [ordered]@{
        scenario = $scenario.Name
        tick = $metrics.tick
        digest = $metrics.state_digest
        profile = $metrics.solver_profile
        metrics = $metricsPath
        image = $imagePath
    }
}

$cellWidth = 640
$cellHeight = 420
$sheet = New-Object System.Drawing.Bitmap ($cellWidth * 2), ($cellHeight * 4)
$graphics = [System.Drawing.Graphics]::FromImage($sheet)
$graphics.Clear([System.Drawing.Color]::FromArgb(10, 15, 25))
$font = New-Object System.Drawing.Font 'Segoe UI', 14, ([System.Drawing.FontStyle]::Bold)
$brush = [System.Drawing.Brushes]::White
try
{
    for ($index = 0; $index -lt $captures.Count; $index++)
    {
        $column = $index % 2
        $row = [math]::Floor($index / 2)
        $x = $column * $cellWidth
        $y = $row * $cellHeight
        $captureImage = [System.Drawing.Image]::FromFile($captures[$index].image)
        try
        {
            $graphics.DrawImage($captureImage, $x, $y + 34, $cellWidth, $cellHeight - 34)
        }
        finally { $captureImage.Dispose() }
        $label = "$($captures[$index].scenario) | tick $($captures[$index].tick) | $($captures[$index].profile)"
        $graphics.DrawString($label, $font, $brush, $x + 8, $y + 7)
    }
    $sheetPath = Join-Path $outputRoot 'recovery-contact-sheet.bmp'
    $sheet.Save($sheetPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
}
finally
{
    $font.Dispose()
    $graphics.Dispose()
    $sheet.Dispose()
}

$review = [ordered]@{
    generated_at = (Get-Date).ToString('o')
    tick = $Tick
    numeric_evidence = 'build/windows-x64/fluid-quality-suite/summary.json'
    contact_sheet = (Join-Path $outputRoot 'recovery-contact-sheet.bmp')
    review_status = 'Pending named human review'
    reviewer = $null
    reviewed_at = $null
    captures = $captures
}
$review | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $outputRoot 'review-manifest.json') -Encoding utf8
Write-Host "[contact-sheet] sheet=$($review.contact_sheet) manifest=$(Join-Path $outputRoot 'review-manifest.json') status=$($review.review_status)"
