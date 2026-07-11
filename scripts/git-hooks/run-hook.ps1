param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('pre-commit', 'commit-msg', 'pre-push')]
    [string]$HookName,

    [Parameter(Position = 1)]
    [string]$CommitMessagePath,

    [string]$StagedFilesPath,

    [switch]$SkipTrackingCheck,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$GitHookArguments
)

$ErrorActionPreference = 'Stop'

function Write-Hook
{
    param([string]$Message)
    Write-Host "[git-hook:$HookName] $Message"
}

function Fail
{
    param([string]$Message)
    Write-Error "[git-hook:$HookName] $Message"
    exit 1
}

function Invoke-RepoCommand
{
    param([scriptblock]$Command)
    & $Command
    if ($LASTEXITCODE -ne 0)
    {
        exit $LASTEXITCODE
    }
}

function Get-RepoRoot
{
    $root = (& git rev-parse --show-toplevel 2>$null)
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($root))
    {
        Fail 'Could not locate repository root.'
    }

    return ([string]$root).Trim()
}

function Invoke-TrackingCheck
{
    if ($SkipTrackingCheck)
    {
        Write-Hook 'tracking check skipped by explicit hook argument'
        return
    }

    $trackingScript = Join-Path $repoRoot 'scripts\check-tracking.ps1'
    if (-not (Test-Path -LiteralPath $trackingScript))
    {
        Fail "Missing tracking script: $trackingScript"
    }

    & powershell -NoProfile -ExecutionPolicy Bypass -File $trackingScript
    if ($LASTEXITCODE -ne 0)
    {
        exit $LASTEXITCODE
    }
}

function Get-StagedFiles
{
    if (-not [string]::IsNullOrWhiteSpace($StagedFilesPath))
    {
        if (-not (Test-Path -LiteralPath $StagedFilesPath))
        {
            Fail "Staged file list does not exist: $StagedFilesPath"
        }

        return @(Get-Content -LiteralPath $StagedFilesPath | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    }

    return @(& git diff --cached --name-only --diff-filter=ACMR)
}

function Test-PreCommit
{
    Invoke-RepoCommand { git diff --cached --check }

    Invoke-TrackingCheck

    $blockedPatterns = @(
        '^build/',
        '^dist/',
        '^\.vs/',
        '^CMakeFiles/'
    )

    foreach ($file in Get-StagedFiles)
    {
        $normalized = ([string]$file).Replace('\', '/')
        foreach ($pattern in $blockedPatterns)
        {
            if ($normalized -match $pattern)
            {
                Fail "Refusing to commit generated or local-output path: $normalized"
            }
        }
    }

    Write-Hook 'pre-commit checks passed'
}

function Test-CommitMessage
{
    if ([string]::IsNullOrWhiteSpace($CommitMessagePath))
    {
        Fail 'Missing commit message path.'
    }
    if (-not (Test-Path -LiteralPath $CommitMessagePath))
    {
        Fail "Commit message file does not exist: $CommitMessagePath"
    }

    $subject = Get-Content -LiteralPath $CommitMessagePath |
        Where-Object { $_ -notmatch '^\s*#' -and -not [string]::IsNullOrWhiteSpace($_) } |
        Select-Object -First 1

    if ([string]::IsNullOrWhiteSpace($subject))
    {
        Fail 'Commit message subject is empty.'
    }

    $allowedPrefix = '^(docs|chore|build|test|release|ci|refactor)(\([^)]+\))?(:|\s)\s*\S+'
    $allowedSpecial = '^(Merge|Revert|fixup!|squash!)\b'
    if ($subject -match 'PSIM-\d{4}' -or $subject -match $allowedPrefix -or $subject -match $allowedSpecial)
    {
        Write-Hook 'commit message check passed'
        return
    }

    Fail "Commit message must include PSIM-#### or start with an approved workflow prefix: docs, chore, build, test, release, ci, or refactor. Subject: $subject"
}

function Test-PrePush
{
    Invoke-TrackingCheck

    $skipByEnv = $env:PSIM_SKIP_PRE_PUSH_TESTS -eq '1'
    $skipByConfig = ((& git config --bool hooks.physicsSim.skipPrePushTests 2>$null) -eq 'true')
    if ($skipByEnv -or $skipByConfig)
    {
        Write-Hook 'pre-push tests skipped by env/config'
        return
    }

    $testScript = Join-Path $repoRoot 'scripts\test.ps1'
    if (-not (Test-Path -LiteralPath $testScript))
    {
        Fail "Missing test script: $testScript"
    }

    & powershell -NoProfile -ExecutionPolicy Bypass -File $testScript
    if ($LASTEXITCODE -ne 0)
    {
        exit $LASTEXITCODE
    }

    Write-Hook 'pre-push checks passed'
}

$repoRoot = Get-RepoRoot
Push-Location $repoRoot
try
{
    switch ($HookName)
    {
        'pre-commit' { Test-PreCommit }
        'commit-msg' { Test-CommitMessage }
        'pre-push' { Test-PrePush }
    }
}
finally
{
    Pop-Location
}
