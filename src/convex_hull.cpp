#include "convex_hull.hpp"
#include <algorithm>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

GeometryResult<std::vector<Point2D>> GrahamScan(DummyClass points) {
    return std::unexpected{GeometryError::Unsupported};
}

}  // namespace geometry::convex_hull