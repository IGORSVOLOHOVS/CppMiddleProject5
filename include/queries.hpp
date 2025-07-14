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

    explicit PointToShapeDistanceVisitor(const Point2D &p) : point(p) {}

    double operator()(const Line& line) const {
        const Point2D& point0 = line.start;
        const Point2D& point1 = line.end;
        const auto v_segment = point1 - point0;
        const auto v_to_point = point - point0;
        const double segment_len_sq = v_segment.Dot(v_segment);

        if (std::abs(segment_len_sq) < 1e-9) {
            return point.DistanceTo(point0);
        }

        const double t = std::clamp(v_to_point.Dot(v_segment) / segment_len_sq, 0.0, 1.0);
        const Point2D projection_point = point0 + v_segment * t;
        return point.DistanceTo(projection_point);
    }

    double operator()(const Circle& circle) const {
        const double dist_to_center = point.DistanceTo(circle.center_p);
        return std::max(0.0, dist_to_center - circle.radius);
    }
    
    double operator()(const Rectangle& rect) const {
        const double closest_x = std::clamp(point.x, rect.bottom_left.x, rect.bottom_left.x + rect.width);
        const double closest_y = std::clamp(point.y, rect.bottom_left.y, rect.bottom_left.y + rect.height);
        
        return point.DistanceTo({closest_x, closest_y});
    }

    double operator()(const Triangle& triangle) const {
        double d1 = (*this)(Line{triangle.a, triangle.b});
        double d2 = (*this)(Line{triangle.b, triangle.c});
        double d3 = (*this)(Line{triangle.c, triangle.a});
        
        return std::min({d1, d2, d3});
    }

    double operator()(const RegularPolygon& poly) const {
        return (*this)(Polygon(poly.Vertices()));
    }
    
    double operator()(const Polygon& poly) const {
        const auto& vertices = poly.Vertices();
        if (vertices.empty()) {
            return std::numeric_limits<double>::infinity();
        }
        if (vertices.size() == 1) {
            return point.DistanceTo(vertices[0]);
        }

        namespace vs = std::views;
        namespace rs = std::ranges;

        auto edges_view = vs::iota(0u, vertices.size())
                        | vs::transform([&](size_t i) {
                            return Line{vertices[i], vertices[(i + 1) % vertices.size()]};
                        });

        auto distances_view = edges_view
                            | vs::transform([this](const Line& edge) {
                                return (*this)(edge);
                            });

        return rs::min(distances_view);
    }

    /* ваш код здесь */
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