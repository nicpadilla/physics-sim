param(
    [switch]$Check,
    [switch]$Uninstall
)

$ErrorActionPreference = 'Stop'

function Fail
{
    param([string]$Message)
    throw "[git-hooks] $Message"
}

$repoRoot = (& git rev-parse --show-toplevel 2>$null)
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repoRoot))
{
    Fail 'Could not locate repository root.'
}
$repoRoot = ([string]$repoRoot).Trim()

$expectedHooksPath = 'scripts/git-hooks'
$hookDir = Join-Path $repoRoot 'scripts\git-hooks'
$requiredHooks = @(
    'pre-commit',
    'commit-msg',
    'pre-push',
    'run-hook.ps1'
)

foreach ($hook in $requiredHooks)
{
    $path = Join-Path $hookDir $hook
    if (-not (Test-Path -LiteralPath $path))
    {
        Fail "Missing hook file: $path"
    }
}

if ($Uninstall)
{
    & git config --unset core.hooksPath
    if ($LASTEXITCODE -ne 0)
    {
        Fail 'Failed to unset core.hooksPath.'
    }

    Write-Host '[git-hooks] core.hooksPath unset'
    exit 0
}

$configuredPathOutput = @(& git config --get core.hooksPath 2>$null)
if ($LASTEXITCODE -ne 0 -or $configuredPathOutput.Count -eq 0)
{
    $configuredPath = ''
}
else
{
    $configuredPath = ([string]$configuredPathOutput[0]).Trim().Replace('\', '/')
}

if ($Check)
{
    if ($configuredPath -ne $expectedHooksPath)
    {
        Fail "core.hooksPath is '$configuredPath', expected '$expectedHooksPath'. Run .\scripts\install-git-hooks.ps1."
    }

    Write-Host "[git-hooks] installed: $expectedHooksPath"
    exit 0
}

& git config core.hooksPath $expectedHooksPath
if ($LASTEXITCODE -ne 0)
{
    Fail 'Failed to set core.hooksPath.'
}

Write-Host "[git-hooks] installed: $expectedHooksPath"
