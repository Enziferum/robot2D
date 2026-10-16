param(
    [string[]]$Files,
    [string]$FilesFrom,
    [string[]]$Scope = @('core/include', 'core/src', 'core/tests', 'core/sandbox', 'editor/include', 'editor/src', 'editor/tests'),
    [string]$BuildDirectory = 'cmake-build-debug',
    [string]$ClangTidy = 'clang-tidy',
    [switch]$ListOnly,
    [switch]$WarningsAsErrors
)

$ErrorActionPreference = 'Stop'
$taskDefaultScope = !$PSBoundParameters.ContainsKey('Scope')
Push-Location (Join-Path $PSScriptRoot '..')
try {
    $taskRoot = (Get-Location).Path
    if($FilesFrom) {
        if($Files) { throw 'Use Files or FilesFrom, not both.' }
        $Files = @((Get-Content -LiteralPath $FilesFrom -Raw | ConvertFrom-Json))
        if(!$Files.Count) { throw 'FilesFrom must contain a non-empty JSON array.' }
    }
    $taskRootPrefix = $taskRoot.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar
    $taskScopes = @($Scope | ForEach-Object {
        $taskResolved = (Resolve-Path -LiteralPath $_).Path.TrimEnd('\', '/')
        if(!$taskResolved.StartsWith($taskRootPrefix, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Scope must be a specific directory or file inside the repository: $_"
        }
        $taskResolved
    })
    if(!$taskScopes.Count) { throw 'At least one scope is required.' }

    function Test-TaskScope([string]$Path) {
        if($taskDefaultScope) {
            foreach($taskVendor in @('core/src/Graphics/internal', 'core/include/robot2D/Graphics/internal')) {
                $taskVendorPath = Join-Path $taskRoot $taskVendor
                if($Path.StartsWith($taskVendorPath + [IO.Path]::DirectorySeparatorChar,
                                    [StringComparison]::OrdinalIgnoreCase)) { return $false }
            }
        }
        foreach($taskSelectedScope in $taskScopes) {
            if($Path.Equals($taskSelectedScope, [StringComparison]::OrdinalIgnoreCase) -or
               $Path.StartsWith($taskSelectedScope + [IO.Path]::DirectorySeparatorChar,
                                [StringComparison]::OrdinalIgnoreCase)) { return $true }
        }
        return $false
    }

    $taskDatabase = Join-Path $BuildDirectory 'compile_commands.json'
    if(!(Test-Path -LiteralPath $taskDatabase)) {
        throw "Missing $taskDatabase. Configure this build with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON."
    }

    $taskEntries = @(Get-Content -LiteralPath $taskDatabase -Raw | ConvertFrom-Json)
    $taskCompiledFiles = @($taskEntries | ForEach-Object {
        $taskSource = $_.file
        if(![IO.Path]::IsPathRooted($taskSource)) { $taskSource = Join-Path $_.directory $taskSource }
        [IO.Path]::GetFullPath($taskSource)
    } | Sort-Object -Unique)
    if($Files) {
        $taskSelectedFiles = @($Files | ForEach-Object {
            $taskSource = (Resolve-Path -LiteralPath $_).Path
            if(!(Test-TaskScope $taskSource)) {
                throw "File is outside the selected scope: $_. Select its directory with -Scope explicitly."
            }
            if($taskCompiledFiles -notcontains $taskSource) { throw "File has no compilation command: $_" }
            $taskSource
        } | Sort-Object -Unique)
    }
    else {
        $taskSelectedFiles = @($taskCompiledFiles | Where-Object { Test-TaskScope $_ })
    }
    if(!$taskSelectedFiles.Count) { throw 'No compilation units found in the selected scope.' }
    if($ListOnly) {
        $taskSelectedFiles
        exit 0
    }

    # Anchor to the exact selected paths: a similarly named directory in a dependency cannot match.
    $taskPatterns = @($taskScopes | ForEach-Object {
        $taskPattern = $_.Replace('\', '/') -replace '([.\^$*+?()\[\]{}|])', '\$1'
        $taskPattern = $taskPattern.Replace('/', '[/\\]')
        "$taskPattern([/\\]|$)"
    })
    $taskHeaderFilter = '^(' + ($taskPatterns -join '|') + ')'
    $taskExcludeFilter = '^$'
    if($taskDefaultScope) {
        $taskExcludeFilter = '[/\\]core[/\\](src[/\\]Graphics|include[/\\]robot2D[/\\]Graphics)[/\\]internal[/\\]'
    }
    $taskArguments = @('-p', $BuildDirectory, "--config-file=$taskRoot/.clang-tidy",
                       "--header-filter=$taskHeaderFilter", "--exclude-header-filter=$taskExcludeFilter")
    if($WarningsAsErrors) { $taskArguments += '--warnings-as-errors=*' }
    $taskCache = Join-Path $BuildDirectory 'CMakeCache.txt'
    $taskCompilerEntry = Select-String -LiteralPath $taskCache -Pattern '^CMAKE_CXX_COMPILER:(FILEPATH|STRING)=(.+)$'
    if(!$taskCompilerEntry) {
        throw "CMAKE_CXX_COMPILER not found in $taskCache."
    }
    $taskCompiler = $taskCompilerEntry.Matches[0].Groups[2].Value
    $taskTarget = (& $taskCompiler -dumpmachine 2>$null | Select-Object -First 1)
    if($LASTEXITCODE -eq 0 -and $taskTarget -match 'mingw') {
        # CMake's database omits the GCC driver's implicit include directories.
        # Use the configured compiler's ordered search list, not local version guesses.
        $taskArguments += "--extra-arg=--target=$taskTarget"
        # Clang must parse its own intrinsic headers before GCC's builtin headers.
        # Resolve the matching driver beside the selected clang-tidy installation.
        $taskTidyExecutable = (Get-Command $ClangTidy -ErrorAction Stop).Source
        $taskClangDriver = Join-Path (Split-Path $taskTidyExecutable -Parent) ('clang' + [IO.Path]::GetExtension($taskTidyExecutable))
        if(Test-Path -LiteralPath $taskClangDriver -PathType Leaf) {
            $taskResourceDirectory = (& $taskClangDriver -print-resource-dir 2>$null | Select-Object -First 1)
            if($LASTEXITCODE -ne 0) { throw 'Could not read the Clang resource directory.' }
            $taskResourceIncludes = Join-Path $taskResourceDirectory 'include'
            if(!(Test-Path -LiteralPath $taskResourceIncludes -PathType Container)) {
                throw "Clang resource headers not found: $taskResourceIncludes"
            }
            $taskArguments += "--extra-arg=-isystem$taskResourceIncludes"
        }
        $taskSearch = @(& $taskCompiler -E -x c++ -v NUL 2>&1 | ForEach-Object { "$_" })
        if($LASTEXITCODE -ne 0) {
            throw 'Could not read MinGW include search paths.'
        }
        $taskInSearch = $false
        foreach($taskLine in $taskSearch) {
            if($taskLine -match '^#include <\.\.\.> search starts here:') {
                $taskInSearch = $true
                continue
            }
            if($taskLine -match '^End of search list\.') { break }
            if($taskInSearch) {
                $taskInclude = $taskLine.Trim()
                if(Test-Path -LiteralPath $taskInclude -PathType Container) {
                    $taskArguments += "--extra-arg=-isystem$taskInclude"
                }
            }
        }
    }

    & $ClangTidy @taskArguments @taskSelectedFiles
    $taskExitCode = $LASTEXITCODE
}
finally {
    Pop-Location
}
exit $taskExitCode
