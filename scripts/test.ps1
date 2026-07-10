param(
    [ValidateSet('Fast', 'Standard', 'Full')]
    [string]$Tier = 'Standard'
)

$ErrorActionPreference = 'Stop'
$bundledCMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'
$ctestCommand = Get-Command ctest -ErrorAction SilentlyContinue
$ctest = if ($ctestCommand) { $ctestCommand.Path } elseif (Test-Path $bundledCMake) { $bundledCMake } else { $null }
if (-not $ctest)
{
    throw 'Could not find ctest on PATH or in the Visual Studio Build Tools installation.'
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'
$arguments = @('--test-dir', $buildDir, '-C', 'Debug', '--output-on-failure', '--parallel', '4')
$budgetSeconds = 0
switch ($Tier)
{
    'Fast'
    {
        $arguments += @('-LE', 'solver|visual|benchmark|release')
        $budgetSeconds = 30
    }
    'Standard'
    {
        $arguments += @('-LE', 'visual|benchmark|release')
        $budgetSeconds = 90
    }
    'Full'
    {
        $budgetSeconds = 480
    }
}

Write-Host "[tests] tier=$Tier budget_seconds=$budgetSeconds"
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
& $ctest @arguments
$exitCode = $LASTEXITCODE
$stopwatch.Stop()
$duration = [math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
Write-Host "[tests] tier=$Tier duration_seconds=$duration"
if ($exitCode -ne 0)
{
    throw "CTest tier $Tier failed with exit code $exitCode."
}
if ($duration -gt $budgetSeconds)
{
    throw "CTest tier $Tier exceeded its $budgetSeconds second budget: $duration seconds."
}
