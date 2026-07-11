param([switch]$SkipBuild)

$ErrorActionPreference = 'Stop'
Import-Module Microsoft.PowerShell.Utility -ErrorAction Stop
$version = '0.2.0-alpha.1'
$repoRoot = Split-Path -Parent $PSScriptRoot
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
New-Item -ItemType Directory -Path $packageRoot -Force | Out-Null

$exe = Join-Path $buildDir 'physics-sim.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw "Missing Release output: $exe" }
Copy-Item -LiteralPath $exe -Destination $packageRoot
Get-ChildItem -LiteralPath $buildDir -Filter '*.dll' -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $packageRoot
}

$packageScenes = Join-Path $packageRoot 'scenes'
New-Item -ItemType Directory -Path $packageScenes -Force | Out-Null
foreach ($scene in @('starter_basin.pscene', 'tutorial_intro.pscene'))
{
    Copy-Item -LiteralPath (Join-Path $repoRoot "scenes\$scene") -Destination $packageScenes
}
Copy-Item -LiteralPath (Join-Path $repoRoot 'README.md') -Destination $packageRoot

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
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $packageRoot 'release-manifest.json') -Encoding utf8

$evidenceDirectory = Join-Path $packageRoot 'verification'
New-Item -ItemType Directory -Path $evidenceDirectory -Force | Out-Null
$verificationLog = Join-Path $buildRoot 'verification-bundle.log'
if (Test-Path -LiteralPath $verificationLog)
{
    Copy-Item -LiteralPath $verificationLog -Destination $evidenceDirectory
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
Write-Host "[release-package] package=$packageRoot"
Write-Host "[release-package] archive=$zipPath sha256=$zipHash"
