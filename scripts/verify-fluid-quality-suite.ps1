$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'
$suiteDir = Join-Path $buildDir 'fluid-quality-suite'
$suiteLog = Join-Path $buildDir 'fluid-quality-suite.log'
$manifestPath = Join-Path $repoRoot 'regression\fluid_quality_suite.psd1'
$exe = Join-Path $buildDir 'Debug\physics_sim_fluid_quality_tests.exe'

function Write-SuiteLog
{
    param([string]$Message)
    Add-Content -LiteralPath $suiteLog -Value $Message
}

function Write-CaseLog
{
    param(
        [string]$Path,
        [string]$Message
    )

    Add-Content -LiteralPath $Path -Value $Message
}

New-Item -ItemType Directory -Path $suiteDir -Force | Out-Null
Set-Content -LiteralPath $suiteLog -Value "[fluid-quality-suite] starting"
Write-SuiteLog "[fluid-quality-suite] manifest: $manifestPath"

if (-not (Test-Path -LiteralPath $exe))
{
    Write-SuiteLog "[fluid-quality-suite] missing exe"
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

if (-not (Test-Path -LiteralPath $manifestPath))
{
    Write-SuiteLog "[fluid-quality-suite] missing manifest"
    throw "Missing fluid-quality manifest: $manifestPath"
}

$manifest = Import-PowerShellDataFile -LiteralPath $manifestPath
if (-not $manifest)
{
    Write-SuiteLog "[fluid-quality-suite] manifest had no content"
    throw "Fluid-quality manifest did not define any content: $manifestPath"
}

$cases = @($manifest.Cases)
if (-not $cases)
{
    Write-SuiteLog "[fluid-quality-suite] manifest had no cases"
    throw "Fluid-quality manifest did not define any cases: $manifestPath"
}

$availableScenarios = @(& $exe --list-scenarios)
$availableSet = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($scenario in $availableScenarios)
{
    $scenarioName = ([string]$scenario).Trim()
    if ([string]::IsNullOrWhiteSpace($scenarioName))
    {
        continue
    }

    [void]$availableSet.Add($scenarioName)
}

$manifestSet = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach ($case in $cases)
{
    [void]$manifestSet.Add([string]$case.Name)
    if (-not $availableSet.Contains([string]$case.Name))
    {
        Write-SuiteLog ("[fluid-quality-suite] missing scenario: {0}" -f $case.Name)
        throw "Fluid-quality executable does not expose scenario $($case.Name)."
    }
}

if ($availableSet.Count -ne $manifestSet.Count)
{
    Write-SuiteLog "[fluid-quality-suite] manifest and executable scenario counts differ"
    throw "Fluid-quality manifest does not cover every executable scenario."
}

foreach ($case in $cases)
{
    $caseName = [string]$case.Name
    $sampleTicks = @($case.SampleTicks)
    $caseLog = Join-Path $suiteDir ("{0}.log" -f $caseName)

    Set-Content -LiteralPath $caseLog -Value ("[fluid-quality-suite] case={0}" -f $caseName)
    Write-CaseLog $caseLog ("[fluid-quality-suite] sample ticks: {0}" -f (($sampleTicks | ForEach-Object { $_.ToString() }) -join ', '))
    Write-SuiteLog ("[fluid-quality-suite] case {0}: starting" -f $caseName)

    $output = & $exe '--scenario' $caseName 2>&1
    $exitCode = $LASTEXITCODE
    foreach ($line in $output)
    {
        Write-CaseLog $caseLog $line
    }

    if ($exitCode -ne 0)
    {
        Write-SuiteLog ("[fluid-quality-suite] case {0}: failed with exit code {1}" -f $caseName, $exitCode)
        throw "physics_sim_fluid_quality_tests.exe returned exit code $exitCode for case $caseName."
    }

    Write-CaseLog $caseLog "[fluid-quality-suite] succeeded"
    Write-SuiteLog ("[fluid-quality-suite] case {0}: succeeded" -f $caseName)
}

Write-SuiteLog "[fluid-quality-suite] success"
Write-Host '[fluid-quality-suite] success'
