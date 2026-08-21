$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$version = (Get-Content -LiteralPath (Join-Path $repoRoot 'VERSION.txt') -Raw).Trim()
$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-release-publish-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
try
{
    $packageName = "physics-sim-$version-windows-x64"
    $archive = Join-Path $tempRoot "$packageName.zip"
    Set-Content -LiteralPath $archive -Value 'release publication test payload' -Encoding utf8
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $archive).Hash.ToLowerInvariant()
    Set-Content -LiteralPath "$archive.sha256" -Value "$hash  $packageName.zip" -Encoding ascii

    $output = & (Join-Path $PSScriptRoot 'publish-release.ps1') -ArtifactRoot $tempRoot -ExpectedTag "v$version" -DryRun | Out-String
    if ($output -notmatch 'dry-run OK' -or $output -notmatch [regex]::Escape($hash))
    {
        throw 'Valid release dry-run did not report the expected archive identity.'
    }

    $mismatchRejected = $false
    try
    {
        & (Join-Path $PSScriptRoot 'publish-release.ps1') -ArtifactRoot $tempRoot -ExpectedTag 'v0.0.0-mismatch' -DryRun *> $null
    }
    catch
    {
        $mismatchRejected = $true
    }
    if (-not $mismatchRejected)
    {
        throw 'Mismatched release tag was accepted.'
    }

    $badChecksum = Join-Path $tempRoot 'bad-checksum'
    New-Item -ItemType Directory -Path $badChecksum -Force | Out-Null
    Copy-Item -LiteralPath $archive -Destination $badChecksum
    Set-Content -LiteralPath (Join-Path $badChecksum "$packageName.zip.sha256") -Value (('0' * 64) + "  $packageName.zip") -Encoding ascii
    $checksumRejected = $false
    try
    {
        & (Join-Path $PSScriptRoot 'publish-release.ps1') -ArtifactRoot $badChecksum -ExpectedTag "v$version" -DryRun *> $null
    }
    catch
    {
        $checksumRejected = $true
    }
    if (-not $checksumRejected)
    {
        throw 'Incorrect release checksum was accepted.'
    }

    $duplicateRoot = Join-Path $tempRoot 'duplicates'
    New-Item -ItemType Directory -Path (Join-Path $duplicateRoot 'a') -Force | Out-Null
    New-Item -ItemType Directory -Path (Join-Path $duplicateRoot 'b') -Force | Out-Null
    Copy-Item -LiteralPath $archive -Destination (Join-Path $duplicateRoot 'a')
    Copy-Item -LiteralPath $archive -Destination (Join-Path $duplicateRoot 'b')
    Copy-Item -LiteralPath "$archive.sha256" -Destination (Join-Path $duplicateRoot 'a')
    Copy-Item -LiteralPath "$archive.sha256" -Destination (Join-Path $duplicateRoot 'b')
    $duplicateRejected = $false
    try
    {
        & (Join-Path $PSScriptRoot 'publish-release.ps1') -ArtifactRoot $duplicateRoot -ExpectedTag "v$version" -DryRun *> $null
    }
    catch
    {
        $duplicateRejected = $true
    }
    if (-not $duplicateRejected)
    {
        throw 'Duplicate release artifacts were accepted.'
    }

    Write-Host '[release-publication-test] dry-run, tag mismatch, checksum mismatch, and duplicate-artifact checks passed'
}
finally
{
    Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
