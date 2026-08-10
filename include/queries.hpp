#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <variant>

namespace geometry::queries {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

/*
 * Класс для поиска расстояния от точки до фигуры
 *
 * Требуется организовать возможность нахождения расстояния для всех возможных фигур типа-суммы Shape
 */
struct PointToShapeDistanceVisitor {
    Point2D point;

    explicit PointToShapeDistanceVisitor(const Point2D& p) : point(p) {}

    double operator()(const Line& line) const {
        const auto v_segment = line.end - line.start;
        const auto v_to_point = point - line.start;
        const double segment_len_sq = v_segment.Dot(v_segment);

        if (std::abs(segment_len_sq) < 1e-9) {
            return point.DistanceTo(line.start);
        }

        const double t = std::clamp(v_to_point.Dot(v_segment) / segment_len_sq, 0.0, 1.0);
        const Point2D projection_point = line.start + v_segment * t;
        return point.DistanceTo(projection_point);
    }

    double operator()(const Circle& circle) const {
        const double r = std::abs(circle.radius);
        const double dist_to_center = point.DistanceTo(circle.center_p);
        return std::max(0.0, dist_to_center - r);
    }

    double operator()(const Rectangle& rect) const {
        // См. Rectangle::BoundBox(): std::minmax возвращает ссылки, а
        // rect.bottom_left.x + rect.width - временное, которое умирает сразу.
        const double right = rect.bottom_left.x + rect.width;
        const double top = rect.bottom_left.y + rect.height;
        const auto [min_x, max_x] = std::minmax(rect.bottom_left.x, right);
        const auto [min_y, max_y] = std::minmax(rect.bottom_left.y, top);

        if (point.x >= min_x && point.x <= max_x && point.y >= min_y && point.y <= max_y) {
            return 0.0;
        }

        return std::ranges::min(rect.Edges() | std::views::transform([this](const Line& edge) { return (*this)(edge); }));
    }

    double operator()(const Triangle& triangle) const {
        const auto v1 = triangle.b - triangle.a, v2 = point - triangle.a;
        const auto v3 = triangle.c - triangle.b, v4 = point - triangle.b;
        const auto v5 = triangle.a - triangle.c, v6 = point - triangle.c;
        const double c1 = v1.Cross(v2), c2 = v3.Cross(v4), c3 = v5.Cross(v6);

        if ((c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0)) {
            return 0.0;
        }
        
        return std::ranges::min(triangle.Edges() | std::views::transform([this](const Line& edge) { return (*this)(edge); }));
    }

    double operator()(const RegularPolygon& poly) const {
        return (*this)(Polygon(poly.Vertices()));
    }

    double operator()(const Polygon& poly) const {
        const auto& vertices = poly.Vertices();
        if (vertices.empty()) return std::numeric_limits<double>::infinity();
        if (vertices.size() == 1) return point.DistanceTo(vertices[0]);

        bool inside = false;
        for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
            if (((vertices[i].y > point.y) != (vertices[j].y > point.y)) &&
                (point.x < (vertices[j].x - vertices[i].x) * (point.y - vertices[i].y) / (vertices[j].y - vertices[i].y) + vertices[i].x)) {
                inside = !inside;
            }
        }
        if (inside) {
            return 0.0;
        }

        auto distances_view = poly.Edges() | std::views::transform([this](const Line& edge) { return (*this)(edge); });
        return std::ranges::min(distances_view);
    }
};

/*
 * Класс для поиска расстояния между двумя фигурами
 *
 * Требуется организовать возможность нахождения расстояния только для следующих комбинаций фигур:
 *    - Any    & Point
 *    - Line   & Line
 *    - Circle & Circle
 *
 * Для всех остальных требуется вернуть пустое значение
 */
struct ShapeToShapeDistanceVisitor {
    using Distance = std::optional<double>;

    Distance operator()(const Circle& c1, const Circle& c2) const {
        double dist_centers = c1.center_p.DistanceTo(c2.center_p);
        double dist = dist_centers - c1.radius - c2.radius;
        return std::max(0.0, dist); 
    }

    Distance operator()(const Line& l1, const Line& l2) const {
        double d1 = PointToShapeDistanceVisitor{l1.start}(l2);
        double d2 = PointToShapeDistanceVisitor{l1.end}(l2);

        double d3 = PointToShapeDistanceVisitor{l2.start}(l1);
        double d4 = PointToShapeDistanceVisitor{l2.end}(l1);

        return std::min({d1, d2, d3, d4});
    }

    Distance operator()(auto&&, auto&&) const {
        return std::nullopt;
    }
};

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {
    return std::visit(PointToShapeDistanceVisitor{point}, shape);
}

inline BoundingBox GetBoundBox(const Shape &shape) {
    return std::visit([](const auto& s){ return s.BoundBox(); }, shape);
}

inline double GetHeight(const Shape &shape) {
    return GetBoundBox(shape).Height();
}

inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {
    const auto box1 = GetBoundBox(shape1);
    const auto box2 = GetBoundBox(shape2);
    return box1.Overlaps(box2);
}

inline std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {
    return std::visit(ShapeToShapeDistanceVisitor{}, shape1, shape2);
}

}  // namespace geometry::queries