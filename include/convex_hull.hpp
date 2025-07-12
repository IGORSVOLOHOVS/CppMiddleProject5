#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <ranges>
#include <stack>
#include <vector>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {};

GeometryResult<std::vector<Point2D>> GrahamScan(DummyClass points);

}  // namespace geometry::convex_hull