$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$roadmapPath = Join-Path $repoRoot 'ROADMAP.md'
$progressPath = Join-Path $repoRoot 'PROGRESS.md'
$issuesPath = Join-Path $repoRoot 'ISSUES.md'
$agentsPath = Join-Path $repoRoot 'AGENTS.md'
$trackingPath = Join-Path $repoRoot 'docs\TRACKING.md'

function Fail
{
    param([string]$Message)
    throw "[tracking] $Message"
}

foreach ($path in @($roadmapPath, $progressPath, $issuesPath, $agentsPath, $trackingPath))
{
    if (-not (Test-Path -LiteralPath $path))
    {
        Fail "Missing required tracking file: $path"
    }
}

$agentsText = Get-Content -LiteralPath $agentsPath -Raw
$trackingText = Get-Content -LiteralPath $trackingPath -Raw
if ($agentsText -notmatch '(?m)^## Commit Discipline$')
{
    Fail "AGENTS.md is missing the Commit Discipline section."
}
if ($trackingText -notmatch '(?m)^## Commit Workflow$')
{
    Fail "docs\TRACKING.md is missing the Commit Workflow section."
}
if ($trackingText -notmatch 'Stage only the files intentionally changed')
{
    Fail "docs\TRACKING.md must document path-scoped staging."
}

$roadmapLines = Get-Content -LiteralPath $roadmapPath
$expectedIds = New-Object System.Collections.Generic.List[string]
$stage = $null
$inMarkers = $false
$markerIndex = 0

foreach ($line in $roadmapLines)
{
    if ($line -match '^##\s+(\d+)\.')
    {
        $stage = [int]$Matches[1]
        $inMarkers = $false
        $markerIndex = 0
        continue
    }

    if ($line -eq 'Completion markers:')
    {
        if ($null -eq $stage)
        {
            Fail 'Found completion markers before a numbered stage.'
        }
        $inMarkers = $true
        continue
    }

    if ($inMarkers -and $line -match '^- ')
    {
        ++$markerIndex
        $expectedIds.Add(('R{0}.{1:00}' -f $stage, $markerIndex))
        continue
    }

    if ($inMarkers -and $line.Trim().Length -eq 0 -and $markerIndex -gt 0)
    {
        $inMarkers = $false
        continue
    }

    if ($inMarkers -and $line.Trim().Length -gt 0)
    {
        $inMarkers = $false
    }
}

if ($expectedIds.Count -eq 0)
{
    Fail 'No roadmap completion markers were found.'
}

$progressText = Get-Content -LiteralPath $progressPath -Raw
$allowedProgressStatuses = @('Missing', 'Partial', 'Done', 'Verified', 'Blocked')
$progressIds = [regex]::Matches($progressText, '\|\s+(R\d+\.\d{2})\s+\|') | ForEach-Object { $_.Groups[1].Value }

$duplicates = $progressIds | Group-Object | Where-Object { $_.Count -gt 1 } | ForEach-Object { $_.Name }
if ($duplicates)
{
    Fail "Duplicate progress IDs: $($duplicates -join ', ')"
}

foreach ($id in $expectedIds)
{
    if ($progressIds -notcontains $id)
    {
        Fail "Missing progress row for roadmap marker $id"
    }
}

foreach ($id in $progressIds)
{
    if ($expectedIds -notcontains $id)
    {
        Fail "Progress row references unknown roadmap marker $id"
    }
}

foreach ($line in (Get-Content -LiteralPath $progressPath))
{
    if ($line -match '^\|\s+(R\d+\.\d{2})\s+\|.*?\|\s+([^|]+?)\s+\|')
    {
        $id = $Matches[1]
        $status = $Matches[2].Trim()
        if ($allowedProgressStatuses -notcontains $status)
        {
            Fail "Progress row $id has invalid status '$status'"
        }
    }
}

$issuesText = Get-Content -LiteralPath $issuesPath -Raw
$issueHeadingMatches = [regex]::Matches($issuesText, '(?m)^###\s+(PSIM-\d{4}):\s+(.+)$')
$issueHeadingIds = $issueHeadingMatches | ForEach-Object { $_.Groups[1].Value }

if ($issueHeadingIds.Count -eq 0)
{
    Fail 'No PSIM issue headings were found.'
}

$duplicateIssueHeadings = $issueHeadingIds | Group-Object | Where-Object { $_.Count -gt 1 } | ForEach-Object { $_.Name }
if ($duplicateIssueHeadings)
{
    Fail "Duplicate issue headings: $($duplicateIssueHeadings -join ', ')"
}

$referencedIssueIds = [regex]::Matches($issuesText, 'PSIM-\d{4}') | ForEach-Object { $_.Value } | Sort-Object -Unique
foreach ($referencedIssueId in $referencedIssueIds)
{
    if ($issueHeadingIds -notcontains $referencedIssueId)
    {
        Fail "Referenced issue ID has no detailed heading: $referencedIssueId"
    }
}

$epicMatches = [regex]::Matches($issuesText, '(?m)^##\s+Epic\s+\d+:\s+(.+)$')
if ($epicMatches.Count -eq 0)
{
    Fail 'No epic sections were found.'
}

for ($i = 0; $i -lt $epicMatches.Count; ++$i)
{
    $epic = $epicMatches[$i]
    $start = $epic.Index
    $end = if ($i + 1 -lt $epicMatches.Count) { $epicMatches[$i + 1].Index } else { $issuesText.Length }
    $section = $issuesText.Substring($start, $end - $start)
    if (-not [regex]::IsMatch($section, '(?m)^###\s+PSIM-\d{4}:'))
    {
        Fail "Epic has no issues: $($epic.Value)"
    }
}

$requiredFields = @(
    'Status:',
    'Priority:',
    'Linked roadmap IDs:',
    'Problem:',
    'Acceptance criteria:',
    'Subtasks:',
    'Verification:',
    'Dependencies:',
    'Implementation notes:'
)
$allowedIssueStatuses = @('Open', 'In Progress', 'Blocked', 'Done', 'Deferred')
$allowedPriorities = @('P0', 'P1', 'P2')

for ($i = 0; $i -lt $issueHeadingMatches.Count; ++$i)
{
    $match = $issueHeadingMatches[$i]
    $issueId = $match.Groups[1].Value
    $start = $match.Index
    $end = if ($i + 1 -lt $issueHeadingMatches.Count) { $issueHeadingMatches[$i + 1].Index } else { $issuesText.Length }
    $section = $issuesText.Substring($start, $end - $start)

    foreach ($field in $requiredFields)
    {
        if ($section -notmatch [regex]::Escape($field))
        {
            Fail "$issueId is missing required field '$field'"
        }
    }

    $statusMatch = [regex]::Match($section, '(?m)^Status:\s*(.+)$')
    if (-not $statusMatch.Success -or ($allowedIssueStatuses -notcontains $statusMatch.Groups[1].Value.Trim()))
    {
        Fail "$issueId has invalid status."
    }
    $issueStatus = $statusMatch.Groups[1].Value.Trim()

    $priorityMatch = [regex]::Match($section, '(?m)^Priority:\s*(.+)$')
    if (-not $priorityMatch.Success -or ($allowedPriorities -notcontains $priorityMatch.Groups[1].Value.Trim()))
    {
        Fail "$issueId has invalid priority."
    }

    if ($issueStatus -in @('Open', 'In Progress') -and $section -notmatch '(?m)^Technical implementation direction:')
    {
        Fail "$issueId is $issueStatus but has no 'Technical implementation direction:' handoff section."
    }

    $linkedIds = [regex]::Matches($section, 'R\d+\.\d{2}') | ForEach-Object { $_.Value } | Sort-Object -Unique
    if ($linkedIds.Count -eq 0)
    {
        Fail "$issueId has no linked roadmap IDs."
    }

    foreach ($linkedId in $linkedIds)
    {
        if ($expectedIds -notcontains $linkedId)
        {
            Fail "$issueId links to unknown roadmap ID $linkedId"
        }
    }

    $dependencySectionMatch = [regex]::Match($section, '(?ms)^Dependencies:\s*(.+?)^Implementation notes:')
    if ($dependencySectionMatch.Success)
    {
        $dependencyIds = [regex]::Matches($dependencySectionMatch.Groups[1].Value, 'PSIM-\d{4}') | ForEach-Object { $_.Value } | Sort-Object -Unique
        foreach ($dependencyId in $dependencyIds)
        {
            if ($issueHeadingIds -notcontains $dependencyId)
            {
                Fail "$issueId depends on unknown issue $dependencyId"
            }
        }
    }

    $implementationNotesMatch = [regex]::Match($section, '(?ms)^Implementation notes:\s*(.+)$')
    if ($issueStatus -in @('Done', 'Deferred'))
    {
        if (-not $implementationNotesMatch.Success -or [string]::IsNullOrWhiteSpace($implementationNotesMatch.Groups[1].Value))
        {
            Fail "$issueId is $issueStatus but has no implementation notes."
        }

        $implementationNotes = $implementationNotesMatch.Groups[1].Value.Trim()
        if ($implementationNotes -match '(?m)^-\s+None yet\.\s*$')
        {
            Fail "$issueId is $issueStatus but still has placeholder implementation notes."
        }
    }
}

foreach ($relativePath in @(
    'ROADMAP.md',
    'PROGRESS.md',
    'ISSUES.md',
    'docs\TRACKING.md',
    'scripts\configure.ps1',
    'scripts\build.ps1',
    'scripts\test.ps1',
    'scripts\run-smoke.ps1',
    'scripts\measure-water-solver.ps1',
    'scripts\verify-all.ps1',
    'scripts\package-release.ps1',
    'scripts\verify-demo-scene.ps1',
    'scripts\check-tracking.ps1'
))
{
    $fullPath = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $fullPath))
    {
        Fail "AGENTS.md references missing path: $relativePath"
    }
}

$allowedAbsolutePathFiles = @(
    'scripts\build.ps1',
    'scripts\configure.ps1',
    'scripts\test.ps1',
    'scripts\run-smoke.ps1',
    'scripts\verify-demo-scene.ps1',
    'scripts\verify-demo-scene-density.ps1',
    'scripts\check-tracking.ps1'
)

$absolutePathPattern = '(?i)\b[A-Z]:\\'
foreach ($file in Get-ChildItem -LiteralPath $repoRoot -Recurse -File)
{
    if ($file.FullName -like "$repoRoot\build\*")
    {
        continue
    }

    $relativeFullName = $file.FullName.Substring($repoRoot.Length + 1)
    if ($allowedAbsolutePathFiles -contains $relativeFullName)
    {
        continue
    }

    if ($file.Extension -notin @('.md', '.ps1', '.cpp', '.hpp', '.txt', '.json', '.cmake', '.pscene'))
    {
        continue
    }

    $content = Get-Content -LiteralPath $file.FullName -Raw
    if ($content -match $absolutePathPattern)
    {
        Fail "Machine-specific absolute path found outside helper scripts: $relativeFullName"
    }
}

Write-Host "[tracking] OK: $($expectedIds.Count) roadmap markers, $($issueHeadingIds.Count) issue IDs, $($epicMatches.Count) epics"
