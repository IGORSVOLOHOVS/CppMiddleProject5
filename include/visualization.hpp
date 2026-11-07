#ifndef GEOMETRY_VISUALIZATION_HPP
#define GEOMETRY_VISUALIZATION_HPP
#include "geometry.hpp"
#include "triangulation.hpp"
#include <span>

namespace geometry::visualization {

void Draw(std::span<geometry::Shape> shapes);

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles);

}  // namespace geometry::visualization

#endif  // GEOMETRY_VISUALIZATION_HPP
