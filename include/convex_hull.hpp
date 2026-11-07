#ifndef GEOMETRY_CONVEX_HULL_HPP
#define GEOMETRY_CONVEX_HULL_HPP
#include "geometry.hpp"
#include <algorithm>
#include <ranges>
#include <span>
#include <stack>
#include <vector>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {};

GeometryResult<std::vector<Point2D>> GrahamScan(std::span<const Point2D> points);

}  // namespace geometry::convex_hull

#endif  // GEOMETRY_CONVEX_HULL_HPP
