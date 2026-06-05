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
        [string]$ScriptPath
    )

    Write-Host "[verification-bundle] $Name"
    Write-BundleLog ("[verification-bundle] {0}: starting" -f $Name)

    try
    {
        & $ScriptPath
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
    @{ Name = 'build'; Script = (Join-Path $PSScriptRoot 'build.ps1') },
    @{ Name = 'tests'; Script = (Join-Path $PSScriptRoot 'test.ps1') },
    @{ Name = 'smoke test'; Script = (Join-Path $PSScriptRoot 'run-smoke.ps1') },
    @{ Name = 'replay regression suite'; Script = (Join-Path $PSScriptRoot 'verify-replay-suite.ps1') },
    @{ Name = 'fluid-quality regression suite'; Script = (Join-Path $PSScriptRoot 'verify-fluid-quality-suite.ps1') }
)

foreach ($step in $steps)
{
    Invoke-BundleStep -Name $step.Name -ScriptPath $step.Script
}

Write-BundleLog "[verification-bundle] success"
Write-Host '[verification-bundle] success'
