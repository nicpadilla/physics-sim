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

$formatFiles = @(
    'src\core\simulation.cpp',
    'src\content\content_module.cpp',
    'src\lab\lab_module.cpp',
    'src\main.cpp'
) | ForEach-Object { Join-Path $repoRoot $_ }
& $clangFormat --dry-run --Werror @formatFiles
if ($LASTEXITCODE -ne 0) { throw '[hygiene] clang-format check failed.' }

$tidyLog = Join-Path $repoRoot 'build\windows-x64\clang-tidy.log'
$previousErrorAction = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
& $clangTidy (Join-Path $repoRoot 'src\core\simulation.cpp') '-checks=-*,bugprone-*,performance-*' '--' '-std=c++20' "-I$(Join-Path $repoRoot 'include')" '-DNOMINMAX' '-DWIN32_LEAN_AND_MEAN' 2>&1 |
    Set-Content -LiteralPath $tidyLog
$tidyExit = $LASTEXITCODE
$ErrorActionPreference = $previousErrorAction
if ($tidyExit -ne 0) { throw "[hygiene] clang-tidy failed; log=$tidyLog" }

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
Write-Host "[hygiene] format, tidy, dependency, secret, and path checks passed; tidy_log=$tidyLog"
