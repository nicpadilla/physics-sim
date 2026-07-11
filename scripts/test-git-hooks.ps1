$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$hookDir = Join-Path $repoRoot 'scripts\git-hooks'
$runner = Join-Path $hookDir 'run-hook.ps1'

function Fail
{
    param([string]$Message)
    throw "[git-hooks-test] $Message"
}

function Invoke-Hook
{
    param(
        [string[]]$Arguments,
        [int]$ExpectedExitCode = 0
    )

    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try
    {
        $output = & powershell -NoProfile -ExecutionPolicy Bypass -File $runner @Arguments 2>&1
        $exitCode = $LASTEXITCODE
    }
    finally
    {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($exitCode -ne $ExpectedExitCode)
    {
        foreach ($line in $output)
        {
            Write-Host $line
        }
        Fail "Hook runner exit code $exitCode did not match expected $ExpectedExitCode for args: $($Arguments -join ' ')"
    }
}

foreach ($relativePath in @(
    'scripts\git-hooks\pre-commit',
    'scripts\git-hooks\commit-msg',
    'scripts\git-hooks\pre-push',
    'scripts\git-hooks\run-hook.ps1',
    'scripts\install-git-hooks.ps1'
))
{
    $fullPath = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $fullPath))
    {
        Fail "Missing hook workflow file: $relativePath"
    }
}

$tempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-hooks-{0}" -f ([System.Guid]::NewGuid().ToString('N')))
New-Item -ItemType Directory -Path $tempDir | Out-Null

try
{
    $validMessage = Join-Path $tempDir 'valid-message.txt'
    $invalidMessage = Join-Path $tempDir 'invalid-message.txt'
    $allowedStaged = Join-Path $tempDir 'allowed-staged.txt'
    $generatedStaged = Join-Path $tempDir 'generated-staged.txt'

    Set-Content -LiteralPath $validMessage -Value 'docs add git workflow hooks'
    Set-Content -LiteralPath $invalidMessage -Value 'misc changes'
    Set-Content -LiteralPath $allowedStaged -Value @(
        'AGENTS.md',
        'docs/TRACKING.md',
        'scripts/git-hooks/pre-commit'
    )
    Set-Content -LiteralPath $generatedStaged -Value @(
        'AGENTS.md',
        'build/windows-x64/generated.obj'
    )

    Invoke-Hook -Arguments @('commit-msg', $validMessage)
    Invoke-Hook -Arguments @('commit-msg', $invalidMessage) -ExpectedExitCode 1
    Invoke-Hook -Arguments @('pre-commit', '-StagedFilesPath', $allowedStaged, '-SkipTrackingCheck')
    Invoke-Hook -Arguments @('pre-commit', '-StagedFilesPath', $generatedStaged, '-SkipTrackingCheck') -ExpectedExitCode 1

    $env:PSIM_SKIP_PRE_PUSH_TESTS = '1'
    try
    {
        Invoke-Hook -Arguments @('pre-push', '-SkipTrackingCheck')
        Invoke-Hook -Arguments @('pre-push', 'origin', 'https://example.invalid/repository.git', '-SkipTrackingCheck')
    }
    finally
    {
        Remove-Item Env:\PSIM_SKIP_PRE_PUSH_TESTS -ErrorAction SilentlyContinue
    }
}
finally
{
    Remove-Item -LiteralPath $tempDir -Recurse -Force
}

Write-Host '[git-hooks-test] success'
