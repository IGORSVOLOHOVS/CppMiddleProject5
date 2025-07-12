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

    /* ваш код здесь */
};

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {

    /* ваш код здесь */
    return 0.0;
}

inline BoundingBox GetBoundBox(const Shape &shape) {

    /* ваш код здесь */
    return {};
}

inline double GetHeight(const Shape &shape) {

    /* ваш код здесь */
    return 0.0;
}

inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {

    /* ваш код здесь */
    return false;
}

std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {

    /* ваш код с ShapeToShapeDistanceVisitor здесь*/
    return std::nullopt;
}

}  // namespace geometry::queries