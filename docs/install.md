# Install

The geometry needs no installing: put `include` on your include path, compile `src/convex_hull.cpp` with your project, and build with C++23.

To build and test it here:

1. Install mingw g++ 13 or newer, CMake 3.28 or newer, Ninja, and LLVM for `clang-tidy` and `clang-format`.
2. Clone this repository.
3. Run `.\recipe.ps1 test`. The first run downloads GoogleTest, whose archive `CMakeLists.txt` pins by hash.

`recipe.ps1` looks for g++ in `C:\ProgramData\mingw64\mingw64\bin` and for LLVM in `C:\Program Files\LLVM\bin`, and adds those folders to its own PATH.

`.\recipe.ps1 build` compiles the core and its tests into `build\`. `.\recipe.ps1 install` does nothing: there is no installer yet.

## The demo

`GeometryApp` prints an analysis of random shapes and draws them through matplot++, which needs gnuplot on PATH. It uses `<print>`, so it needs g++ 14 or newer, and `recipe.ps1` does not build it. With those on the machine:

```powershell
cmake -S . -B build-demo -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-demo
```

`GEOMETRY_APP_DEMO` is on by default, so a plain configure fetches matplot++ from its `master` branch. No gate builds this path.

`conanfile.py`, `CMakePresets.json`, `scripts/build_windows.ps1`, `.devcontainer` and `.vscode` are what the course and the old GitHub route left. No gate runs them.
