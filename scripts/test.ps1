$ErrorActionPreference = 'Stop'

$bundledCMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'
$ctestCommand = Get-Command ctest -ErrorAction SilentlyContinue
$ctest = $null

if ($ctestCommand) {
    $ctest = $ctestCommand.Path
}

if (-not $ctest) {
    if (Test-Path $bundledCMake) {
        $ctest = $bundledCMake
    }
    else {
        throw "Could not find ctest on PATH or at $bundledCMake"
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'

& $ctest --test-dir $buildDir -C Debug --output-on-failure
