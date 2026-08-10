# cpp-middle-project-sprint-5 <!-- omit in toc -->

- [Начало работы](#начало-работы)
- [Сборка проекта и запуск тестов](#сборка-проекта-и-запуск-тестов)
  - [Команды для сборки проекта](#команды-для-сборки-проекта)
  - [Команды для запуска приложения](#команды-для-запуска-приложения)
  - [Команда для запуска тестов](#команда-для-запуска-тестов)
  - [Команда для запуска clang-format - Обязательное требование перед сдачей работы на ревью](#команда-для-запуска-clang-format---обязательное-требование-перед-сдачей-работы-на-ревью)
  - [Команды для запуска отладчика](#команды-для-запуска-отладчика)
- [Сборка под Windows](#сборка-под-windows)
  - [Что нужно установить](#что-нужно-установить)
  - [Как собрать](#как-собрать)
  - [Пресеты CMake](#пресеты-cmake)
- [Дополнительно](#дополнительно)


Шаблон репозитория для практического задания 5-го спринта «Мидл разработчик С++»

## Начало работы

1. Нажмите зелёную кнопку `Use this template`, затем `Create a new repository`.
2. Назовите свой репозиторий.
3. Склонируйте созданный репозиторий командой `git clone your-repository-name`.
4. Создайте новую ветку командой `git switch -c development`.
5. Откройте проект в `Visual Studio Code`.
6. Нажмите `F1` и откройте проект в dev-контейнере командой `Dev Containers: Reopen in Container`.

## Сборка проекта и запуск тестов

Данный репозиторий использует три инструмента:

- **Conan** — свободный менеджер пакетов для C и C++ с открытым исходным кодом (MIT). Позволяет настраивать процесс сборки программ, скачивать и устанавливать сторонние зависимости и необходимые инструменты. Подробнее о Conan:
  - https://habr.com/ru/articles/884464
  - https://docs.conan.io/2.0/tutorial/consuming_packages/build_simple_cmake_project.html
  - https://docs.conan.io/2.0/tutorial/consuming_packages/the_flexibility_of_conanfile_py.html

- **CPM.cmake** - CMake dependency manager. Поскольку не все пакеты доступны в `Conan`, в качестве альтернативы удобно воспользоваться `CPM.cmake`
  - https://github.com/cpm-cmake/CPM.cmake

- **cmake** — генератор систем сборки для C и C++. Позволяет создавать проекты, которые могут компилироваться на различных платформах и с различными компиляторами. Подробнее о cmake:
  - https://dzen.ru/a/ZzZGUm-4o0u-IQlb
  - https://neerc.ifmo.ru/wiki/index.php?title=CMake_Tutorial
  - https://cmake.org/cmake/help/book/mastering-cmake/cmake/Help/guide/tutorial/index.html

- **VS Code Dev Docker container** - Docker контейнер, который содержит полностью настроенное окружение для выполнение задания. Подробнее об этой функциональности:
  - https://habr.com/ru/articles/822707/ - "Почти все, что вы хотели бы знать про Docker"
  - https://code.visualstudio.com/docs/devcontainers/containers - официальная документация VS Code
  - https://www.youtube.com/watch?v=p9L7YFqHGk4 - "Docker container for VS Code"
  - https://www.youtube.com/watch?v=pg19Z8LL06w&t=174s&pp=ygUPRG9ja2VyY29udGFpbmVy - "Docker in 1 hour"

### Команды для сборки проекта

Используйте `F5` для выполнения следующих шагов:
- Создание папки `build`
- Вызов `conan` команд для установки требуемых библиотек и запуска процесса сборки
- Запуска `lldb` отладчика

### Команды для запуска приложения

```bash
cd build
./GeometryApp
```

### Команда для запуска тестов

```bash
cd build
./GeometryApp_tests
```

### Команда для запуска clang-format - Обязательное требование перед сдачей работы на ревью

В данном репозитории настроен автоматический запуск clang-format (файл конфигурации - `.vscode/settings.json`) при сохранении любого файла с кодом
Убедитесь, что эта функциональность работает:
- Добавьте несколько пустых линий в любой файл
- Сохраните его
- Если пустые линии были удалены - всё работает!
  - Если нет - убедитесь, что `clangd` работает (при открытии файла с кодом в самом низу `VS Code` на голубой полоске должно быть написано `clangd: idle`), для этого:
    - вам необходимо нажать `F1` и выполнить команду `clangd: Download language server`
    - вам необходимо нажать `F1` и выполнить команду `clangd: Restart language server`
    - вам необходимо нажать `F1` и выполнить команду `Developer: Reload Window`

### Команды для запуска отладчика

В `Visual Studio Code` настройки параметров для запуска отладчика находятся в `.vscode/launch.json` файле. Поскольку в этом файле уже есть одна конфигурация `Launch GeometryApp` для запуска приложения, которое вычисляет контрольную сумму файла, то для запуска отладчика достаточно нажать `F5` или открыть окно `Run and Debug` комбинацией клавиш `Ctrl+Shift+D`.

## Сборка под Windows

Dev-контейнер остаётся основным способом сборки, но проект собирается и нативно —
компилятором MSVC, без Docker и без WSL. Точка входа одна: `scripts\build_windows.ps1`.

### Что нужно установить

- **Visual Studio 2022** (Community достаточно) с рабочей нагрузкой
  «Разработка классических приложений на C++». Нужен компилятор MSVC v143;
  проверялось на `cl.exe` 19.44. Проект опирается на `std::expected`,
  `std::println`, `std::views::cartesian_product` и `std::views::enumerate`,
  так что более ранние версии, скорее всего, не соберут его.
- **CMake ≥ 3.30** и **Ninja** на `PATH`. Оба приезжают вместе с компонентом
  Visual Studio «C++ CMake tools for Windows», если ставить их отдельно не хочется.
- **Conan 2** (`pip install conan`) — из него приезжает GTest. Профиль `default`
  скрипт создаёт сам, если его ещё нет.
- **Git** на `PATH`: `CMakeLists.txt` тянет CPM и matplot++ прямо из GitHub.
- **gnuplot** — только для запуска `GeometryApp.exe`. matplot++ рисует через него,
  и без gnuplot окна с графиками не откроются. Для сборки и для `ctest` он не нужен.

Отдельно запускать «Developer Command Prompt» не требуется — скрипт сам находит
`vcvars64.bat` (через `vswhere`, с запасными путями) и вносит окружение MSVC в
свой процесс.

### Как собрать

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1
```

Скрипт вызывает Conan, настраивает CMake, собирает всё и прогоняет `ctest`, а в
конце печатает пути к получившимся `.exe`. Результат лежит в `build\windows`:
приложение — `build\windows\GeometryApp.exe`, тесты — `build\windows\GeometryApp_tests.exe`.
Первый запуск заметно дольше остальных: CPM выкачивает и собирает matplot++.

Полезные ключи:

| Ключ | Зачем |
| --- | --- |
| `-BuildType Debug` | сборка с отладочной информацией в `build\windows-debug` |
| `-Clean` | удалить каталог сборки и собрать с нуля |
| `-SkipTests` | не запускать `ctest` |

Под MSVC у стандарта есть одна оговорка. В Linux проект собирается как C++26,
а у CMake нет флага C++26 для MSVC: `Modules/Compiler/MSVC-CXX.cmake` знает
стандарты только до CXX23. Поэтому в Windows-ветке `CMakeLists.txt` запрашивает
23 — для MSVC это разворачивается ровно в `/std:c++latest`, то есть в самый
свежий режим, который умеет `cl.exe`.

### Пресеты CMake

`CMakePresets.json` описывает обе платформы, поэтому IDE (Visual Studio, VS Code,
CLion) подхватывает конфигурацию сама:

| Пресет | Платформа | Генератор | Каталог сборки |
| --- | --- | --- | --- |
| `linux-default` | Linux / dev-контейнер | Unix Makefiles | `build/` |
| `windows-msvc-release` | Windows, MSVC x64 | Ninja | `build/windows/` |
| `windows-msvc-debug` | Windows, MSVC x64 | Ninja | `build/windows-debug/` |

Каждый пресет ссылается на `conan_toolchain.cmake`, поэтому перед запуском пресета
руками нужно сначала выполнить `conan install` — именно это и делает
`scripts\build_windows.ps1`. Windows-пресеты вдобавок рассчитаны на окружение MSVC:
запускать их вручную надо из «Developer PowerShell for VS 2022», иначе CMake не
найдёт `cl.exe`.

## Дополнительно

- Автодополнение `Ctrl + Space`. Для настройки автодополнения вам необходимо нажать `F1` и выполнить команду `clangd: Download language server`. VS Code сам предложит установить подходящую версию `clangd` (всплывашка в правом нижнем углу). После завершения установки потребуется перезагрузить окно (кнопка перезапуска будет находиться также справа снизу или нажать `F1` и выполнить команду `Developer: Reload Window`)

Если всё сделано правильно - после успешной сборки проекта вы сможете использовать автодополнение
