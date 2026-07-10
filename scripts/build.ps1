param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'

$bundledCMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmake = $null

if ($cmakeCommand) {
    $cmake = $cmakeCommand.Path
}

if (-not $cmake) {
    if (Test-Path $bundledCMake) {
        $cmake = $bundledCMake
    }
    else {
        throw "Could not find cmake on PATH or at $bundledCMake"
    }
}

& $cmake --preset windows-x64
$preset = if ($Configuration -eq 'Release') { 'windows-x64-release' } else { 'windows-x64-debug' }
& $cmake --build --preset $preset
