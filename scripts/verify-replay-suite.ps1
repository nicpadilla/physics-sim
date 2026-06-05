$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\windows-x64'
$suiteDir = Join-Path $buildDir 'replay-suite'
$suiteLog = Join-Path $buildDir 'replay-suite.log'
$manifestPath = Join-Path $repoRoot 'regression\replay_suite.psd1'
$exe = Join-Path $buildDir 'Debug\physics-sim.exe'

function Write-SuiteLog
{
    param([string]$Message)
    Add-Content -LiteralPath $suiteLog -Value $Message
}

function Write-CaseLog
{
    param(
        [string]$Path,
        [string]$Message
    )

    Add-Content -LiteralPath $Path -Value $Message
}

New-Item -ItemType Directory -Path $suiteDir -Force | Out-Null
Set-Content -LiteralPath $suiteLog -Value "[replay-suite] starting"
Write-SuiteLog "[replay-suite] manifest: $manifestPath"

if (-not (Test-Path -LiteralPath $exe))
{
    Write-SuiteLog "[replay-suite] missing exe"
    throw "Could not find $exe. Run .\scripts\build.ps1 first."
}

if (-not (Test-Path -LiteralPath $manifestPath))
{
    Write-SuiteLog "[replay-suite] missing manifest"
    throw "Missing replay manifest: $manifestPath"
}

$cases = Import-PowerShellDataFile -LiteralPath $manifestPath
if (-not $cases)
{
    Write-SuiteLog "[replay-suite] manifest had no cases"
    throw "Replay manifest did not define any cases: $manifestPath"
}

foreach ($case in $cases)
{
    $caseName = [string]$case.Name
    $baseline = Join-Path $repoRoot ([string]$case.Baseline)
    $replayFile = Join-Path $repoRoot ([string]$case.ReplayFile)
    $capture = Join-Path $suiteDir ("{0}-capture.bmp" -f $caseName)
    $caseLog = Join-Path $suiteDir ("{0}.log" -f $caseName)
    $appLog = Join-Path $suiteDir ("{0}-app.log" -f $caseName)
    $settings = Join-Path $suiteDir ("{0}-settings.txt" -f $caseName)

    Set-Content -LiteralPath $caseLog -Value ("[replay-suite] case={0}" -f $caseName)
    Write-CaseLog $caseLog ("[replay-suite] baseline: {0}" -f $baseline)
    Write-CaseLog $caseLog ("[replay-suite] capture:  {0}" -f $capture)
    Write-SuiteLog ("[replay-suite] case {0}: starting" -f $caseName)

    if (-not (Test-Path -LiteralPath $baseline))
    {
        Write-CaseLog $caseLog "[replay-suite] missing baseline"
        throw "Missing replay baseline: $baseline"
    }

    if (-not (Test-Path -LiteralPath $replayFile))
    {
        Write-CaseLog $caseLog "[replay-suite] missing replay file"
        throw "Missing replay file: $replayFile"
    }

    foreach ($path in @($capture, $appLog, $settings))
    {
        if (Test-Path -LiteralPath $path)
        {
            Remove-Item -LiteralPath $path -Force
        }
    }

    $caseSucceeded = $false
    try
    {
        $arguments = @(
            '--log-file', $appLog,
            '--settings-file', $settings,
            '--skip-session-shell',
            '--replay-file', $replayFile,
            '--dump-frame', $capture
        ) + @($case.Arguments)

        Write-CaseLog $caseLog "[replay-suite] launching app"
        $process = Start-Process -FilePath $exe -ArgumentList $arguments -Wait -PassThru -WorkingDirectory $repoRoot
        Write-CaseLog $caseLog ("[replay-suite] app exit code: {0}" -f $process.ExitCode)

        if ($process.ExitCode -ne 0)
        {
            throw "physics-sim.exe returned exit code $($process.ExitCode)"
        }

        if (-not (Test-Path -LiteralPath $capture))
        {
            Write-CaseLog $caseLog "[replay-suite] capture missing"
            throw "Replay capture was not written to $capture"
        }

        $baselineHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $baseline).Hash
        $captureHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $capture).Hash
        Write-CaseLog $caseLog ("[replay-suite] baseline hash: {0}" -f $baselineHash)
        Write-CaseLog $caseLog ("[replay-suite] capture hash:  {0}" -f $captureHash)

        if ($baselineHash -ne $captureHash)
        {
            Write-CaseLog $caseLog "[replay-suite] hash mismatch"
            throw "Replay suite mismatch for $caseName.`nBaseline: $baselineHash`nCapture:  $captureHash"
        }

        Write-CaseLog $caseLog "[replay-suite] hash match"
        $caseSucceeded = $true
        Write-SuiteLog ("[replay-suite] case {0}: succeeded" -f $caseName)
    }
    catch
    {
        Write-SuiteLog ("[replay-suite] case {0}: failed: {1}" -f $caseName, $_.Exception.Message)
        throw
    }
    finally
    {
        if ($caseSucceeded -and (Test-Path -LiteralPath $capture))
        {
            Remove-Item -LiteralPath $capture -Force
            Write-CaseLog $caseLog "[replay-suite] capture removed"
        }
    }
}

Write-SuiteLog "[replay-suite] success"
Write-Host '[replay-suite] success'
