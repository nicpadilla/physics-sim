param([switch]$All)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$llvmBin = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin'
$clangFormat = (Get-Command clang-format -ErrorAction SilentlyContinue).Path
$clangTidy = (Get-Command clang-tidy -ErrorAction SilentlyContinue).Path
if (-not $clangFormat) { $clangFormat = Join-Path $llvmBin 'clang-format.exe' }
if (-not $clangTidy) { $clangTidy = Join-Path $llvmBin 'clang-tidy.exe' }
if (-not (Test-Path -LiteralPath $clangFormat) -or -not (Test-Path -LiteralPath $clangTidy))
{
    throw '[hygiene] clang-format and clang-tidy are required.'
}

$allCppFiles = @(& git -C $repoRoot ls-files -- '*.cpp' '*.hpp')
if ($All)
{
    $formatRelative = @($allCppFiles)
}
else
{
    $changed = @()
    $base = if ($env:GITHUB_BASE_REF) { "origin/$($env:GITHUB_BASE_REF)" } else { 'HEAD~1' }
    & git -C $repoRoot rev-parse --verify $base *> $null
    if ($LASTEXITCODE -eq 0)
    {
        $changed += @(& git -C $repoRoot diff --name-only --diff-filter=ACMRT "$base...HEAD" -- '*.cpp' '*.hpp')
    }
    $changed += @(& git -C $repoRoot diff --name-only --diff-filter=ACMRT -- '*.cpp' '*.hpp')
    $changed += @(& git -C $repoRoot diff --cached --name-only --diff-filter=ACMRT -- '*.cpp' '*.hpp')
    $formatRelative = @($changed | Where-Object { $_ -and ($allCppFiles -contains $_) } | Sort-Object -Unique)
}

if ($formatRelative.Count -gt 0)
{
    $formatFiles = $formatRelative | ForEach-Object { Join-Path $repoRoot $_ }
    & $clangFormat --dry-run --Werror @formatFiles
    if ($LASTEXITCODE -ne 0) { throw '[hygiene] clang-format check failed.' }
}

$tidySources = @(
    'src\core\simulation.cpp',
    'src\core\water_feel_metrics.cpp',
    'src\content\content_module.cpp',
    'src\app\surface_reconstruction.cpp',
    'src\app\water_visual_effects.cpp'
)
$tidyRoot = Join-Path $repoRoot 'build\windows-x64\clang-tidy'
New-Item -ItemType Directory -Path $tidyRoot -Force | Out-Null
foreach ($relative in $tidySources)
{
    $source = Join-Path $repoRoot $relative
    $log = Join-Path $tidyRoot (($relative -replace '[\\/]', '-') + '.log')
    $previousErrorAction = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & $clangTidy $source '-checks=-*,bugprone-*,performance-*' '--' '-std=c++20' "-I$(Join-Path $repoRoot 'include')" '-DNOMINMAX' '-DWIN32_LEAN_AND_MEAN' 2>&1 |
        Set-Content -LiteralPath $log
    $tidyExit = $LASTEXITCODE
    $ErrorActionPreference = $previousErrorAction
    if ($tidyExit -ne 0) { throw "[hygiene] clang-tidy failed for $relative; log=$log" }
}

$secretPattern = '(?i)(-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----|ghp_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}|AKIA[0-9A-Z]{16})'
$trackedFiles = & git -C $repoRoot ls-files
foreach ($relativePath in $trackedFiles)
{
    $path = Join-Path $repoRoot $relativePath
    if (Test-Path -LiteralPath $path -PathType Leaf)
    {
        $match = Select-String -LiteralPath $path -Pattern $secretPattern -Quiet -ErrorAction SilentlyContinue
        if ($match) { throw "[hygiene] possible secret in $relativePath" }
    }
}

& (Join-Path $PSScriptRoot 'check-dependencies.ps1')
& (Join-Path $PSScriptRoot 'check-tracking.ps1')
Write-Host "[hygiene] format_files=$($formatRelative.Count) tidy_files=$($tidySources.Count); dependency, tracking, secret, and path checks passed"
