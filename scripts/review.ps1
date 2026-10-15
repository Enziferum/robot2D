param(
    [string[]]$Files,
    [string]$Base,
    [string]$Commit,
    [string]$Focus,
    [string]$Model,
    [string]$Codex = 'codex',
    [string]$BuildDirectory,
    [switch]$RunChecks,
    [switch]$PrepareOnly
)

$ErrorActionPreference = 'Stop'
$taskArguments = @((Join-Path $PSScriptRoot 'review.py'), '--codex', $Codex)
if($Base) { $taskArguments += @('--base', $Base) }
if($Commit) { $taskArguments += @('--commit', $Commit) }
if($Focus) { $taskArguments += @('--focus', $Focus) }
if($Model) { $taskArguments += @('--model', $Model) }
if($BuildDirectory) { $taskArguments += @('--build-directory', $BuildDirectory) }
if($RunChecks) { $taskArguments += '--run-checks' }
if($PrepareOnly) { $taskArguments += '--prepare-only' }
if($Files) { $taskArguments += '--files'; $taskArguments += $Files }
$taskPreviousEncoding = $env:PYTHONIOENCODING
try {
    $env:PYTHONIOENCODING = 'utf-8'
    & python @taskArguments
    $taskExitCode = $LASTEXITCODE
}
finally {
    $env:PYTHONIOENCODING = $taskPreviousEncoding
}
exit $taskExitCode
