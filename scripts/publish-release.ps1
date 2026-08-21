param(
    [Parameter(Mandatory = $true)]
    [string]$ArtifactRoot,

    [string]$ExpectedTag = $env:GITHUB_REF_NAME,

    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

function Fail
{
    param([string]$Message)
    throw "[release-publish] $Message"
}

$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Raw).Trim()
if ([string]::IsNullOrWhiteSpace($ExpectedTag))
{
    Fail 'ExpectedTag is required.'
}

& (Join-Path $PSScriptRoot 'check-version.ps1') -ExpectedTag $ExpectedTag

$artifactRootFull = [System.IO.Path]::GetFullPath($ArtifactRoot)
if (-not (Test-Path -LiteralPath $artifactRootFull -PathType Container))
{
    Fail "artifact root is missing: $artifactRootFull"
}

$packageName = "physics-sim-$version-windows-x64"
$archives = @(Get-ChildItem -LiteralPath $artifactRootFull -Recurse -File -Filter "$packageName.zip")
$checksums = @(Get-ChildItem -LiteralPath $artifactRootFull -Recurse -File -Filter "$packageName.zip.sha256")
if ($archives.Count -ne 1)
{
    Fail "expected exactly one release archive, found $($archives.Count)"
}
if ($checksums.Count -ne 1)
{
    Fail "expected exactly one release checksum, found $($checksums.Count)"
}

$notesPath = Join-Path $repoRoot "docs\release-notes-$version.md"
if (-not (Test-Path -LiteralPath $notesPath -PathType Leaf))
{
    Fail "release notes are missing: $notesPath"
}

$expectedChecksumName = "$packageName.zip"
$checksumLine = (Get-Content -LiteralPath $checksums[0].FullName -Raw).Trim()
if ($checksumLine -notmatch '^[0-9a-fA-F]{64}\s{2}(.+)$' -or $Matches[1] -ne $expectedChecksumName)
{
    Fail "checksum sidecar does not identify $expectedChecksumName"
}

$archiveHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $archives[0].FullName).Hash.ToLowerInvariant()
$recordedHash = ($checksumLine -split '\s+', 2)[0].ToLowerInvariant()
if ($archiveHash -ne $recordedHash)
{
    Fail "archive hash $archiveHash differs from checksum $recordedHash"
}

$isPrerelease = $version.Contains('-')
$summary = [ordered]@{
    tag = $ExpectedTag
    version = $version
    prerelease = $isPrerelease
    archive = $archives[0].FullName
    checksum = $checksums[0].FullName
    sha256 = $archiveHash
    notes = $notesPath
}

if ($DryRun)
{
    $summary | ConvertTo-Json -Depth 3
    Write-Host "[release-publish] dry-run OK: tag=$ExpectedTag archive=$($archives[0].Name) sha256=$archiveHash"
    return
}

if ([string]::IsNullOrWhiteSpace($env:GH_TOKEN))
{
    Fail 'GH_TOKEN is required outside dry-run mode.'
}
if (-not (Get-Command gh -ErrorAction SilentlyContinue))
{
    Fail 'GitHub CLI is required outside dry-run mode.'
}

$repo = $env:GITHUB_REPOSITORY
if ([string]::IsNullOrWhiteSpace($repo))
{
    Fail 'GITHUB_REPOSITORY is required outside dry-run mode.'
}

& gh release view $ExpectedTag --repo $repo *> $null
$releaseExists = $LASTEXITCODE -eq 0
if ($releaseExists)
{
    $editArguments = @(
        'release', 'edit', $ExpectedTag,
        '--repo', $repo,
        '--title', "Physics Sim $version",
        '--notes-file', $notesPath
    )
    if ($isPrerelease) { $editArguments += '--prerelease' }
    & gh @editArguments
    if ($LASTEXITCODE -ne 0) { Fail 'failed to update release metadata' }

    & gh release upload $ExpectedTag $archives[0].FullName $checksums[0].FullName --repo $repo --clobber
    if ($LASTEXITCODE -ne 0) { Fail 'failed to upload release artifacts' }
}
else
{
    $createArguments = @(
        'release', 'create', $ExpectedTag,
        $archives[0].FullName,
        $checksums[0].FullName,
        '--repo', $repo,
        '--title', "Physics Sim $version",
        '--notes-file', $notesPath
    )
    if ($isPrerelease) { $createArguments += '--prerelease' }
    & gh @createArguments
    if ($LASTEXITCODE -ne 0) { Fail 'failed to create release' }
}

Write-Host "[release-publish] published tag=$ExpectedTag archive=$($archives[0].Name) sha256=$archiveHash"
