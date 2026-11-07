# cpp-middle-project5

Two-dimensional geometry in modern C++23: shapes, intersections, distances, a convex hull and a Delaunay triangulation. It began as the sprint 5 project of the Yandex Practicum "Middle C++ developer" course. The geometry is headers plus one source file and depends on nothing but the standard library; only the console demo needs matplot++ and gnuplot.

## What it does

- **Shapes**: `Line`, `Triangle`, `Rectangle`, `RegularPolygon`, `Circle` and `Polygon`, held in one `Shape` variant, each with `std::formatter` support.
- **Intersections**: `intersections::GetIntersectPoint` between two shapes.
- **Queries**: `queries::DistanceToPoint`, `GetHeight` and `BoundingBoxesOverlap`.
- **Convex hull**: `convex_hull::GrahamScan`.
- **Triangulation**: `triangulation::DelaunayTriangulation`.
- **Utilities**: `utils::ShapeGenerator`, `FindAllCollisions` and `FindHighestShape`.

Most errors travel as `std::expected<T, GeometryError>`. Two places throw on purpose, and `.\recipe.ps1 test` pins them: `GetIntersectPoint` throws `std::logic_error` for a pair of shapes it does not support, and the formatter of `std::vector<Point2D>` throws `std::format_error` for a bad format string, as `std::format` asks.

## Layout

```text
include/  the geometry, as headers
src/      convex_hull.cpp, and the demo: main.cpp and visualization.cpp
tests/    GoogleTest cases
```

## Build and test

`.\recipe.ps1 test` builds the core, runs `ctest`, `clang-tidy` and `clang-format`, and searches `include` and `src` for locks and for `throw`, `try` and `catch`. It needs mingw g++, cmake, ninja and LLVM, and neither matplot++ nor gnuplot: it configures with `-DGEOMETRY_APP_DEMO=OFF`. `docs/install.md` has the details, and `docs/manual.md` covers the library and the demo.

MIT licensed: `LICENSE` is the course's, copyright Yandex Practicum.
