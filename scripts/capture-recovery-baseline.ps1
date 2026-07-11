param(
    [string]$OutputDirectory,
    [string]$ReportPath,
    [switch]$FailOnStepFailure
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $repoRoot 'build\windows-x64'
if (-not $OutputDirectory)
{
    $OutputDirectory = Join-Path $buildRoot 'recovery-baseline'
}
if (-not [System.IO.Path]::IsPathRooted($OutputDirectory))
{
    $OutputDirectory = Join-Path $repoRoot $OutputDirectory
}
if ($ReportPath -and -not [System.IO.Path]::IsPathRooted($ReportPath))
{
    $ReportPath = Join-Path $repoRoot $ReportPath
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$stepsDirectory = Join-Path $OutputDirectory 'steps'
New-Item -ItemType Directory -Path $stepsDirectory -Force | Out-Null

$started = Get-Date
$results = [System.Collections.Generic.List[object]]::new()

function Get-RepoRelativePath
{
    param([string]$Path)
    $resolved = [System.IO.Path]::GetFullPath($Path)
    return $resolved.Substring($repoRoot.Length + 1).Replace('\', '/')
}

function Invoke-BaselineStep
{
    param(
        [string]$Name,
        [string]$Command,
        [scriptblock]$Action,
        [string]$Profile = '',
        [string]$Scenario = ''
    )

    Write-Host "[recovery-baseline] $Name"
    $safeName = $Name.ToLowerInvariant() -replace '[^a-z0-9]+', '-'
    $logPath = Join-Path $stepsDirectory "$safeName.log"
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $exitCode = 0
    $status = 'passed'
    $message = ''
    try
    {
        & $Action *>&1 | Tee-Object -LiteralPath $logPath
        if ($LASTEXITCODE -and $LASTEXITCODE -ne 0)
        {
            throw "process returned exit code $LASTEXITCODE"
        }
    }
    catch
    {
        $exitCode = if ($LASTEXITCODE) { $LASTEXITCODE } else { 1 }
        $status = 'failed'
        $message = $_.Exception.Message
        Add-Content -LiteralPath $logPath -Value "[recovery-baseline] failure: $message"
    }
    finally
    {
        $stopwatch.Stop()
    }

    $results.Add([pscustomobject]@{
        name = $Name
        command = $Command
        status = $status
        exit_code = $exitCode
        duration_seconds = [math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
        profile = $Profile
        scenario = $Scenario
        log = Get-RepoRelativePath $logPath
        message = $message
    })
}

$gitCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
$gitBranch = (& git -C $repoRoot branch --show-current).Trim()
$bundledCMake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmake = if ($cmakeCommand) { $cmakeCommand.Path } elseif (Test-Path $bundledCMake) { $bundledCMake } else { $null }
$cmakeVersion = if ($cmake) { (& $cmake --version | Select-Object -First 1) } else { 'cmake unavailable' }
$processor = (Get-CimInstance Win32_Processor | Select-Object -First 1 -ExpandProperty Name).Trim()
$operatingSystem = Get-CimInstance Win32_OperatingSystem
$vsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$visualStudioValue = if (Test-Path $vsWhere) { & $vsWhere -latest -property catalog_productDisplayVersion } else { $null }
$visualStudio = if ($visualStudioValue) { ([string]$visualStudioValue).Trim() } else { 'unknown' }
$windowsSdk = Get-ChildItem 'HKLM:\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0' -ErrorAction SilentlyContinue |
    ForEach-Object { (Get-ItemProperty $_.PSPath -ErrorAction SilentlyContinue).ProductVersion } |
    Select-Object -First 1

$environment = [ordered]@{
    captured_at = $started.ToString('o')
    commit = $gitCommit
    branch = $gitBranch
    processor = $processor
    operating_system = "$($operatingSystem.Caption) $($operatingSystem.Version)"
    visual_studio = $visualStudio
    windows_sdk = if ($windowsSdk) { $windowsSdk } else { 'unknown' }
    cmake = $cmakeVersion
    configuration = 'Debug'
    build_directory = 'build/windows-x64'
}

Invoke-BaselineStep 'tracking' '.\scripts\check-tracking.ps1' { & (Join-Path $PSScriptRoot 'check-tracking.ps1') }
Invoke-BaselineStep 'build' '.\scripts\build.ps1' { & (Join-Path $PSScriptRoot 'build.ps1') }
Invoke-BaselineStep 'ctest-all' '.\scripts\test.ps1' { & (Join-Path $PSScriptRoot 'test.ps1') } -Profile 'all' -Scenario '26 legacy CTest targets'
Invoke-BaselineStep 'smoke' '.\scripts\run-smoke.ps1' { & (Join-Path $PSScriptRoot 'run-smoke.ps1') } -Profile 'balanced' -Scenario 'application startup'
Invoke-BaselineStep 'replay-suite' '.\scripts\verify-replay-suite.ps1' { & (Join-Path $PSScriptRoot 'verify-replay-suite.ps1') } -Profile 'balanced' -Scenario 'legacy replay manifest'
Invoke-BaselineStep 'fluid-quality-suite' '.\scripts\verify-fluid-quality-suite.ps1' { & (Join-Path $PSScriptRoot 'verify-fluid-quality-suite.ps1') } -Profile 'balanced,quality' -Scenario 'legacy canonical scenarios'
Invoke-BaselineStep 'solver-profiles' '.\scripts\measure-water-solver.ps1 -Profile All' { & (Join-Path $PSScriptRoot 'measure-water-solver.ps1') -Profile All } -Profile 'fast,balanced,quality' -Scenario 'small-container-stress,demo-grid-flow'
Invoke-BaselineStep 'basin-early-surface' '.\scripts\verify-recovery-basin.ps1' { & (Join-Path $PSScriptRoot 'verify-recovery-basin.ps1') } -Profile 'balanced' -Scenario 'Recovery basin tick 240'
Invoke-BaselineStep 'basin-density' '.\scripts\verify-recovery-basin-density.ps1' { & (Join-Path $PSScriptRoot 'verify-recovery-basin-density.ps1') } -Profile 'balanced' -Scenario 'Recovery basin tick 2400'
Invoke-BaselineStep 'basin-surface' '.\scripts\verify-recovery-basin-surface.ps1' { & (Join-Path $PSScriptRoot 'verify-recovery-basin-surface.ps1') } -Profile 'balanced' -Scenario 'Recovery basin tick 2400'
Invoke-BaselineStep 'package' '.\scripts\package-release.ps1' { & (Join-Path $PSScriptRoot 'package-release.ps1') } -Profile 'balanced' -Scenario 'legacy package'

$goldens = Get-ChildItem -LiteralPath (Join-Path $repoRoot 'regression') -Filter '*.bmp' | ForEach-Object {
    [pscustomobject]@{
        path = Get-RepoRelativePath $_.FullName
        sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash
        bytes = $_.Length
    }
}

$bundle = [ordered]@{
    schema_version = 1
    environment = $environment
    results = $results
    legacy_goldens = $goldens
    completed_at = (Get-Date).ToString('o')
}
$jsonPath = Join-Path $OutputDirectory 'baseline-summary.json'
$bundle | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $jsonPath -Encoding utf8

$markdown = [System.Collections.Generic.List[string]]::new()
$backtick = [char]96
$markdown.Add('# Recovery Baseline')
$markdown.Add('')
$markdown.Add("Captured at: $backtick$($environment.captured_at)$backtick  ")
$markdown.Add("Commit: $backtick$gitCommit$backtick  ")
$markdown.Add("Branch: $backtick$gitBranch$backtick  ")
$markdown.Add("Host: $backtick$processor$backtick; $($environment.operating_system)  ")
$markdown.Add("Toolchain: Visual Studio $visualStudio; Windows SDK $($environment.windows_sdk); $cmakeVersion")
$markdown.Add('')
$markdown.Add('## Command Results')
$markdown.Add('')
$markdown.Add('| Step | Status | Duration (s) | Profile | Scenario | Log |')
$markdown.Add('| --- | --- | ---: | --- | --- | --- |')
foreach ($result in $results)
{
    $markdown.Add("| $($result.name) | $($result.status) | $($result.duration_seconds) | $($result.profile) | $($result.scenario) | $backtick$($result.log)$backtick |")
}
$markdown.Add('')
$markdown.Add('## Evidence Boundary')
$markdown.Add('')
$markdown.Add('- This bundle records reproducibility and current numeric/log output; it does not grant human acceptance.')
$markdown.Add('- The legacy fluid harness does not produce an image for every canonical numeric scenario. That is recorded as a recovery tooling gap rather than silently treated as visual evidence.')
$markdown.Add('- Legacy golden hashes are listed in `baseline-summary.json` and remain historical evidence only.')
$generatedReport = Join-Path $OutputDirectory 'baseline-report.md'
$markdown | Set-Content -LiteralPath $generatedReport -Encoding utf8

if ($ReportPath)
{
    $parent = Split-Path -Parent $ReportPath
    New-Item -ItemType Directory -Path $parent -Force | Out-Null
    Copy-Item -LiteralPath $generatedReport -Destination $ReportPath -Force
}

$failed = @($results | Where-Object { $_.status -ne 'passed' })
Write-Host "[recovery-baseline] summary: $jsonPath"
Write-Host "[recovery-baseline] report:  $generatedReport"
if ($failed.Count -gt 0)
{
    Write-Warning "Recovery baseline recorded $($failed.Count) failed step(s). See the report and structured summary."
    if ($FailOnStepFailure)
    {
        throw "Recovery baseline completed with $($failed.Count) failed step(s)."
    }
}
