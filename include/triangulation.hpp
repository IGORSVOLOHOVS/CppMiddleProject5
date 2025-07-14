#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <flat_map> 
#include <vector>

namespace geometry::triangulation {

struct DelaunayTriangle {
    Point2D a, b, c;

    DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

    bool ContainsPoint(const Point2D &p) const {
        Point2D center = Circumcenter();
        double radius = Circumradius();
        return center.DistanceTo(p) <= radius + 1e-10;
    }

    Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < 1e-10) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    double Circumradius() const {
        Point2D center = Circumcenter();
        return center.DistanceTo(a);
    }

    bool SharesEdge(const DelaunayTriangle &other) const {
        std::vector<Point2D> this_points = {a, b, c};
        std::vector<Point2D> other_points = {other.a, other.b, other.c};

        int shared_count = 0;
        for (const Point2D &p1 : this_points) {
            for (const Point2D &p2 : other_points) {
                if (std::abs(p1.x - p2.x) < 1e-10 && std::abs(p1.y - p2.y) < 1e-10) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge &other) const {
        if (std::abs(p1.x - other.p1.x) > 1e-10)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > 1e-10)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > 1e-10)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    bool operator==(const Edge &other) const {
        return std::abs(p1.x - other.p1.x) < 1e-10 && std::abs(p1.y - other.p1.y) < 1e-10 &&
               std::abs(p2.x - other.p2.x) < 1e-10 && std::abs(p2.y - other.p2.y) < 1e-10;
    }
};

inline GeometryResult<std::vector<DelaunayTriangle>> DelaunayTriangulation(std::span<const Point2D> points) {
    constexpr unsigned SCALE_COEF = 20;

    if (points.size() < 3) {
        return std::unexpected(GeometryError::InsufficientPoints);
    }

    namespace rs = std::ranges;
    namespace vs = std::views;

    std::vector<DelaunayTriangle> triangulation;

    auto [min_x, max_x] = rs::minmax(points | vs::transform(&Point2D::x));
    auto [min_y, max_y] = rs::minmax(points | vs::transform(&Point2D::y));
    
    double dx = max_x - min_x, dy = max_y - min_y;
    double delta_max = std::max(dx, dy);
    Point2D mid_point{(min_x + max_x) / 2, (min_y + max_y) / 2};

    Point2D p1{mid_point.x - SCALE_COEF * delta_max, mid_point.y - delta_max};
    Point2D p2{mid_point.x, mid_point.y + SCALE_COEF * delta_max};
    Point2D p3{mid_point.x + SCALE_COEF * delta_max, mid_point.y - delta_max};
    
    triangulation.emplace_back(p1, p2, p3);
    const DelaunayTriangle super_triangle{p1, p2, p3};


    for (const Point2D& point : points) {
        
        auto bad_triangles_view = triangulation 
                                | vs::filter([&](const auto& tri) { return tri.ContainsPoint(point); });
        std::vector<DelaunayTriangle> bad_triangles(bad_triangles_view.begin(), bad_triangles_view.end());

        if (bad_triangles.empty()) continue;

        std::flat_map<Edge, int> edge_counts;
        for (const auto& tri : bad_triangles) {
            auto v = tri.vertices();
            edge_counts[Edge(v[0], v[1])]++;
            edge_counts[Edge(v[1], v[2])]++;
            edge_counts[Edge(v[2], v[0])]++;
        }

        auto hole_edges_view = edge_counts 
                             | vs::filter([](const auto& pair) { return pair.second == 1; }) 
                             | vs::keys;
        
        std::erase_if(triangulation, [&bad_triangles](const DelaunayTriangle& tri){
            return rs::any_of(bad_triangles, [&](const DelaunayTriangle& bad) {
                    return tri.a == bad.a && tri.b == bad.b && tri.c == bad.c;
                });
        });

        for (const auto& edge : hole_edges_view) {
            triangulation.emplace_back(edge.p1, edge.p2, point);
        }
    }

    std::erase_if(triangulation, [&super_triangle](const DelaunayTriangle& tri){
        auto has_super_vertex = [&super_triangle](const Point2D& v) {
            return v == super_triangle.a || v == super_triangle.b || v == super_triangle.c;
        };
        return has_super_vertex(tri.a) || has_super_vertex(tri.b) || has_super_vertex(tri.c);
    });

    return triangulation;
}
}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};
