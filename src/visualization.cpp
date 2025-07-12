#include "visualization.hpp"
#include "geometry.hpp"

#include <matplot/matplot.h>
#include <print>

namespace geometry::visualization {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

void Draw(std::span<geometry::Shape> shapes) {
    using namespace geometry;
    using namespace matplot;

    // Disable gnuplot warnings
    auto f = figure(false);
    f->backend()->run_command("unset warnings");
    f->ioff();
    f->size(900, 900);

    hold(on);     // Multiple plots mode
    axis(equal);  // Squre view
    grid(on);     // Enable grid by default

    for (const auto &[index, shape] : std::ranges::views::enumerate(shapes)) {
        std::visit(Multilambda{[&](const Line &line) {
                                   const auto lines = line.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("yellow");
                               },
                               [&](const Triangle &tri) {
                                   const auto lines = tri.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("blue");
                               },
                               [&](const Rectangle &rect) {
                                   const auto lines = rect.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("green");
                               },
                               [&](const RegularPolygon &poly) {
                                   const auto lines = poly.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("magenta");
                               },
                               [&](const Circle &circle) {
                                   const auto lines = circle.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("red");
                               },
                               [&](const Polygon &poly) {
                                   const auto lines = poly.Lines();
                                   plot(lines.x, lines.y)->line_width(2).color("cyan");
                               }},
                   shape);

        // Add shape number
        const auto center = shape.visit([](auto &&s) { return s.Center(); });
        auto t = text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    // Display plot
    f->show();
}

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles) {
    // dаш код здесь
}

}  // namespace geometry::visualization
