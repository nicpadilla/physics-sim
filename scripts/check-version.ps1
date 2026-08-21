param(
    [string]$ExpectedTag = '',
    [string]$VersionFile = '',
    [string]$GalleryManifest = '',
    [string]$ReplayManifest = '',
    [switch]$SkipRepositoryMetadata
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$repoRootFull = [System.IO.Path]::GetFullPath($repoRoot).TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar

function Fail
{
    param([string]$Message)
    throw "[version] $Message"
}

function Resolve-RepositoryPath
{
    param([string]$RelativePath, [string]$Description)

    if ([string]::IsNullOrWhiteSpace($RelativePath) -or [System.IO.Path]::IsPathRooted($RelativePath))
    {
        Fail "$Description must be a non-empty repository-relative path: $RelativePath"
    }

    $segments = @($RelativePath -split '[\\/]')
    if ($segments -contains '..')
    {
        Fail "$Description escapes the repository: $RelativePath"
    }

    $normalized = $RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
    $fullPath = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $normalized))
    if (-not $fullPath.StartsWith($repoRootFull, [System.StringComparison]::OrdinalIgnoreCase))
    {
        Fail "$Description escapes the repository: $RelativePath"
    }

    return $fullPath
}

if ([string]::IsNullOrWhiteSpace($VersionFile)) { $VersionFile = Join-Path $repoRoot 'VERSION.txt' }
if ([string]::IsNullOrWhiteSpace($GalleryManifest)) { $GalleryManifest = Join-Path $repoRoot 'gallery\gallery.manifest' }
if ([string]::IsNullOrWhiteSpace($ReplayManifest)) { $ReplayManifest = Join-Path $repoRoot 'regression\replays\package.manifest' }

foreach ($required in @($VersionFile, $GalleryManifest, $ReplayManifest))
{
    if (-not (Test-Path -LiteralPath $required -PathType Leaf))
    {
        Fail "required file is missing: $required"
    }
}

$version = (Get-Content -LiteralPath $VersionFile -Raw).Trim()
$versionPattern = '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(-[0-9A-Za-z]+(?:[0-9A-Za-z.-]*[0-9A-Za-z])?)?$'
$versionMatch = [regex]::Match($version, $versionPattern)
if (-not $versionMatch.Success)
{
    Fail "VERSION.txt is not a supported semantic version: $version"
}
$versionCore = "$($versionMatch.Groups[1].Value).$($versionMatch.Groups[2].Value).$($versionMatch.Groups[3].Value)"

if (-not [string]::IsNullOrWhiteSpace($ExpectedTag) -and $ExpectedTag -ne "v$version")
{
    Fail "tag '$ExpectedTag' does not match authoritative version 'v$version'"
}

if (-not $SkipRepositoryMetadata)
{
    $vcpkgPath = Join-Path $repoRoot 'vcpkg.json'
    $vcpkg = Get-Content -LiteralPath $vcpkgPath -Raw | ConvertFrom-Json
    if ($vcpkg.'version-string' -ne $version)
    {
        Fail "vcpkg version '$($vcpkg.'version-string')' differs from VERSION.txt '$version'"
    }

    $cmakeText = Get-Content -LiteralPath (Join-Path $repoRoot 'CMakeLists.txt') -Raw
    if ($cmakeText -notmatch 'VERSION\.txt' -or $cmakeText -notmatch 'PHYSICS_SIM_VERSION_FULL' -or $cmakeText -notmatch 'PHYSICS_SIM_VERSION_CORE')
    {
        Fail 'CMakeLists.txt does not derive project metadata from VERSION.txt.'
    }

    $versionTemplate = Join-Path $repoRoot 'cmake\physics_sim_version.hpp.in'
    if (-not (Test-Path -LiteralPath $versionTemplate -PathType Leaf))
    {
        Fail 'generated runtime version header template is missing.'
    }

    $readmeText = Get-Content -LiteralPath (Join-Path $repoRoot 'README.md') -Raw
    $expectedReadmeVersion = 'Version `' + $version + '`'
    if (-not $readmeText.Contains($expectedReadmeVersion))
    {
        Fail "README.md does not identify version $version."
    }
    $expectedReleaseNotesLink = "docs/release-notes-$version.md"
    if (-not $readmeText.Contains($expectedReleaseNotesLink))
    {
        Fail "README.md does not link release notes for $version."
    }

    $releaseNotes = Join-Path $repoRoot "docs\release-notes-$version.md"
    if (-not (Test-Path -LiteralPath $releaseNotes -PathType Leaf))
    {
        Fail "release notes are missing: $releaseNotes"
    }
    $releaseHeading = (Get-Content -LiteralPath $releaseNotes -TotalCount 1).Trim()
    if ($releaseHeading -ne "# Physics Sim $version")
    {
        Fail "release-note heading '$releaseHeading' differs from version $version."
    }
}

$galleryLines = @(Get-Content -LiteralPath $GalleryManifest | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
if ($galleryLines.Count -lt 2 -or $galleryLines[0] -ne "physics-sim-gallery`t1")
{
    Fail 'gallery manifest has an unsupported or missing header.'
}

$galleryIds = New-Object System.Collections.Generic.List[string]
$galleryScenes = New-Object System.Collections.Generic.List[string]
$galleryThumbnails = New-Object System.Collections.Generic.List[string]
$challengeIds = New-Object System.Collections.Generic.List[string]
foreach ($line in $galleryLines | Select-Object -Skip 1)
{
    $fields = @($line -split "`t")
    if ($fields.Count -lt 9 -or $fields[0] -ne 'entry')
    {
        Fail "malformed gallery entry: $line"
    }

    $id = $fields[1]
    $category = $fields[2]
    $scenePath = $fields[4]
    $thumbnailPath = $fields[5]
    if ([string]::IsNullOrWhiteSpace($id) -or $galleryIds.Contains($id))
    {
        Fail "duplicate or empty gallery id: $id"
    }
    if ($galleryScenes.Contains($scenePath)) { Fail "duplicate gallery scene: $scenePath" }
    if ($galleryThumbnails.Contains($thumbnailPath)) { Fail "duplicate gallery thumbnail: $thumbnailPath" }

    $sceneFull = Resolve-RepositoryPath $scenePath 'gallery scene'
    $thumbnailFull = Resolve-RepositoryPath $thumbnailPath 'gallery thumbnail'
    if (-not (Test-Path -LiteralPath $sceneFull -PathType Leaf)) { Fail "gallery scene is missing: $scenePath" }
    if (-not (Test-Path -LiteralPath $thumbnailFull -PathType Leaf)) { Fail "gallery thumbnail is missing: $thumbnailPath" }

    $galleryIds.Add($id)
    $galleryScenes.Add($scenePath)
    $galleryThumbnails.Add($thumbnailPath)
    if ($category -eq 'challenges') { $challengeIds.Add($id) }
}

$trackedGalleryThumbnails = @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'gallery\thumbnails') -Filter '*.bmp' -File |
    ForEach-Object { "gallery/thumbnails/$($_.Name)" } | Sort-Object)
$unreferencedThumbnails = @($trackedGalleryThumbnails | Where-Object { -not $galleryThumbnails.Contains($_) })
if ($unreferencedThumbnails.Count -gt 0)
{
    Fail "unreferenced curated thumbnails: $($unreferencedThumbnails -join ', ')"
}

$replayLines = @(Get-Content -LiteralPath $ReplayManifest | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
if ($replayLines.Count -lt 2 -or $replayLines[0] -ne "physics-sim-replay-package`t1")
{
    Fail 'replay package manifest has an unsupported or missing header.'
}

$replayChallengeIds = New-Object System.Collections.Generic.List[string]
$replayFiles = New-Object System.Collections.Generic.List[string]
foreach ($line in $replayLines | Select-Object -Skip 1)
{
    $fields = @($line -split "`t")
    if ($fields.Count -ne 3 -or $fields[0] -ne 'replay')
    {
        Fail "malformed replay package entry: $line"
    }

    $challengeId = $fields[1]
    $replayName = $fields[2]
    if ($replayChallengeIds.Contains($challengeId)) { Fail "duplicate packaged challenge id: $challengeId" }
    if ($replayFiles.Contains($replayName)) { Fail "duplicate packaged replay: $replayName" }
    if (-not $challengeIds.Contains($challengeId)) { Fail "packaged replay references unknown challenge: $challengeId" }
    if ($replayName -ne [System.IO.Path]::GetFileName($replayName)) { Fail "replay entry must be a file name: $replayName" }

    $replayRelative = "regression/replays/$replayName"
    $replayFull = Resolve-RepositoryPath $replayRelative 'packaged replay'
    if (-not (Test-Path -LiteralPath $replayFull -PathType Leaf)) { Fail "packaged replay is missing: $replayName" }

    $replayChallengeIds.Add($challengeId)
    $replayFiles.Add($replayName)
}

$missingChallengeReplays = @($challengeIds | Where-Object { -not $replayChallengeIds.Contains($_) })
if ($missingChallengeReplays.Count -gt 0)
{
    Fail "challenge gallery entries have no packaged replay: $($missingChallengeReplays -join ', ')"
}

$trackedChallengeReplays = @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'regression\replays') -Filter 'challenge_*.replay' -File |
    ForEach-Object { $_.Name } | Sort-Object)
$unreferencedChallengeReplays = @($trackedChallengeReplays | Where-Object { -not $replayFiles.Contains($_) })
if ($unreferencedChallengeReplays.Count -gt 0)
{
    Fail "unreferenced challenge replays: $($unreferencedChallengeReplays -join ', ')"
}

Write-Host "[version] OK: version=$version core=$versionCore gallery_entries=$($galleryIds.Count) challenges=$($challengeIds.Count) replays=$($replayFiles.Count)"
