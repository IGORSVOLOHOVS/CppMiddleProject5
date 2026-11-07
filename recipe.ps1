param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('build', 'install', 'uninstall', 'clean', 'run', 'publish', 'info', 'test')]
    [string]$Verb,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$RunArgs
)

# Everything down to $OverridePhrase is read as TEXT by the git hooks and the
# pipeline: one literal per line, no variables, no concatenation.
$Name    = 'cpp-middle-project5'# the repository, lower-case kebab-case
$Code    = 'cmp5'               # the project code; the first word of every branch
$Version = '1.0.0'
$Summary = 'Two-dimensional geometry in C++23: shapes, intersections, convex hull and Delaunay triangulation.'
$Needs   = @()                   # other lab projects, name@tag - this one depends on none
$Tools   = @()                   # toolchains the lab must fetch - mingw g++, cmake, ninja and LLVM are on the machine

# A console demo taken as a library on purpose: no $App, no installer.
# The computational core: what `publish` hands over, what the wiki reads, and where `test` allows no lock and no exception.
$Provides = @('include', 'src')

$CommitTypes  = @('feat', 'fix', 'docs', 'refactor', 'test', 'build', 'ci', 'chore')
$CommitScopes = @('core', 'build', 'ci', 'docs', 'deps')
$BranchTypes  = @('feat', 'fix', 'docs', 'refactor', 'test', 'build', 'ci', 'chore', 'hotfix', 'experiment')
$Reviewers    = @('igors')
$OverridePhrase = '1234567890'   # section 8 - the owner's override; a mark, not a password

# Native tools report on stderr and their exit code is the verdict, so a line on stderr must not stop the verb.
$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot

# The course throws on purpose in these files, and the exception issue names them. They leave the word
# search for exceptions and stay under this exact count instead, so a new throw there still fails.
$ThrowsOnPurpose = @('include/geometry.hpp', 'include/intersections.hpp', 'src/main.cpp')
$ThrowLinesOnPurpose = 4

# ValueFromRemainingArguments leaves $RunArgs null when nothing follows the verb.
if ($null -eq $RunArgs) { $RunArgs = @() }

# .claude/.env is gitignored here and never committed. A variable already set wins.
$envFile = Join-Path $PSScriptRoot '.claude\.env'
if (Test-Path $envFile) {
    foreach ($line in Get-Content -Encoding UTF8 $envFile) {
        if ($line -match '^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*?)\s*$' -and
            -not (Test-Path "env:$($Matches[1])")) {
            Set-Item "env:$($Matches[1])" $Matches[2].Trim('"', "'")
        }
    }
}

# The runner starts with neither folder on PATH. mingw goes first, so the test binaries load their libstdc++.
foreach ($folder in 'C:\Program Files\LLVM\bin', 'C:\ProgramData\mingw64\mingw64\bin') {
    if (Test-Path $folder) { $env:Path = "$folder;$env:Path" }
}

function Build-Core {
    # g++ by name: Ninja left to choose finds MSVC's linker and fails its test program.
    cmake -S . -B build -G Ninja '-DCMAKE_CXX_COMPILER=g++' '-DCMAKE_BUILD_TYPE=Release' '-DGEOMETRY_APP_DEMO=OFF'
    if ($LASTEXITCODE -ne 0) { exit 1 }
    cmake --build build
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

function Assert-NoWordInCore([string]$What, [string]$Words, [string[]]$Spared = @()) {
    # A folder with no tracked file would make the search below find nothing and read as a pass.
    foreach ($folder in $Provides) {
        if (-not (git ls-files -- $folder)) { Write-Host "$folder holds no tracked file, so the $What check would prove nothing."; exit 1 }
    }
    $where = @($Provides) + @($Spared | ForEach-Object { ":(exclude)$_" })
    git grep -n -I -w -E --untracked $Words -- @where
    # 1 is the verdict wanted: found nothing. 0 is a hit, and anything else is a search that did not run.
    if ($LASTEXITCODE -ne 1) { Write-Host "The $What check failed: exit $LASTEXITCODE, over $($Provides -join ', ')."; exit 1 }
    Write-Host "No $What in $($Provides -join ', ')$(if ($Spared) { ", but for $($Spared -join ', ')" })."
}

function Assert-ThrowsOnPurposeDidNotGrow([string]$Words) {
    foreach ($file in $ThrowsOnPurpose) {
        if (-not (git ls-files -- $file)) { Write-Host "$file is gone, so it must leave `$ThrowsOnPurpose."; exit 1 }
    }
    $lines = @(git grep -n -I -w -E --untracked $Words -- @ThrowsOnPurpose)
    # Fewer fails too: the count only means something while it is exact.
    if ($lines.Count -ne $ThrowLinesOnPurpose) { Write-Host "$($lines.Count) exception line(s) in the files named on purpose, where `$ThrowLinesOnPurpose says $ThrowLinesOnPurpose."; exit 1 }
    Write-Host "$ThrowLinesOnPurpose exception line(s) on purpose in $($ThrowsOnPurpose -join ', '), and no more."
}

Push-Location $root
try {
    switch ($Verb) {
        'build' {
            Build-Core
            exit 0
        }
        'install' { Write-Host 'nothing to install'; exit 0 }
        'uninstall' { Write-Host 'nothing to uninstall'; exit 0 }
        'clean' {
            foreach ($path in 'dist', 'build', 'out') {
                if (Test-Path $path) { Remove-Item -Recurse -Force $path }
            }
            exit 0
        }
        'run' {
            # The demo needs matplot++ and <print>, so this runs the test binary and hands it any gtest flag.
            Build-Core
            & (Join-Path $root 'build\GeometryApp_tests.exe') @RunArgs
            exit $LASTEXITCODE
        }
        'publish' {
            # Hands the headers to the lab: the tree section 6 fixes for a $Needs entry.
            if (-not $env:LAB_SHARE) { Write-Host 'LAB_SHARE is not set, so there is nowhere to publish to'; exit 1 }
            $target = Join-Path $env:LAB_SHARE "needs\$Name\v$Version"
            foreach ($folder in $Provides) {
                $destination = Join-Path $target $folder
                New-Item -ItemType Directory -Force $destination | Out-Null
                Copy-Item -Recurse -Force "$folder\*" $destination
            }
            exit 0
        }
        'info' {
            Write-Host "Name    : $Name"
            Write-Host "Code    : $Code"
            Write-Host "Version : $Version"
            Write-Host "Summary : $Summary"
            Write-Host "Provides: $($Provides -join ', ')"
            exit 0
        }
        'test' {
            # The git hooks first (section 10), so a fresh clone has them too.
            git -C $root config core.hooksPath .claude/hooks/git
            if ($LASTEXITCODE -ne 0) { exit 1 }
            Build-Core
            # --no-tests=error: ctest alone says "No tests were found" and exits 0.
            ctest --test-dir build --output-on-failure --no-tests=error
            if ($LASTEXITCODE -ne 0) { exit 1 }
            # -p build after the build. The headers come in through these units and .clang-tidy's filter.
            clang-tidy -p build --quiet (git ls-files 'src/convex_hull.cpp' 'tests/*.cpp')
            if ($LASTEXITCODE -ne 0) { exit 1 }
            # In place, never --dry-run: the gate runs git diff --exit-code after this verb.
            clang-format -i (git ls-files 'include/*.hpp' 'src/*.cpp' 'tests/*.cpp')
            if ($LASTEXITCODE -ne 0) { exit 1 }
            Assert-NoWordInCore 'lock' 'mutex|timed_mutex|recursive_mutex|recursive_timed_mutex|shared_mutex|shared_timed_mutex|lock_guard|unique_lock|shared_lock|scoped_lock|condition_variable|condition_variable_any|counting_semaphore|binary_semaphore|latch|barrier|call_once|once_flag|CRITICAL_SECTION|pthread_mutex_t|pthread_rwlock_t|pthread_spinlock_t'
            Assert-NoWordInCore 'exception' 'throw|try|catch' $ThrowsOnPurpose
            Assert-ThrowsOnPurposeDidNotGrow 'throw|try|catch'
            exit 0
        }
    }
}
finally {
    Pop-Location
}
