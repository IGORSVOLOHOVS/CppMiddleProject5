#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <ranges>
#include <stack>
#include <vector>
#include <span>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {};

GeometryResult<std::vector<Point2D>> GrahamScan(std::span<const Point2D> points);

}  // namespace geometry::convex_hull