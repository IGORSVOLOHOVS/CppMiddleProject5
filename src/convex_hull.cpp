#include "convex_hull.hpp"
#include "geometry.hpp"
#include <algorithm>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

GeometryResult<std::vector<Point2D>> GrahamScan(std::span<const Point2D> points) {
    if (points.size() < 3) {
        return std::unexpected{GeometryError::InsufficientPoints};
    }

    std::vector<Point2D> local_points(points.begin(), points.end());

    auto p0_it = std::ranges::min_element(local_points, {}, 
        [](const Point2D& p) { return std::tie(p.y, p.x); });

    std::iter_swap(local_points.begin(), p0_it);
    const Point2D p0 = local_points[0];

    auto points_to_sort = std::ranges::subrange(local_points.begin() + 1, local_points.end());
    std::ranges::sort(points_to_sort, [&](const Point2D& a, const Point2D& b) {
        double cross_prod = CrossProduct(a, p0, b);
        if (is_zero(cross_prod)) {
            return p0.DistanceTo(a) < p0.DistanceTo(b);
        }
        return is_greater(cross_prod, 0.0);
    });
    
    std::vector<Point2D> hull;
    hull.reserve(local_points.size());

    hull.emplace_back(local_points[0]);
    hull.emplace_back(local_points[1]);

    for (auto i : std::views::iota(2u, local_points.size())) {
        while (hull.size() > 1 && CrossProduct(local_points[i], hull.back(), hull[hull.size() - 2]) <= 0) {
            hull.pop_back();
        }
        hull.emplace_back(local_points[i]);
    }

    return hull;
}
}  // namespace geometry::convex_hull