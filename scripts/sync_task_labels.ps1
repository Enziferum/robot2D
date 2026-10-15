param(
    [switch]$Apply
)

$ErrorActionPreference = 'Stop'
$taskRoot = Split-Path $PSScriptRoot -Parent
$taskRegistry = Get-Content -LiteralPath (Join-Path $taskRoot '.github/tasks.json') -Raw | ConvertFrom-Json
$taskLabels = Get-Content -LiteralPath (Join-Path $taskRoot '.github/task-labels.json') -Raw | ConvertFrom-Json

# Validate the whole plan before making any remote changes.
foreach($taskEntry in $taskLabels.tasks.PSObject.Properties) {
    if(!$taskRegistry.tasks.PSObject.Properties[$taskEntry.Name] -or
       !$taskRegistry.github.issues.PSObject.Properties[$taskEntry.Name]) {
        throw "Task has no registry/Issue mapping: $($taskEntry.Name)"
    }
    foreach($taskLabel in $taskEntry.Value) {
        if(!$taskLabels.definitions.PSObject.Properties[$taskLabel]) {
            throw "Undefined label: $taskLabel"
        }
    }
}

if(!$Apply) {
    foreach($taskEntry in $taskLabels.tasks.PSObject.Properties) {
        $taskNumber = $taskRegistry.github.issues.($taskEntry.Name)
        Write-Output "$($taskEntry.Name) (#${taskNumber}): $($taskEntry.Value -join ', ')"
    }
    Write-Output 'Preview only. Use -Apply to add these labels through GitHub CLI.'
    return
}

$taskGhCommand = Get-Command gh -ErrorAction SilentlyContinue
$taskGh = if($taskGhCommand) { $taskGhCommand.Source } else { Join-Path $taskRoot 'work/github-cli/bin/gh.exe' }
if(!(Test-Path -LiteralPath $taskGh)) { throw 'GitHub CLI is required. Install gh and run gh auth login.' }
$taskRepo = $taskRegistry.github.repository
$taskRemoteLabels = & $taskGh label list --repo $taskRepo --limit 1000 --json name
if($LASTEXITCODE -ne 0) { throw 'Cannot read repository labels.' }
$taskKnownNames = @((($taskRemoteLabels -join "`n") | ConvertFrom-Json) | ForEach-Object name)
foreach($taskDefinition in $taskLabels.definitions.PSObject.Properties) {
    if($taskKnownNames -notcontains $taskDefinition.Name) {
        & $taskGh label create $taskDefinition.Name --repo $taskRepo --color $taskDefinition.Value.color --description $taskDefinition.Value.description
        if($LASTEXITCODE -ne 0) { throw "Cannot create label $($taskDefinition.Name)" }
    }
}
foreach($taskEntry in $taskLabels.tasks.PSObject.Properties) {
    $taskNumber = $taskRegistry.github.issues.($taskEntry.Name)
    # Confirm the mapping before touching the Issue. Preserve all existing labels.
    $taskTitle = & $taskGh issue view $taskNumber --repo $taskRepo --json title --jq .title
    if($LASTEXITCODE -ne 0 -or !$taskTitle.StartsWith("[$($taskEntry.Name)] ")) {
        throw "Issue #$taskNumber does not match $($taskEntry.Name)"
    }
    & $taskGh issue edit $taskNumber --repo $taskRepo --add-label ($taskEntry.Value -join ',')
    if($LASTEXITCODE -ne 0) { throw "Cannot label $($taskEntry.Name)" }
}
