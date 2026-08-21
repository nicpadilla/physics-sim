$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-usability-test-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
try
{
    $packageRoot = Join-Path $tempRoot 'physics-sim-test-package'
    New-Item -ItemType Directory -Path $packageRoot -Force | Out-Null
    [ordered]@{
        name = 'Physics Sim'
        version = '9.9.9-test'
        platform = 'windows-x64'
        commit = '0123456789abcdef0123456789abcdef01234567'
        modes = @('sandbox', 'lab')
        scene_format = 2
        replay_format = 2
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $packageRoot 'release-manifest.json') -Encoding utf8
    Set-Content -LiteralPath (Join-Path $packageRoot 'payload.txt') -Value 'test payload' -Encoding utf8

    $zip = Join-Path $tempRoot 'physics-sim-test-package.zip'
    Compress-Archive -LiteralPath $packageRoot -DestinationPath $zip
    $evidencePath = Join-Path $tempRoot 'evidence.json'
    & (Join-Path $PSScriptRoot 'capture-usability-package.ps1') -PackagePath $zip -OutputPath $evidencePath

    $evidence = Get-Content -LiteralPath $evidencePath -Raw | ConvertFrom-Json
    if ($evidence.schema -ne 'physics-sim-usability-package' -or $evidence.schema_version -ne 1)
    {
        throw 'Evidence schema is incorrect.'
    }
    if ($evidence.version -ne '9.9.9-test' -or $evidence.commit -ne '0123456789abcdef0123456789abcdef01234567')
    {
        throw 'Evidence lost release identity.'
    }
    $expectedHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zip).Hash.ToLowerInvariant()
    if ($evidence.package_sha256 -ne $expectedHash -or $evidence.package_type -ne 'zip')
    {
        throw 'Evidence contains the wrong ZIP hash or package type.'
    }

    $directoryEvidencePath = Join-Path $tempRoot 'directory-evidence.json'
    & (Join-Path $PSScriptRoot 'capture-usability-package.ps1') -PackagePath $packageRoot -OutputPath $directoryEvidencePath
    $directoryEvidence = Get-Content -LiteralPath $directoryEvidencePath -Raw | ConvertFrom-Json
    if ($directoryEvidence.package_type -ne 'directory' -or [string]::IsNullOrWhiteSpace($directoryEvidence.package_sha256))
    {
        throw 'Directory evidence is incomplete.'
    }

    $invalidRoot = Join-Path $tempRoot 'invalid-package'
    New-Item -ItemType Directory -Path $invalidRoot -Force | Out-Null
    $rejected = $false
    try
    {
        & (Join-Path $PSScriptRoot 'capture-usability-package.ps1') -PackagePath $invalidRoot -OutputPath (Join-Path $tempRoot 'invalid.json')
    }
    catch
    {
        $rejected = $true
    }
    if (-not $rejected)
    {
        throw 'A package without release-manifest.json was accepted.'
    }

    Write-Host '[usability-package-test] ZIP, directory, identity, and invalid-package checks passed'
}
finally
{
    Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
