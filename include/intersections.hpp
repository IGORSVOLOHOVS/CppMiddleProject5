#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>

namespace geometry::intersections {

/*
 * Класс для поиска пересечений между двумя фигурами
 *
 * Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
 *    - Line   & Line
 *    - Line   & Circle
 *    - Circle & Circle
 *
 * Для всех остальных требуется выбросить исключение std::logic_error
 */
class IntersectionVisitor {
public:
    using IntersectionResult = GeometryResult<Point2D>;

    IntersectionResult operator()(const Line& l1, const Line& l2) const {
        const Point2D p1 = l1.start, p2 = l1.end;
        const Point2D p3 = l2.start, p4 = l2.end;

        const double det = (p2.x - p1.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p4.x - p3.x);

        if (std::abs(det) < 1e-9) {
            return std::unexpected{GeometryError::DegenrateCase};
        }

        const double t = ((p3.x - p1.x) * (p4.y - p3.y) - (p3.y - p1.y) * (p4.x - p3.x)) / det;
        const double u = ((p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)) / det;

        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            return Point2D{p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y)};
        }

        return std::unexpected{GeometryError::NoIntersection};
    }

    IntersectionResult operator()(const Line& l, const Circle& c) const {
        const Point2D d = l.end - l.start;
        const Point2D oc = l.start - c.center_p;

        const double a = d.Dot(d);
        const double b = 2 * oc.Dot(d);
        const double C = oc.Dot(oc) - c.radius * c.radius;

        const double discriminant = b * b - 4 * a * C;
        if (discriminant < 0) {
            return std::unexpected{GeometryError::NoIntersection};
        }

        const double sqrt_d = std::sqrt(discriminant);
        const double t1 = (-b - sqrt_d) / (2 * a);
        const double t2 = (-b + sqrt_d) / (2 * a);

        if (t1 >= 0 && t1 <= 1) {
            return l.start + d * t1;
        }
        if (t2 >= 0 && t2 <= 1) {
            return l.start + d * t2;
        }

        return std::unexpected{GeometryError::NoIntersection};
    }

    IntersectionResult operator()(const Circle& c1, const Circle& c2) const {
        const double d = c1.center_p.DistanceTo(c2.center_p);
        const double r1 = c1.radius, r2 = c2.radius;

        if (d > r1 + r2 || d < std::abs(r1 - r2)) {
            return std::unexpected{GeometryError::NoIntersection};
        }
        if (d == 0 && r1 == r2) {
             return std::unexpected{GeometryError::DegenrateCase};
        }

        const double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
        const double h = std::sqrt(r1 * r1 - a * a);

        const Point2D p_mid = c1.center_p + (c2.center_p - c1.center_p) * (a / d);

        return Point2D{
            p_mid.x + h * (c2.center_p.y - c1.center_p.y) / d,
            p_mid.y - h * (c2.center_p.x - c1.center_p.x) / d
        };
    }

    IntersectionResult operator()(auto&&, auto&&) const {
        throw std::logic_error{"Intersection for these types is not supported."};
    }
};

inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) { 
    return std::visit(IntersectionVisitor{}, shape1, shape2)
        .transform([](const Point2D& point) { 
            return std::optional<Point2D>(point);
        })
        .value_or(std::nullopt);
}

}  // namespace geometry::intersections