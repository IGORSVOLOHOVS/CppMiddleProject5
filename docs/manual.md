# Manual

## The library

Shapes are plain values, and `Shape` is the variant of them all:

```cpp
#include "convex_hull.hpp"
#include "queries.hpp"

using namespace geometry;

Shape circle = Circle{{10, 0}, 5};
double distance = queries::DistanceToPoint(circle, Point2D{4, 0});

std::vector<Point2D> points = {{0, 0}, {4, 0}, {4, 4}, {0, 4}, {2, 2}};
auto hull = convex_hull::GrahamScan(points);
if (!hull) { /* hull.error() is a GeometryError */ }
```

`std::format` prints every shape: `std::format("{}", shape)`.

`intersections::GetIntersectPoint(first, second)` answers an optional point. It throws `std::logic_error` for a pair of types it does not support, so the demo catches that and goes on.

## The tests

`.\recipe.ps1 run` builds and runs the test binary, and hands it any further arguments:

| command | what it does |
| --- | --- |
| `.\recipe.ps1 run` | every test case |
| `.\recipe.ps1 run --gtest_list_tests` | names the test cases |
| `.\recipe.ps1 run --gtest_filter=QueriesTest.*` | runs one suite |

## The demo

Built as `docs/install.md` says, `GeometryApp` generates 15 random shapes, prints their intersections, distances and bounding box collisions, and draws them, their convex hull and a Delaunay triangulation in gnuplot windows. No gate builds or checks it yet.
