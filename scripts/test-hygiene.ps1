$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$llvmBin = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin'
$clangFormat = (Get-Command clang-format -ErrorAction SilentlyContinue).Path
$clangTidy = (Get-Command clang-tidy -ErrorAction SilentlyContinue).Path
if (-not $clangFormat) { $clangFormat = Join-Path $llvmBin 'clang-format.exe' }
if (-not $clangTidy) { $clangTidy = Join-Path $llvmBin 'clang-tidy.exe' }
if (-not (Test-Path -LiteralPath $clangFormat) -or -not (Test-Path -LiteralPath $clangTidy))
{
    throw '[hygiene-self-test] clang-format and clang-tidy are required.'
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("physics-sim-hygiene-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
try
{
    $badFormat = Join-Path $tempRoot 'bad-format.cpp'
    Set-Content -LiteralPath $badFormat -Encoding utf8 -Value 'int  main(){return 0;}'

    $previousErrorAction = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & $clangFormat --dry-run --Werror $badFormat *> $null
    $formatExit = $LASTEXITCODE
    $ErrorActionPreference = $previousErrorAction
    if ($formatExit -eq 0)
    {
        throw '[hygiene-self-test] deliberate formatting failure was not detected.'
    }

    $badTidy = Join-Path $tempRoot 'bad-tidy.cpp'
    Set-Content -LiteralPath $badTidy -Encoding utf8 -Value @'
int duplicated_branch(bool condition)
{
    if (condition)
    {
        return 1;
    }
    else
    {
        return 1;
    }
}
'@

    $previousErrorAction = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & $clangTidy $badTidy '-checks=-*,bugprone-branch-clone' '--warnings-as-errors=*' '--' '-std=c++20' *> $null
    $tidyExit = $LASTEXITCODE
    $ErrorActionPreference = $previousErrorAction
    if ($tidyExit -eq 0)
    {
        throw '[hygiene-self-test] deliberate clang-tidy failure was not detected.'
    }

    Write-Host '[hygiene-self-test] deliberate format and static-analysis failures were detected'
}
finally
{
    Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
