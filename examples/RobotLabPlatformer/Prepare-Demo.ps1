param([string]$Destination = (Join-Path $PSScriptRoot 'work/RobotLabPlatformer'))
$ErrorActionPreference = 'Stop'
$taskSource = [IO.Path]::GetFullPath($PSScriptRoot)
$taskTarget = [IO.Path]::GetFullPath($Destination)
if((Split-Path $taskTarget -Leaf) -ne 'RobotLabPlatformer') {
    throw 'Destination folder must be named RobotLabPlatformer (project browser convention).'
}
if(Test-Path -LiteralPath $taskTarget) {
    throw "Destination already exists; choose a new directory to preserve local edits: $taskTarget"
}
New-Item -ItemType Directory -Path $taskTarget -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $taskSource 'assets'),(Join-Path $taskSource '.robot2d') -Destination $taskTarget -Recurse
$taskData = Get-Content -LiteralPath (Join-Path $taskSource 'RobotLabPlatformer.robot2D') -Raw | ConvertFrom-Json
$taskData.Path = $taskTarget.Replace('\','/')
$taskData | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskTarget 'RobotLabPlatformer.robot2D') -Encoding utf8
Write-Output "Ready: Add existing -> $taskTarget. The editor builds C# on opening/Play."
