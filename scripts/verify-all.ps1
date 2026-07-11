$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'
$log = Join-Path $buildDir 'verification-bundle.log'

function Write-BundleLog
{
    param([string]$Message)

    Add-Content -LiteralPath $log -Value $Message
}

function Invoke-BundleStep
{
    param(
        [string]$Name,
        [string]$ScriptPath,
        [hashtable]$Arguments = @{}
    )

    Write-Host "[verification-bundle] $Name"
    Write-BundleLog ("[verification-bundle] {0}: starting" -f $Name)

    try
    {
        $global:LASTEXITCODE = 0
        & $ScriptPath @Arguments
        if ($null -ne $LASTEXITCODE -and $LASTEXITCODE -ne 0)
        {
            throw "step exited with code $LASTEXITCODE"
        }
        Write-BundleLog ("[verification-bundle] {0}: succeeded" -f $Name)
    }
    catch
    {
        $message = $_.Exception.Message
        Write-BundleLog ("[verification-bundle] {0}: failed: {1}" -f $Name, $message)
        throw ("[verification-bundle] {0} failed: {1}" -f $Name, $message)
    }
}

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
Set-Content -LiteralPath $log -Value "[verification-bundle] starting"

$steps = @(
    @{ Name = 'tracking validation'; Script = (Join-Path $PSScriptRoot 'check-tracking.ps1') },
    @{ Name = 'repository hygiene'; Script = (Join-Path $PSScriptRoot 'check-hygiene.ps1') },
    @{ Name = 'build'; Script = (Join-Path $PSScriptRoot 'build.ps1') },
    @{ Name = 'tests'; Script = (Join-Path $PSScriptRoot 'test.ps1'); Arguments = @{ Tier = 'Full' } },
    @{ Name = 'smoke test'; Script = (Join-Path $PSScriptRoot 'run-smoke.ps1') },
    @{ Name = 'replay regression suite'; Script = (Join-Path $PSScriptRoot 'verify-replay-suite.ps1') },
    @{ Name = 'fluid-quality regression suite'; Script = (Join-Path $PSScriptRoot 'verify-fluid-quality-suite.ps1') },
    @{ Name = 'solver benchmark'; Script = (Join-Path $PSScriptRoot 'measure-water-solver.ps1'); Arguments = @{ Profile = 'All' } },
    @{ Name = 'release package'; Script = (Join-Path $PSScriptRoot 'package-release.ps1'); Arguments = @{ SkipBuild = $true } }
)

foreach ($step in $steps)
{
    $arguments = if ($step.Arguments) { $step.Arguments } else { @{} }
    Invoke-BundleStep -Name $step.Name -ScriptPath $step.Script -Arguments $arguments
}

Write-BundleLog "[verification-bundle] success"
Write-Host '[verification-bundle] success'
