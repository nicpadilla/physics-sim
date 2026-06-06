$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'
$suiteDir = Join-Path $buildDir 'fluid-quality-suite'
$suiteLog = Join-Path $buildDir 'fluid-quality-suite.log'
$manifestPath = Join-Path $repoRoot 'regression\fluid_quality_suite.psd1'
$exe = Join-Path $buildDir 'Debug\physics_sim_fluid_quality_tests.exe'

$strictQualityScenarios = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
[void]$strictQualityScenarios.Add('still-pool')
[void]$strictQualityScenarios.Add('hydrostatic-column')
[void]$strictQualityScenarios.Add('particle-overcrowding')
[void]$strictQualityScenarios.Add('long-run-stress')

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

function Get-ManifestProfile
{
    param([psobject]$Case)

    if ([string]::IsNullOrWhiteSpace([string]$Case.Profile))
    {
        throw "Fluid-quality manifest case $($Case.Name) is missing a Profile entry."
    }

    $profile = ([string]$Case.Profile).Trim().ToLowerInvariant()
    switch ($profile)
    {
        'balanced' { return 'balanced' }
        'quality' { return 'quality' }
        default
        {
            throw "Fluid-quality manifest case $($Case.Name) uses unsupported profile '$profile'."
        }
    }
}

function Parse-ScenarioFields
{
    param([string]$Line)

    $fields = @{}
    foreach ($token in ($Line -split '\s+'))
    {
        if (-not $token.Contains('='))
        {
            continue
        }

        $parts = $token -split '=', 2
        if ($parts.Count -eq 2 -and -not [string]::IsNullOrWhiteSpace($parts[0]))
        {
            $fields[$parts[0]] = $parts[1]
        }
    }

    return $fields
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
${scenarioProfiles} = @{}
foreach ($case in $cases)
{
    $caseName = ([string]$case.Name).Trim()
    if ([string]::IsNullOrWhiteSpace($caseName))
    {
        Write-SuiteLog "[fluid-quality-suite] manifest case missing name"
        throw "Fluid-quality manifest contained a case without a name."
    }

    $profile = Get-ManifestProfile $case

    if (-not $manifestSet.Contains($caseName))
    {
        [void]$manifestSet.Add($caseName)
    }

    if (-not $availableSet.Contains($caseName))
    {
        Write-SuiteLog ("[fluid-quality-suite] missing scenario: {0}" -f $caseName)
        throw "Fluid-quality executable does not expose scenario $caseName."
    }

    if (-not $scenarioProfiles.ContainsKey($caseName))
    {
        $scenarioProfiles[$caseName] = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    }

    [void]$scenarioProfiles[$caseName].Add($profile)

    if ($profile -eq 'quality' -and -not $strictQualityScenarios.Contains($caseName))
    {
        Write-SuiteLog ("[fluid-quality-suite] unsupported quality-only scenario: {0}" -f $caseName)
        throw "Fluid-quality manifest reserved the quality profile for unsupported scenario $caseName."
    }
}

if ($availableSet.Count -ne $manifestSet.Count)
{
    Write-SuiteLog "[fluid-quality-suite] manifest and executable scenario counts differ"
    throw "Fluid-quality manifest does not cover every executable scenario."
}

foreach ($scenarioName in $availableSet)
{
    if (-not $scenarioProfiles.ContainsKey($scenarioName) -or -not $scenarioProfiles[$scenarioName].Contains('balanced'))
    {
        Write-SuiteLog ("[fluid-quality-suite] missing balanced profile for scenario: {0}" -f $scenarioName)
        throw "Fluid-quality manifest did not include a balanced case for $scenarioName."
    }
}

foreach ($case in $cases)
{
    $caseName = ([string]$case.Name).Trim()
    $profile = Get-ManifestProfile $case
    $sampleTicks = @($case.SampleTicks)
    $caseLog = Join-Path $suiteDir ("{0}-{1}.log" -f $caseName, $profile)

    Set-Content -LiteralPath $caseLog -Value ("[fluid-quality-suite] case={0} profile={1}" -f $caseName, $profile)
    Write-CaseLog $caseLog ("[fluid-quality-suite] sample ticks: {0}" -f (($sampleTicks | ForEach-Object { $_.ToString() }) -join ', '))
    Write-SuiteLog ("[fluid-quality-suite] case {0} ({1}): starting" -f $caseName, $profile)

    $output = & $exe '--scenario' $caseName '--profile' $profile 2>&1
    $exitCode = $LASTEXITCODE
    foreach ($line in $output)
    {
        Write-CaseLog $caseLog $line
    }

    if ($exitCode -ne 0)
    {
        Write-SuiteLog ("[fluid-quality-suite] case {0} ({1}): failed with exit code {2}" -f $caseName, $profile, $exitCode)
        throw "physics_sim_fluid_quality_tests.exe returned exit code $exitCode for case $caseName ($profile)."
    }

    Write-CaseLog $caseLog "[fluid-quality-suite] succeeded"
    Write-SuiteLog ("[fluid-quality-suite] case {0} ({1}): succeeded" -f $caseName, $profile)

    $finalLines = @($output | Where-Object { ([string]$_).TrimStart().StartsWith('scenario=') })
    if ($finalLines.Count -eq 0)
    {
        Write-SuiteLog ("[fluid-quality-suite] case {0} ({1}): missing final scenario line" -f $caseName, $profile)
        throw "Fluid-quality case $caseName ($profile) did not print the required final scenario line."
    }

    $fields = Parse-ScenarioFields ([string]$finalLines[-1])
    foreach ($requiredField in @('scenario', 'profile', 'tier', 'mass_error', 'density_error', 'kinetic_energy', 'pressure_residual', 'removed', 'outflow'))
    {
        if (-not $fields.ContainsKey($requiredField))
        {
            Write-SuiteLog ("[fluid-quality-suite] case {0} ({1}): missing field {2}" -f $caseName, $profile, $requiredField)
            throw "Fluid-quality case $caseName ($profile) did not print required field $requiredField."
        }
    }

    if ($fields['scenario'] -ne $caseName)
    {
        throw "Fluid-quality case $caseName ($profile) reported scenario $($fields['scenario']) in its final line."
    }

    if ($fields['profile'] -ne $profile)
    {
        throw "Fluid-quality case $caseName ($profile) reported profile $($fields['profile']) in its final line."
    }

    $expectedTier = if ($profile -eq 'quality') { 'offline' } else { 'live' }
    if ($fields['tier'] -ne $expectedTier)
    {
        throw "Fluid-quality case $caseName ($profile) reported tier $($fields['tier']) instead of $expectedTier."
    }

    foreach ($numericField in @('mass_error', 'density_error', 'kinetic_energy', 'pressure_residual'))
    {
        try
        {
            [void][double]::Parse($fields[$numericField], [System.Globalization.CultureInfo]::InvariantCulture)
        }
        catch
        {
            throw "Fluid-quality case $caseName ($profile) reported invalid numeric value for ${numericField}: $($fields[$numericField])."
        }
    }

    foreach ($integerField in @('removed', 'outflow'))
    {
        try
        {
            [void][UInt64]::Parse($fields[$integerField], [System.Globalization.CultureInfo]::InvariantCulture)
        }
        catch
        {
            throw "Fluid-quality case $caseName ($profile) reported invalid integer value for ${integerField}: $($fields[$integerField])."
        }
    }
}

Write-SuiteLog "[fluid-quality-suite] success"
Write-Host '[fluid-quality-suite] success'
