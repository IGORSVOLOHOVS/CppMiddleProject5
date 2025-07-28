#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>
#include <variant> 

namespace geometry::intersections {

struct NoIntersection {};
struct InfiniteIntersections {};
using PointPair = std::pair<Point2D, Point2D>;

using Intersection = std::variant<NoIntersection, Point2D, PointPair, InfiniteIntersections>;

class IntersectionVisitor {
public:
    Intersection operator()(const Line& l1, const Line& l2) const {
        const auto p1 = l1.start, v1 = l1.Direction();
        const auto p2 = l2.start, v2 = l2.Direction();
        const double denominator = v1.Cross(v2);

        if (std::abs(denominator) < 1e-9) {
            return NoIntersection{};
        }
        const double t = (p2 - p1).Cross(v2) / denominator;
        const double u = (p1 - p2).Cross(v1) / -denominator;

        if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
            return p1 + v1 * t;
        }
        return NoIntersection{};
    }

    Intersection operator()(const Line& l, const Circle& c) const {
        const double r = std::abs(c.radius);
        const Point2D d = l.Direction();
        const Point2D oc = l.start - c.center_p;

        const double a = d.Dot(d);
        const double b = 2 * oc.Dot(d);
        const double C = oc.Dot(oc) - r * r;

        const double discriminant = b * b - 4 * a * C;
        if (discriminant < -1e-9) { 
            return NoIntersection{};
        }

        std::vector<Point2D> points;
        const double sqrt_d = (discriminant > 0) ? std::sqrt(discriminant) : 0;
        
        const double t1 = (-b - sqrt_d) / (2 * a);
        if (t1 >= 0 && t1 <= 1) {
            points.push_back(l.start + d * t1);
        }

        if (discriminant > 1e-9) { 
            const double t2 = (-b + sqrt_d) / (2 * a);
            if (t2 >= 0 && t2 <= 1) {
                points.push_back(l.start + d * t2);
            }
        }
        
        if (points.empty()) return NoIntersection{};
        if (points.size() == 1) return points[0];
        return PointPair{points[0], points[1]};
    }

    Intersection operator()(const Circle& c1, const Circle& c2) const {
        const double r1 = std::abs(c1.radius), r2 = std::abs(c2.radius);
        const double d = c1.center_p.DistanceTo(c2.center_p);

        if (d > r1 + r2 + 1e-9 || d < std::abs(r1 - r2) - 1e-9) {
            return NoIntersection{};
        }
        if (d < 1e-9 && std::abs(r1 - r2) < 1e-9) {
            return InfiniteIntersections{};
        }

        const double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
        const double h_squared = r1 * r1 - a * a;
        const double h = (h_squared > 0) ? std::sqrt(h_squared) : 0;
        
        const Point2D p_mid = c1.center_p + (c2.center_p - c1.center_p) * (a / d);

        if (h < 1e-9) { 
            return p_mid;
        }

        const Point2D v = (c2.center_p - c1.center_p).Normalize() * h;
        const Point2D p1 = {p_mid.x + v.y, p_mid.y - v.x};
        const Point2D p2 = {p_mid.x - v.y, p_mid.y + v.x};
        return PointPair{p1, p2};
    }

    Intersection operator()(auto&&, auto&&) const {
        throw std::logic_error{"Intersection for these types is not supported."};
    }
};

inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    const Intersection result = std::visit(IntersectionVisitor{}, shape1, shape2);

    return std::visit(
        [](auto&& arg) -> std::optional<Point2D> {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, Point2D>) {
                return arg; 
            }
            if constexpr (std::is_same_v<T, PointPair>) {
                return arg.first; 
            }
            return std::nullopt;
        },
        result);
}

} // namespace geometry::intersections