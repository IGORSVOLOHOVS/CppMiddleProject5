#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <iostream>
#include <print>
#include <iterator>
#include <ranges>

using namespace geometry;

namespace rs = std::ranges;
namespace vs = std::views;

void PrintAllIntersections(const Shape &shape, std::span<const Shape> others) {
    std::println("\nIntersections with {}:", shape);

    rs::for_each(others | vs::filter([&](const auto& s){ return &s != &shape; }), 
        [&](const Shape& other_shape) {
            try {
                if (auto pt = intersections::GetIntersectPoint(shape, other_shape)) {
                    std::println("  - vs {}: FOUND at {}", other_shape, *pt);
                }
            } catch (const std::logic_error&) {} 
    });
}

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\nDistances from {}:", p);

    rs::for_each(shapes | vs::take(5), [&](const Shape& s) {
        std::println("  - dist to {}: {:.2f}", s, queries::DistanceToPoint(s, p));
    });
}

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\nShape Analysis:");

    std::println(" Bounding Box Collisions:");
    auto colliding_pairs = vs::cartesian_product(shapes, shapes)
                         | vs::filter([](const auto& p) {
                               auto& [s1, s2] = p;
                               return &s1 < &s2 && queries::BoundingBoxesOverlap(s1, s2);
                           });
    rs::for_each(colliding_pairs, [](const auto& p) {
        std::println("  - {} and {}", std::get<0>(p), std::get<1>(p));
    });

    if (auto it = rs::max_element(shapes, {}, &queries::GetHeight); it != shapes.end()) {
        std::println(" Highest: {} (h={:.2f})", *it, queries::GetHeight(*it));
    }

    auto supported_dist = vs::cartesian_product(shapes, shapes)
                        | vs::filter([](const auto& p) { auto& [s1, s2] = p; return &s1 < &s2; })
                        | vs::transform([](const auto& p) {
                              return std::tuple{std::get<0>(p), std::get<1>(p), queries::DistanceBetweenShapes(std::get<0>(p), std::get<1>(p))};
                          })
                        | vs::filter([](const auto& t) { return std::get<2>(t).has_value(); })
                        | vs::take(1);

    if (!supported_dist.empty()) {
         std::println(" Sample Distance:");
         rs::for_each(supported_dist, [](const auto& t) {
            std::println("  - {} vs {}: dist={:.2f}", std::get<0>(t), std::get<1>(t), *std::get<2>(t));
        });
    }
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\nExtra Shape Analysis:");

    auto high_shapes = shapes
                     | vs::filter([](const auto& s) { return queries::GetBoundBox(s).min_y > 50.0; })
                     | vs::take(3);

    if(!high_shapes.empty()){
        std::println(" Shapes above y=50.0:");
        rs::for_each(high_shapes, [](const auto& s) { std::println("  - {}", s); });
    }

    if (!shapes.empty()) {
        auto [min_it, max_it] = rs::minmax_element(shapes, {}, &queries::GetHeight);
        std::println(" Min/Max Height:");
        std::println("  - Min: {} (h={:.2f})", *min_it, queries::GetHeight(*min_it));
        std::println("  - Max: {} (h={:.2f})", *max_it, queries::GetHeight(*max_it));
    }
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    std::vector<Shape> shapes = generator.GenerateShapes(15);

    std::println("Generated {} shapes:", shapes.size());
    rs::for_each(vs::enumerate(shapes), [](const auto& indexed_shape) {
        auto [index, shape] = indexed_shape;
        std::println(" [{:2}] h={:5.2f}, {}", index, queries::GetHeight(shape), shape);
    });

    if (!shapes.empty()) {
        PrintAllIntersections(shapes[0], shapes);
    }
    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);
    PerformShapeAnalysis(shapes);
    PerformExtraShapeAnalysis(shapes);
    
    geometry::visualization::Draw(shapes);
    
    auto all_vertices_view = shapes
                           | vs::transform([](const Shape& s) {
                                 return std::visit([](const auto& s_var) { return s_var.Vertices(); }, s);
                             })
                           | vs::join;
    std::vector<Point2D> points;
    rs::copy(all_vertices_view, std::back_inserter(points));

    if (auto hull_result = geometry::convex_hull::GrahamScan(points)) {
        shapes.push_back(geometry::Polygon(*hull_result));
        geometry::visualization::Draw(shapes);
    } else {
        std::println("Error {}", static_cast<int>(hull_result.error()));
    }
    
    {
        std::vector<Point2D> delaunay_points = {{0,0}, {10,0}, {5,8}, {15,5}, {2,12}, {18,1}, {12,15}};

        if (auto tri_result = geometry::triangulation::DelaunayTriangulation(delaunay_points)) {
            geometry::visualization::Draw(*tri_result);
        } else {
            std::println("Error {}", static_cast<int>(tri_result.error()));
        }
    }
    
    return 0;
}