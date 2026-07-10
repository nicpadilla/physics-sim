$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

function Fail
{
    param([string]$Message)
    throw "[dependencies] $Message"
}

function Assert-NoIncludes
{
    param(
        [string]$RelativeDirectory,
        [string[]]$ForbiddenPatterns
    )

    $directory = Join-Path $repoRoot $RelativeDirectory
    foreach ($file in Get-ChildItem -LiteralPath $directory -Recurse -File -Include *.cpp,*.hpp)
    {
        $content = Get-Content -LiteralPath $file.FullName -Raw
        foreach ($pattern in $ForbiddenPatterns)
        {
            if ($content -match $pattern)
            {
                Fail "$RelativeDirectory contains forbidden dependency '$pattern' in $($file.Name)"
            }
        }
    }
}

Assert-NoIncludes 'src\core' @('(?i)#include\s*[<"]SDL', '(?i)#include\s*[<"]imgui')
Assert-NoIncludes 'src\content' @('(?i)#include\s*[<"]SDL', '(?i)#include\s*[<"]imgui')
Assert-NoIncludes 'src\app' @('(?i)#include\s*[<"]imgui')

$mainText = Get-Content -LiteralPath (Join-Path $repoRoot 'src\main.cpp') -Raw
$allowedMainIncludes = @('physics_sim/application.hpp', 'physics_sim/lab_application.hpp', 'cstdlib', 'string_view', 'Windows.h')
$mainIncludes = [regex]::Matches($mainText, '(?m)^#include\s*[<"]([^>"]+)[>"]') | ForEach-Object { $_.Groups[1].Value }
foreach ($include in $mainIncludes)
{
    if ($allowedMainIncludes -notcontains $include)
    {
        Fail "src\main.cpp includes non-composition dependency '$include'"
    }
}

Write-Host '[dependencies] OK: core/content are platform independent; app excludes ImGui; main is composition only'
