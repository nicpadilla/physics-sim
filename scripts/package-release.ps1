param([switch]$SkipBuild)

$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
$repoRoot = Split-Path -Parent $PSScriptRoot
$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Raw).Trim()
$expectedTag = ''
if ($env:GITHUB_REF -like 'refs/tags/*')
{
    $expectedTag = $env:GITHUB_REF.Substring('refs/tags/'.Length)
}
elseif ($env:GITHUB_REF_TYPE -eq 'tag' -and -not [string]::IsNullOrWhiteSpace($env:GITHUB_REF_NAME))
{
    $expectedTag = $env:GITHUB_REF_NAME
}

$versionCheckArguments = @{}
if (-not [string]::IsNullOrWhiteSpace($expectedTag)) { $versionCheckArguments.ExpectedTag = $expectedTag }
& (Join-Path $PSScriptRoot 'check-version.ps1') @versionCheckArguments

$buildRoot = Join-Path $repoRoot 'build\windows-x64'
$buildDir = Join-Path $buildRoot 'Release'
$distRoot = Join-Path $repoRoot 'dist'
$packageName = "physics-sim-$version-windows-x64"
$packageRoot = Join-Path $distRoot $packageName
$zipPath = Join-Path $distRoot "$packageName.zip"

$repoRootFull = [System.IO.Path]::GetFullPath($repoRoot)
$packageRootFull = [System.IO.Path]::GetFullPath($packageRoot)
if (-not $packageRootFull.StartsWith($repoRootFull, [System.StringComparison]::OrdinalIgnoreCase))
{
    throw "Refusing to recreate a package outside the repository root: $packageRootFull"
}
if (-not $SkipBuild)
{
    & (Join-Path $PSScriptRoot 'build.ps1') -Configuration Release
}
if (Test-Path -LiteralPath $packageRoot) { Remove-Item -LiteralPath $packageRoot -Recurse -Force }
if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath -Force }
if (Test-Path -LiteralPath "$zipPath.sha256") { Remove-Item -LiteralPath "$zipPath.sha256" -Force }
New-Item -ItemType Directory -Path $packageRoot -Force | Out-Null

$exe = Join-Path $buildDir 'physics-sim.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw "Missing Release output: $exe" }
Copy-Item -LiteralPath $exe -Destination $packageRoot
Get-ChildItem -LiteralPath $buildDir -Filter '*.dll' -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $packageRoot
}

$galleryManifest = Join-Path $repoRoot 'gallery\gallery.manifest'
$galleryLines = @(Get-Content -LiteralPath $galleryManifest | Where-Object { $_ -like "entry`t*" })
if ($galleryLines.Count -eq 0) { throw 'Gallery manifest contains no package entries.' }
$scenePaths = @()
$thumbnailPaths = @()
foreach ($line in $galleryLines)
{
    $fields = @($line -split "`t")
    if ($fields.Count -lt 9) { throw "Malformed gallery manifest entry: $line" }
    $scenePaths += $fields[4]
    $thumbnailPaths += $fields[5]
}
$scenePaths = @($scenePaths | Sort-Object -Unique)
$thumbnailPaths = @($thumbnailPaths | Sort-Object -Unique)

$packageScenes = Join-Path $packageRoot 'scenes'
New-Item -ItemType Directory -Path $packageScenes -Force | Out-Null
foreach ($scenePath in $scenePaths)
{
    $source = Join-Path $repoRoot $scenePath
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Gallery scene is missing: $scenePath" }
    Copy-Item -LiteralPath $source -Destination $packageScenes
}

$packageGallery = Join-Path $packageRoot 'gallery'
$packageThumbnails = Join-Path $packageGallery 'thumbnails'
New-Item -ItemType Directory -Path $packageThumbnails -Force | Out-Null
Copy-Item -LiteralPath $galleryManifest -Destination $packageGallery
foreach ($thumbnailPath in $thumbnailPaths)
{
    $source = Join-Path $repoRoot $thumbnailPath
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Gallery thumbnail is missing: $thumbnailPath" }
    Copy-Item -LiteralPath $source -Destination $packageThumbnails
}

$replayManifest = Join-Path $repoRoot 'regression\replays\package.manifest'
$replayLines = @(Get-Content -LiteralPath $replayManifest | Where-Object { $_ -like "replay`t*" })
if ($replayLines.Count -eq 0) { throw 'Replay package manifest contains no entries.' }
$packageReplays = Join-Path $packageRoot 'replays'
New-Item -ItemType Directory -Path $packageReplays -Force | Out-Null
foreach ($line in $replayLines)
{
    $fields = @($line -split "`t")
    if ($fields.Count -ne 3) { throw "Malformed replay package entry: $line" }
    $replay = $fields[2]
    $source = Join-Path $repoRoot "regression\replays\$replay"
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Packaged replay is missing: $replay" }
    Copy-Item -LiteralPath $source -Destination $packageReplays
}
Copy-Item -LiteralPath $replayManifest -Destination $packageReplays

Copy-Item -LiteralPath (Join-Path $repoRoot 'README.md') -Destination $packageRoot
Copy-Item -LiteralPath (Join-Path $repoRoot "docs\release-notes-$version.md") -Destination $packageRoot
Copy-Item -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Destination $packageRoot

$licenses = Join-Path $packageRoot 'licenses'
New-Item -ItemType Directory -Path $licenses -Force | Out-Null
foreach ($dependency in @('imgui', 'sdl2'))
{
    $copyright = Join-Path $buildRoot "vcpkg_installed\x64-windows\share\$dependency\copyright"
    if (-not (Test-Path -LiteralPath $copyright)) { throw "Missing dependency license: $copyright" }
    Copy-Item -LiteralPath $copyright -Destination (Join-Path $licenses "$dependency.txt")
}

$commit = (& git -C $repoRoot rev-parse HEAD).Trim()
$manifest = [ordered]@{
    name = 'Physics Sim'
    version = $version
    platform = 'windows-x64'
    commit = $commit
    modes = @('sandbox', 'lab')
    scene_format = 2
    replay_format = 2
    gallery_manifest = 'gallery/gallery.manifest'
    replay_package_manifest = 'replays/package.manifest'
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $packageRoot 'release-manifest.json') -Encoding utf8

$evidenceDirectory = Join-Path $packageRoot 'verification'
New-Item -ItemType Directory -Path $evidenceDirectory -Force | Out-Null
$verificationLog = Join-Path $buildRoot 'verification-bundle.log'
if (Test-Path -LiteralPath $verificationLog)
{
    Copy-Item -LiteralPath $verificationLog -Destination $evidenceDirectory
}
foreach ($evidence in @(
    'audio-review\summary.json',
    'challenge-review\summary.json',
    'fluid-quality-suite\summary.json'
))
{
    $source = Join-Path $buildRoot $evidence
    if (Test-Path -LiteralPath $source)
    {
        $destination = Join-Path $evidenceDirectory ($evidence.Replace('\', '-'))
        Copy-Item -LiteralPath $source -Destination $destination
    }
}

$sandboxLog = Join-Path $evidenceDirectory 'packaged-sandbox.log'
$sandboxSettings = Join-Path $evidenceDirectory 'packaged-settings.txt'
$sandboxArgs = @('--mode', 'sandbox', '--skip-session-shell', '--scene-path', 'scenes\starter_basin.pscene',
    '--log-file', $sandboxLog, '--settings-file', $sandboxSettings, '--auto-exit-ms', '1200')
$sandbox = Start-Process -FilePath (Join-Path $packageRoot 'physics-sim.exe') -ArgumentList $sandboxArgs -WorkingDirectory $packageRoot -PassThru -WindowStyle Hidden
if (-not $sandbox.WaitForExit(15000)) { Stop-Process -Id $sandbox.Id -Force; throw 'Packaged sandbox smoke timed out.' }
if ($sandbox.ExitCode -ne 0) { throw "Packaged sandbox smoke failed with exit $($sandbox.ExitCode)." }

$labBundle = Join-Path $evidenceDirectory 'lab-capture'
$labArgs = @('--mode', 'lab', '--capture-bundle', $labBundle, '--auto-exit-ms', '5000')
$lab = Start-Process -FilePath (Join-Path $packageRoot 'physics-sim.exe') -ArgumentList $labArgs -WorkingDirectory $packageRoot -PassThru -WindowStyle Hidden
if (-not $lab.WaitForExit(15000)) { Stop-Process -Id $lab.Id -Force; throw 'Packaged lab smoke timed out.' }
if ($lab.ExitCode -ne 0 -or -not (Test-Path -LiteralPath (Join-Path $labBundle 'metrics.json'))) { throw 'Packaged lab smoke failed.' }

$checksumPath = Join-Path $packageRoot 'SHA256SUMS.txt'
$checksumLines = Get-ChildItem -LiteralPath $packageRoot -Recurse -File |
    Where-Object { $_.FullName -ne $checksumPath } |
    Sort-Object FullName |
    ForEach-Object {
        $relative = $_.FullName.Substring($packageRootFull.Length + 1).Replace('\', '/')
        $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
        "$hash  $relative"
    }
Set-Content -LiteralPath $checksumPath -Value $checksumLines -Encoding ascii
Compress-Archive -LiteralPath $packageRoot -DestinationPath $zipPath -CompressionLevel Optimal
$zipHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash.ToLowerInvariant()
Set-Content -LiteralPath "$zipPath.sha256" -Value "$zipHash  $packageName.zip" -Encoding ascii
Write-Host "[release-package] version=$version package=$packageRoot"
Write-Host "[release-package] archive=$zipPath sha256=$zipHash"
