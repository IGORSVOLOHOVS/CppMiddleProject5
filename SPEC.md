# cpp-middle-project5 — Software Requirements Specification

Reverse-engineered from the code at commit `9136b85` on 2026-11-18. It describes implemented behaviour only. Hub: [`docs/README.md`](docs/README.md).

<!-- jira-sync:start -->
Jira: KAN-143

## Tasks

This list is one ticket in Jira project KAN, [KAN-143](https://igorsvolohovs.atlassian.net/browse/KAN-143). Each line is a checkbox there: tick it in Jira and the sync ticks it here; add a checkbox there and it is appended here. Done items are the features described in section 3; open items are what the project declares but the code does not do yet.

- [x] Primitives and tolerance (section 3.1) <!-- spec-primitives-and-tolerance -->
- [x] Shapes (section 3.2) <!-- spec-shapes -->
- [x] Intersections (section 3.3) <!-- spec-intersections -->
- [x] Queries (section 3.4) <!-- spec-queries -->
- [x] Convex hull (section 3.5) <!-- spec-convex-hull -->
- [x] Delaunay triangulation (section 3.6) <!-- spec-delaunay-triangulation -->
- [x] Shape utilities (section 3.7) <!-- spec-shape-utilities -->
- [x] Demo application (section 3.8) <!-- spec-demo-application -->
- [ ] `StackForGrahamScan`, an empty class that nothing uses (declared, not implemented) <!-- spec-stackforgrahamscan-an-empty-class-that-nothing-use -->
- [ ] Shape-to-shape distance for "Any & Point", named in the visitor comment (declared, not implemented) <!-- spec-shape-to-shape-distance-for-any-point-named-in-the -->
- [ ] `GeometryError::Unsupported`, `NoIntersection`, `DegenrateCase`, declared but never returned (declared, not implemented) <!-- spec-geometryerror-unsupported-nointersection-degenrate -->
- [ ] `recipe.ps1 install`, which prints "nothing to install" (declared, not implemented) <!-- spec-recipe-ps1-install-which-prints-nothing-to-install -->

<!-- jira-sync:end -->

## 1. Purpose and scope

**In scope:** the C++23 geometry library in `include/` and `src/convex_hull.cpp`, the `GeometryApp` console demo (`src/main.cpp`, `src/visualization.cpp`), and the tests in `tests/`.

**Out of scope:** the shared lab tooling (`recipe.ps1` hooks and gates, `.claude/`, `.gitea/`, `tools/wiki.py`, `tests/manual/serve.py`, `tests/manual/index.html`). The legacy build routes `conanfile.py`, `CMakePresets.json`, `scripts/build_windows.ps1`, `.devcontainer/` and `.vscode/` are also out of scope; `docs/install.md` says no gate runs them.

## 2. Context

- **Actors:** a C++ developer who includes the headers; a person running the demo. The repository names no other user (`README.md`).
- **External systems:** GoogleTest v1.13.0 for tests. For the demo only: matplot++, CPM.cmake, and gnuplot at runtime (`CMakeLists.txt:28-57`, `docs/install.md`).
- **Runtime environment:** any C++23 compiler for the library. The gated build uses mingw g++ 13+ with Ninja on Windows (`recipe.ps1:51-62`, `docs/install.md`). The demo needs g++ 14+ for `<print>` (`docs/install.md`).

## 3. Functional requirements

### 3.1 Primitives and tolerance

- **FR-1** The system shall compare doubles with tolerance `EPSILON`. It defaults to `1e-10` and can be overridden with `-DGEOMETRY_EPSILON`. Source: `include/geometry.hpp:42-60`. Verification: no test.
- **FR-2** The system shall provide `Point2D` addition, subtraction, scaling, dot and cross product, length, distance and normalisation. Normalising a zero vector gives `(0, 0)`. Source: `include/geometry.hpp:66-94`. Verification: `GeometryTest.Point2DOperations` (addition, subtraction and distance only).
- **FR-3** The system shall treat two `BoundingBox`es as overlapping unless they are separated by more than `EPSILON` on an axis. Touching boxes therefore count as overlapping. Source: `include/geometry.hpp:125-131`. Verification: `QueriesTest.BoundingBoxesOverlap` (the touching case is not tested).
- **FR-4** The system shall return `GeometryError::InvalidInput` from `BoundingBox::Center()` when the width or the height is zero. Source: `include/geometry.hpp:136-153`. Verification: no test.

### 3.2 Shapes

- **FR-5** The system shall represent `Line`, `Triangle`, `Rectangle`, `RegularPolygon`, `Circle` and `Polygon` as alternatives of `Shape`. Each has `BoundBox()`, `Center()` and `Vertices()`. Source: `include/geometry.hpp:39,156-450`. Verification: `GeometryTest.*`.
- **FR-6** The system shall use the absolute value of a negative `Rectangle` width or height, and of a negative `Circle`/`RegularPolygon` radius, in area, size and bounding box. Source: `include/geometry.hpp:275-291,324-329,395-397`. Verification: no test.
- **FR-7** The system shall generate `Circle` vertices as `N` points evenly spaced in angle, default 30. A `RegularPolygon` has `sides` such vertices on its circumcircle. Source: `include/geometry.hpp:333-351,395-397`. Verification: `GeometryTest.CircleProperties`.
- **FR-8** The system shall compute a `Polygon`'s bounding box once, at construction, and return `(0, 0)` as its centre when that box has zero width or height. Source: `include/geometry.hpp:409-433`. Verification: `GeometryTest.PolygonProperties` (non-degenerate case only).
- **FR-9** The system shall produce an outline for each shape (closed for every shape except `Line`, which gives `N` points along the segment) as `Lines2DDyn` (x and y arrays) through `Lines(N)`. Source: `include/geometry.hpp:177-240,268,315,353-367,399-404,445`. Verification: no test.
- **FR-10** The system shall format each shape, `Point2D` and `Shape` with `std::format`, using two decimal places for coordinates. Source: `include/geometry.hpp:453-579`. Verification: no test.
- **FR-11** The system shall format `std::vector<Point2D>` as `[p, p, ...]`, or one point per line with `{:new_line}`. Any other specifier throws `std::format_error`. Source: `include/geometry.hpp:462-503`. Verification: no test.

### 3.3 Intersections

- **FR-12** The system shall return the intersection point of two `Line` segments. Parallel segments (|cross| < 1e-9) and segments that do not meet give no point. Source: `include/intersections.hpp:18-33`. Verification: `IntersectionTest.LineLineIntersection`.
- **FR-13** The system shall return the 0, 1 or 2 points where a `Line` segment meets a `Circle`, for the argument order (Line, Circle). Source: `include/intersections.hpp:35-69`. Verification: `IntersectionTest.LineCircleIntersection`.
- **FR-14** The system shall return the intersection of two circles: 0, 1 or 2 points, or `InfiniteIntersections` for coincident circles. Source: `include/intersections.hpp:71-96`. Verification: `IntersectionTest.CircleCircleIntersection`.
- **FR-15** The system shall throw `std::logic_error` from `GetIntersectPoint` for every other pair of types, (Circle, Line) included. Source: `include/intersections.hpp:98-100`. Verification: `IntersectionTest.UnsupportedIntersection`.
- **FR-16** `GetIntersectPoint` shall return the first point of a pair, and `std::nullopt` for no intersection or infinite intersections. Source: `include/intersections.hpp:103-119`. Verification: `IntersectionTest.*`.

### 3.4 Queries

- **FR-17** The system shall return the distance from a point to any `Shape`. The distance is 0 inside or on a closed shape, `+inf` for an empty `Polygon`, and a `RegularPolygon` is measured as a `Polygon` of its vertices. Source: `include/queries.hpp:20-100,137-139`. Verification: `QueriesTest.DistanceToPoint`.
- **FR-18** The system shall return a shape-to-shape distance only for (Circle, Circle) and (Line, Line), and `std::nullopt` for other pairs. Source: `include/queries.hpp:112-132,153-155`. Verification: no test.
- **FR-19** The system shall return a shape's bounding box, its height (bounding-box height), and whether two shapes' bounding boxes overlap. Source: `include/queries.hpp:141-151`. Verification: `QueriesTest.GetBoundBox`, `QueriesTest.GetHeight`, `QueriesTest.BoundingBoxesOverlap`.

### 3.5 Convex hull

- **FR-20** The system shall return the convex hull of a point set by Graham scan. Collinear points on the hull are dropped. Source: `src/convex_hull.cpp:13-48`. Verification: `ConvexHullTest.GrahamScanSquare`.
- **FR-21** The system shall return `GeometryError::InsufficientPoints` for fewer than 3 input points. Source: `src/convex_hull.cpp:14-16`. Verification: `ConvexHullTest.GrahamScanInsufficientPoints`.

### 3.6 Delaunay triangulation

- **FR-22** The system shall triangulate a point set by incremental insertion into a super-triangle (scale factor 20), using the circumcircle test, then remove the triangles that touch the super-triangle. Source: `include/triangulation.hpp:103-166`. Verification: `TriangulationTest.DelaunayTriangulationBasic`.
- **FR-23** The system shall return `GeometryError::InsufficientPoints` for fewer than 3 points. Source: `include/triangulation.hpp:106-108`. Verification: no test.

### 3.7 Shape utilities

- **FR-24** `ShapeGenerator` shall produce random shapes from an `mt19937` seeded with the constant 20. Coordinates and sizes come from constructor ranges; polygon side counts are fixed at 3..12. It never generates a `Polygon` (type index 0..4). Source: `include/shape_utils.hpp:12-66`. Verification: no test.
- **FR-25** The system shall return every unordered pair of shapes whose bounding boxes overlap. Source: `include/shape_utils.hpp:68-78`. Verification: `ShapeUtilsTest.FindAllCollisions`.
- **FR-26** The system shall return the index of the shape with the greatest height, or `std::nullopt` for an empty list. Source: `include/shape_utils.hpp:80-89`. Verification: `ShapeUtilsTest.FindHighestShape`.

### 3.8 Demo application

- **FR-27** `GeometryApp` shall generate 15 shapes and print them, the intersections of shape 0, distances from (10, 10) to the first 5 shapes, bounding-box collisions, the highest shape, one sample shape distance, the shapes above y = 50 and the min/max heights. Source: `src/main.cpp:20-106`. Verification: no test.
- **FR-28** `GeometryApp` shall draw the shapes, then the shapes with their convex hull, then the Delaunay triangulation of 7 fixed points, in gnuplot windows through matplot++. Source: `src/main.cpp:108-132`, `src/visualization.cpp:14-89`. Verification: no test.

### 3.9 Declared but not implemented

| Item | Source |
| --- | --- |
| `StackForGrahamScan`, an empty class that nothing uses | `include/convex_hull.hpp:14` |
| Shape-to-shape distance for "Any & Point", named in the visitor comment | `include/queries.hpp:102-111` |
| `GeometryError::Unsupported`, `NoIntersection`, `DegenrateCase`, declared but never returned | `include/geometry.hpp:25-31` |
| `recipe.ps1 install`, which prints "nothing to install" | `recipe.ps1:93`, `docs/install.md` |

## 4. Interfaces

### 4.1 C++ API (namespace `geometry`)

| Function | Signature (abridged) | Errors | Source |
| --- | --- | --- | --- |
| `intersections::GetIntersectPoint` | `(const Shape&, const Shape&) -> std::optional<Point2D>` | throws `std::logic_error` | `include/intersections.hpp:103` |
| `queries::DistanceToPoint` | `(const Shape&, const Point2D&) -> double` | — | `include/queries.hpp:137` |
| `queries::DistanceBetweenShapes` | `(const Shape&, const Shape&) -> std::optional<double>` | — | `include/queries.hpp:153` |
| `queries::GetBoundBox` / `GetHeight` / `BoundingBoxesOverlap` | `(const Shape&...)` | — | `include/queries.hpp:141-151` |
| `convex_hull::GrahamScan` | `(std::span<const Point2D>) -> GeometryResult<std::vector<Point2D>>` | `InsufficientPoints` | `include/convex_hull.hpp:16` |
| `triangulation::DelaunayTriangulation` | `(std::span<const Point2D>) -> GeometryResult<std::vector<DelaunayTriangle>>` | `InsufficientPoints` | `include/triangulation.hpp:103` |
| `utils::FindAllCollisions` | `(std::span<const Shape>) -> std::vector<std::pair<Shape, Shape>>` | — | `include/shape_utils.hpp:68` |
| `utils::FindHighestShape` | `(std::span<const Shape>) -> std::optional<size_t>` | — | `include/shape_utils.hpp:80` |
| `visualization::Draw` | `(std::span<Shape>)`, `(std::span<const DelaunayTriangle>)` | — | `include/visualization.hpp:9-11` |

### 4.2 Command line

| Command | Arguments | Source |
| --- | --- | --- |
| `GeometryApp` | none | `src/main.cpp:91` |
| `GeometryApp_tests` (or `.\recipe.ps1 run ...`) | standard gtest flags, e.g. `--gtest_filter`, `--gtest_list_tests` | `tests/main.cpp`, `docs/manual.md` |

### 4.3 Text formats

| Type | Format | Source |
| --- | --- | --- |
| `Point2D` | `(x.xx, y.yy)` | `include/geometry.hpp:459` |
| `Circle` | `Circle(center=(..), r=..)` | `include/geometry.hpp:521` |
| `Polygon` | `Polygon[N points]: [p p ... ]` | `include/geometry.hpp:562-568` |
| `DelaunayTriangle` | `DelaunayTriangle(a, b, c)` | `include/triangulation.hpp:175` |

## 5. Data

- Entities: `Point2D {x, y}`; `BoundingBox {min_x, min_y, max_x, max_y}`; `Line {start, end}`; `Triangle {a, b, c}`; `Rectangle {bottom_left, width, height}`; `Circle {center_p, radius}`; `RegularPolygon {center_p, radius, sides}`; `Polygon` (private `points_` and a cached `bounding_box_`); `DelaunayTriangle {a, b, c}`; `Edge {p1, p2}`, with endpoints ordered at construction. Source: `include/geometry.hpp`, `include/triangulation.hpp:15-91`.
- Storage: none. The library keeps everything in memory and reads or writes no files.
- `tests/manual/tests.json`: `{"version": 1, "tests": [...]}`. Each entry needs `id`, `title`, `expected` and `steps`, and may have `issue`, `blocking` and `needs` (`tests/test_manual_list.py`). It currently holds no entries.

## 6. Non-functional requirements

- **NFR-1** The code is C++23 with no compiler extensions (`CMakeLists.txt:4-6`).
- **NFR-2** The core and tests compile with `-Wall -Wextra -Wpedantic -Werror` on non-MSVC compilers; MSVC gets only `/utf-8 /bigobj` (`CMakeLists.txt:15-21`). clang-tidy runs `clang-analyzer-*`, `bugprone-*`, `performance-*` and `portability-*` with warnings as errors (`.clang-tidy`, `recipe.ps1:135`).
- **NFR-3** The geometry core depends only on the standard library (`README.md`; includes in `include/*.hpp` other than `visualization.hpp`).
- **NFR-4** `include/` and `src/` contain no lock or synchronisation primitive. `throw/try/catch` appear on exactly 4 lines, all in `include/geometry.hpp`, `include/intersections.hpp` and `src/main.cpp` (`recipe.ps1:34-35,141-142`).
- **NFR-5** The code builds with libstdc++ and MSVC STL. `<flat_map>` falls back to `std::map`, and `std::abs`/`variant::visit` are avoided where MSVC lacks them (`include/triangulation.hpp:93-101`, `include/geometry.hpp:48-54`, `src/visualization.cpp:56-58`).
- **NFR-6** The GoogleTest download is pinned by SHA256 (`CMakeLists.txt:51-56`).

## 7. Constraints and dependencies

| Dependency | Version | Source |
| --- | --- | --- |
| CMake | ≥ 3.28 | `CMakeLists.txt:1` |
| g++ (mingw) | ≥ 13 (core), ≥ 14 (demo) | `docs/install.md` |
| GoogleTest | v1.13.0, archive + SHA256 | `CMakeLists.txt:51-56` |
| matplot++ | `master` (unpinned) | `CMakeLists.txt:35-39` |
| CPM.cmake | `origin/master` (unpinned) | `CMakeLists.txt:28-33` |
| gnuplot | unspecified, demo runtime | `docs/install.md` |
| Python + pytest | unspecified, for CTest `manual_list` | `CMakeLists.txt:75-79` |
| Conan (legacy) | `gtest/1.13.0`, `cmake/3.30.0` | `conanfile.py:12-13` |

Licence: MIT, copyright Yandex Practicum (`LICENSE`, `README.md`).

## 8. Acceptance

1. `.\recipe.ps1 test` exits with `$LASTEXITCODE` 0. This covers the build, CTest (`geometry_tests`, `manual_list`), clang-tidy, clang-format with no diff, and the lock and exception searches (`recipe.ps1:126-145`, `CLAUDE.md` rule 3).
2. Every FR whose verification names a test passes under `.\recipe.ps1 run`. The FRs marked "no test" need a new test or a manual check.
3. The demo build command from `docs/install.md` produces `GeometryApp`, which prints the analysis of FR-27 and opens the plots of FR-28. No gate checks this.
4. One requirement concerns the shared standard: the repository conforms to repo-quality-template, lab requirements revision 4.0.0, except as listed in `README.md`.

## 9. Open questions

- Who uses the library besides the course assignment, and is it consumed by another lab project through `publish`? `$Needs` elsewhere is not visible here.
- Should `GetIntersectPoint` support the (Circle, Line) order and the other shape pairs, or is throwing the intended contract? `README.md` calls the throw deliberate.
- Should `DistanceBetweenShapes(line, line)` return 0 for crossing segments? Should circle/circle use `abs(radius)`, as the other paths do?
- Should `Polygon::Center()` report a degenerate box instead of returning `(0, 0)`?
- No accuracy, performance or input-size targets are stated. The triangulation is O(n²) or worse (inference from the per-point scan over all triangles, `include/triangulation.hpp:129-156`).
- Are `CMakePresets.json`, `conanfile.py` and `scripts/build_windows.ps1` to be kept, updated or removed?
- No test confirms the behaviour for collinear or duplicate points in `GrahamScan` and `DelaunayTriangulation`.
