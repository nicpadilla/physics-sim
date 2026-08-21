param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath,

    [string]$OutputPath = 'usability-package-evidence.json'
)

$ErrorActionPreference = 'Stop'

function Get-DirectoryDigest
{
    param([Parameter(Mandatory = $true)][string]$Root)

    $rootFull = [System.IO.Path]::GetFullPath($Root).TrimEnd('\', '/')
    $lines = Get-ChildItem -LiteralPath $rootFull -Recurse -File |
        Sort-Object FullName |
        ForEach-Object {
            $relative = $_.FullName.Substring($rootFull.Length + 1).Replace('\', '/')
            $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
            "$hash  $relative"
        }

    $temp = [System.IO.Path]::GetTempFileName()
    try
    {
        Set-Content -LiteralPath $temp -Value $lines -Encoding ascii
        return (Get-FileHash -Algorithm SHA256 -LiteralPath $temp).Hash.ToLowerInvariant()
    }
    finally
    {
        Remove-Item -LiteralPath $temp -Force -ErrorAction SilentlyContinue
    }
}

$resolved = Resolve-Path -LiteralPath $PackagePath -ErrorAction Stop
$item = Get-Item -LiteralPath $resolved
$tempRoot = $null
try
{
    if ($item.PSIsContainer)
    {
        $packageRoot = $item.FullName
        $packageType = 'directory'
        $packageHash = Get-DirectoryDigest -Root $packageRoot
    }
    elseif ($item.Extension -ieq '.zip')
    {
        $packageType = 'zip'
        $packageHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $item.FullName).Hash.ToLowerInvariant()
        $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-usability-" + [guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
        Expand-Archive -LiteralPath $item.FullName -DestinationPath $tempRoot -Force
        $packageRoot = $tempRoot
    }
    else
    {
        throw 'PackagePath must be a packaged directory or ZIP archive.'
    }

    $manifests = @(Get-ChildItem -LiteralPath $packageRoot -Recurse -File -Filter 'release-manifest.json')
    if ($manifests.Count -ne 1)
    {
        throw "Expected exactly one release-manifest.json, found $($manifests.Count)."
    }

    $manifest = Get-Content -LiteralPath $manifests[0].FullName -Raw | ConvertFrom-Json
    foreach ($required in @('name', 'version', 'platform', 'commit'))
    {
        if (-not $manifest.PSObject.Properties.Name.Contains($required) -or [string]::IsNullOrWhiteSpace([string]$manifest.$required))
        {
            throw "Release manifest is missing required field '$required'."
        }
    }

    $evidence = [ordered]@{
        schema = 'physics-sim-usability-package'
        schema_version = 1
        captured_utc = [DateTime]::UtcNow.ToString('o')
        package_name = $item.Name
        package_type = $packageType
        package_sha256 = $packageHash
        product_name = [string]$manifest.name
        version = [string]$manifest.version
        commit = [string]$manifest.commit
        platform = [string]$manifest.platform
        modes = @($manifest.modes)
        scene_format = $manifest.scene_format
        replay_format = $manifest.replay_format
    }

    $outputFull = [System.IO.Path]::GetFullPath($OutputPath)
    $parent = Split-Path -Parent $outputFull
    if ($parent) { New-Item -ItemType Directory -Path $parent -Force | Out-Null }
    $evidence | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $outputFull -Encoding utf8
    Write-Host "[usability-package] evidence=$outputFull sha256=$packageHash version=$($manifest.version) commit=$($manifest.commit)"
}
finally
{
    if ($tempRoot)
    {
        Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}
