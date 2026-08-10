#Requires -Version 5.1
<#
.SYNOPSIS
    Единственная точка входа для нативной сборки проекта под Windows (MSVC).

.DESCRIPTION
    Скрипт делает ровно то, что в Linux делает dev-контейнер, но без Docker:
    вносит окружение MSVC в текущий процесс, подтягивает зависимости через
    Conan, конфигурирует CMake по пресету из CMakePresets.json, собирает и
    прогоняет тесты.

    Почему окружение MSVC вносится, а не запускается "cmd /c vcvars && cmake":
    генератор Ninja вызывает cl.exe десятки раз, и все вызовы должны видеть
    одни и те же INCLUDE/LIB/PATH. Один раз импортировать переменные в процесс
    надёжнее, чем оборачивать каждую команду в cmd.

    Почему Conan вызывается условно: проверка наличия conanfile оставлена,
    чтобы скрипт остался общим для всех репозиториев спринтов — в некоторых
    из них conanfile нет, и безусловный `conan install` упал бы на пустом месте.

    Внимание: matplot++ (подтягивается через CPM прямо из CMakeLists.txt)
    нужен gnuplot только во время ВЫПОЛНЕНИЯ GeometryApp.exe. Для сборки и для
    ctest он не требуется, поэтому скрипт его не проверяет.

.PARAMETER BuildType
    Release (по умолчанию) или Debug. Выбирает соответствующий пресет.

.PARAMETER Clean
    Удалить каталог сборки перед конфигурацией — проверка сборки "с нуля".

.PARAMETER SkipTests
    Не запускать ctest после сборки.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1
.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1 -BuildType Debug -Clean
#>

[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string] $BuildType = 'Release',

    [switch] $Clean,

    [switch] $SkipTests
)

# Молчаливый провал здесь дороже всего: собранный наполовину проект выглядит
# как успешный. Поэтому любая ошибка cmdlet-а — исключение, а код возврата
# каждой нативной команды проверяется явно (см. Invoke-CheckedNativeCommand).
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepositoryRoot = Split-Path -Parent $PSScriptRoot

function Write-Step {
    param([string] $Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Stop-WithClearMessage {
    param([string] $Message)
    Write-Host ""
    Write-Host "BUILD FAILED: $Message" -ForegroundColor Red
    exit 1
}

function Invoke-CheckedNativeCommand {
    <#
        Нативные .exe не бросают исключений — они лишь выставляют $LASTEXITCODE,
        а $ErrorActionPreference на них не действует. Без этой обёртки скрипт
        радостно дошёл бы до конца после провалившегося cmake.
    #>
    param(
        [Parameter(Mandatory = $true)][string]   $Executable,
        [Parameter(Mandatory = $true)][string[]] $Arguments,
        [Parameter(Mandatory = $true)][string]   $FailureMessage
    )

    Write-Host "    $Executable $($Arguments -join ' ')" -ForegroundColor DarkGray
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        Stop-WithClearMessage "$FailureMessage (exit code $LASTEXITCODE)"
    }
}

function Find-VcVarsBatchFile {
    <#
        Порядок поиска: сначала vswhere (официальный способ, переживает
        обновления и Build Tools/Professional-редакции), потом жёстко заданные
        пути — на случай, где vswhere не установлен.
    #>
    $vsWherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vsWherePath) {
        $installationPath = & $vsWherePath -latest -products * `
            -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
            -property installationPath
        if ($LASTEXITCODE -eq 0 -and $installationPath) {
            $candidate = Join-Path ($installationPath | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }

    $fallbackCandidates = @(
        'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat',
        'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
    )
    foreach ($candidate in $fallbackCandidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }

    return $null
}

function Enter-MsvcEnvironment {
    <#
        vcvars64.bat правит окружение только внутри своего cmd. Чтобы правки
        достались всем последующим вызовам cmake/ninja/cl, запускаем bat,
        печатаем `set` и переносим результат в текущий процесс PowerShell.
    #>
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Host "    cl.exe уже на PATH — окружение MSVC не трогаем."
        return
    }

    $vcVarsBatchFile = Find-VcVarsBatchFile
    if (-not $vcVarsBatchFile) {
        Stop-WithClearMessage @'
Не найден vcvars64.bat. Нужен Visual Studio 2022 (или Build Tools) с
компонентом "MSVC v143 - VS 2022 C++ x64/x86 build tools".
Проверенный путь по умолчанию:
  C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
'@
    }

    Write-Host "    vcvars64: $vcVarsBatchFile"

    $environmentDump = & cmd.exe /c "call `"$vcVarsBatchFile`" >nul 2>&1 && set"
    if ($LASTEXITCODE -ne 0 -or -not $environmentDump) {
        Stop-WithClearMessage "Не удалось выполнить $vcVarsBatchFile (exit code $LASTEXITCODE). Установка Visual Studio повреждена?"
    }

    foreach ($line in $environmentDump) {
        # Значения PATH сами содержат '=', поэтому режем только по первому.
        $separatorIndex = $line.IndexOf('=')
        if ($separatorIndex -lt 1) { continue }
        $name = $line.Substring(0, $separatorIndex)
        $value = $line.Substring($separatorIndex + 1)
        Set-Item -Path "Env:$name" -Value $value
    }

    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        Stop-WithClearMessage "Окружение MSVC импортировано, но cl.exe всё равно не найден на PATH."
    }
    Write-Host "    cl.exe: $((Get-Command cl.exe).Source)"
}

function Assert-ToolAvailable {
    param(
        [Parameter(Mandatory = $true)][string] $ToolName,
        [Parameter(Mandatory = $true)][string] $HowToInstall
    )
    $command = Get-Command $ToolName -ErrorAction SilentlyContinue
    if (-not $command) {
        Stop-WithClearMessage "Не найден $ToolName. $HowToInstall"
    }
    Write-Host "    $ToolName : $($command.Source)"
}

# --------------------------------------------------------------------------
# 1. Окружение и инструменты
# --------------------------------------------------------------------------
# Сначала MSVC, потом проверка cmake/ninja: у многих они приезжают вместе с
# Visual Studio ("C++ CMake tools for Windows") и появляются на PATH только
# после vcvars. Проверять до этого — значит отказывать рабочей машине.
Write-Step "Вхожу в окружение MSVC"
Enter-MsvcEnvironment

Write-Step "Проверяю инструменты"
Assert-ToolAvailable -ToolName 'cmake' -HowToInstall 'Поставьте CMake >= 3.30 и добавьте его в PATH: https://cmake.org/download/'
Assert-ToolAvailable -ToolName 'ninja' -HowToInstall 'Поставьте Ninja и добавьте его в PATH (или доставьте компонент VS "C++ CMake tools for Windows").'
Assert-ToolAvailable -ToolName 'ctest' -HowToInstall 'ctest ставится вместе с CMake — проверьте, что каталог CMake\bin целиком в PATH.'
Assert-ToolAvailable -ToolName 'git'   -HowToInstall 'CMakeLists.txt тянет CPM и matplot++ прямо из GitHub, без git это невозможно: https://git-scm.com/download/win'

$presetName = if ($BuildType -eq 'Debug') { 'windows-msvc-debug' } else { 'windows-msvc-release' }
$buildDirectory = if ($BuildType -eq 'Debug') {
    Join-Path $RepositoryRoot 'build\windows-debug'
} else {
    Join-Path $RepositoryRoot 'build\windows'
}
# Пути должны совпадать с "toolchainFile" соответствующего пресета в CMakePresets.json.
$conanOutputFolder = Join-Path $buildDirectory 'conan'
$conanToolchainFile = Join-Path $conanOutputFolder 'build\generators\conan_toolchain.cmake'

# --------------------------------------------------------------------------
# 2. Очистка — строго до Conan
# --------------------------------------------------------------------------
# Каталог с артефактами Conan лежит ВНУТРИ каталога сборки, поэтому чистить
# после `conan install` значило бы удалить только что сгенерированный
# conan_toolchain.cmake и получить непонятную ошибку на этапе конфигурации.
if ($Clean) {
    Write-Step "Чищу каталог сборки"
    if (Test-Path -LiteralPath $buildDirectory) {
        Write-Host "    $buildDirectory"
        Remove-Item -LiteralPath $buildDirectory -Recurse -Force
    } else {
        Write-Host "    $buildDirectory — уже пусто"
    }

    # CMakeUserPresets.json генерирует Conan, и он ссылается include-ом на файл
    # внутри удалённого каталога. Оставить его — значит получить отказ cmake
    # ещё до чтения нашего пресета. Conan пересоздаст его следующим шагом.
    $conanUserPresets = Join-Path $RepositoryRoot 'CMakeUserPresets.json'
    if (Test-Path -LiteralPath $conanUserPresets) {
        Write-Host "    $conanUserPresets"
        Remove-Item -LiteralPath $conanUserPresets -Force
    }
}

# --------------------------------------------------------------------------
# 3. Зависимости через Conan — только если репозиторий действительно его использует
# --------------------------------------------------------------------------
$conanRecipeFiles = @('conanfile.py', 'conanfile.txt') |
    ForEach-Object { Join-Path $RepositoryRoot $_ } |
    Where-Object { Test-Path -LiteralPath $_ }

if ($conanRecipeFiles) {
    Write-Step "Ставлю зависимости через Conan"
    Assert-ToolAvailable -ToolName 'conan' -HowToInstall 'Репозиторий содержит conanfile. Поставьте Conan: pip install conan'

    # Профиль по умолчанию создаётся один раз на машину; без него conan install
    # падает с "profile 'default' not found", что новичка ставит в тупик.
    # $ErrorActionPreference временно ослаблен: в Windows PowerShell 5.1
    # перенаправление stderr нативной программы порождает NativeCommandError,
    # и при 'Stop' безобидная проверка превратилась бы в аварию.
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & conan profile path default 2>&1 | Out-Null
    $conanDefaultProfileExitCode = $LASTEXITCODE
    $ErrorActionPreference = $previousErrorActionPreference

    if ($conanDefaultProfileExitCode -ne 0) {
        Write-Host "    Профиля default нет — создаю (conan profile detect)."
        Invoke-CheckedNativeCommand -Executable 'conan' `
            -Arguments @('profile', 'detect', '--force') `
            -FailureMessage 'conan profile detect не сработал'
    }

    # Про аргументы:
    #   compiler.cppstd=23 — потолок, который Conan знает для msvc (см.
    #     settings.yml). Сам проект собирается как C++26: стандарт задаётся в
    #     CMakeLists.txt и перекрывает значение из conan_toolchain.cmake.
    #     На GTest это не влияет — он C++14-совместимый.
    #   generator=Ninja — иначе CMakeToolchain считает, что генератор Visual
    #     Studio, и прописывает в conan_toolchain.cmake CMAKE_GENERATOR_PLATFORM
    #     и CMAKE_GENERATOR_TOOLSET, которых Ninja не понимает.
    Invoke-CheckedNativeCommand -Executable 'conan' `
        -Arguments @('install', $RepositoryRoot,
                     '--output-folder', $conanOutputFolder,
                     '--build', 'missing',
                     '--settings', "build_type=$BuildType",
                     '--settings', 'compiler.cppstd=23',
                     '--conf', 'tools.cmake.cmaketoolchain:generator=Ninja') `
        -FailureMessage 'conan install не сработал'

    if (-not (Test-Path -LiteralPath $conanToolchainFile)) {
        Stop-WithClearMessage @"
conan install отработал, но $conanToolchainFile не появился.
Скорее всего в conanfile.py изменился layout(); поправьте пути
"toolchainFile" в CMakePresets.json и `$conanToolchainFile в этом скрипте.
"@
    }
} else {
    Write-Step "Conan пропущен"
    Write-Host "    В репозитории нет conanfile.py/conanfile.txt — внешних зависимостей нет."
}

# --------------------------------------------------------------------------
# 4. Конфигурация и сборка
# --------------------------------------------------------------------------
Write-Step "Конфигурирую CMake (пресет $presetName)"
Write-Host "    Первый запуск дольше остальных: CPM скачивает и собирает matplot++."
Push-Location $RepositoryRoot
try {
    Invoke-CheckedNativeCommand -Executable 'cmake' `
        -Arguments @('--preset', $presetName) `
        -FailureMessage "cmake --preset $presetName не сработал"

    Write-Step "Собираю"
    Invoke-CheckedNativeCommand -Executable 'cmake' `
        -Arguments @('--build', '--preset', $presetName) `
        -FailureMessage "cmake --build --preset $presetName не сработал"

    if (-not $SkipTests) {
        Write-Step "Запускаю тесты (ctest)"
        Invoke-CheckedNativeCommand -Executable 'ctest' `
            -Arguments @('--preset', $presetName) `
            -FailureMessage 'ctest сообщил о провалившихся тестах'
    }
} finally {
    Pop-Location
}

# --------------------------------------------------------------------------
# 5. Что получилось
# --------------------------------------------------------------------------
# CMakeFiles\ отбрасывается: там лежит CMakeCXXCompilerId.exe — пробник самого
# CMake, а не артефакт проекта. Показывать его как результат сборки нечестно.
# _deps\ — тоже не наши бинарники, а примеры и утилиты matplot++.
$producedExecutables = @(Get-ChildItem -LiteralPath $buildDirectory -Filter '*.exe' -File -Recurse |
    Where-Object { $_.FullName -notmatch '\\CMakeFiles\\' -and $_.FullName -notmatch '\\_deps\\' } |
    Sort-Object FullName)

if (-not $producedExecutables) {
    Stop-WithClearMessage "Сборка завершилась без ошибок, но в $buildDirectory нет ни одного .exe. Это не успех."
}

Write-Step "Готово"
Write-Host "    Каталог сборки: $buildDirectory"
foreach ($executable in $producedExecutables) {
    Write-Host "    $($executable.FullName)" -ForegroundColor Green
}
Write-Host ""
Write-Host "Точка входа проекта — $((Join-Path $buildDirectory 'GeometryApp.exe'))"
Write-Host "Ему нужен gnuplot на PATH: без него окна с графиками не откроются."
Write-Host "  winget install gnuplot   (или http://www.gnuplot.info/)"
