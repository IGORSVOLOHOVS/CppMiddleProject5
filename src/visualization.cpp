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
        // Не shape.visit(...): метод-visit у std::variant - это C++26 (P2637R3),
        // libstdc++ 15 его уже имеет, MSVC STL 14.44 - ещё нет. Свободная
        // std::visit делает ровно то же самое и есть везде начиная с C++17.
        const auto center = std::visit([](auto &&s) { return s.Center(); }, shape);
        auto t = text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    // Display plot
    f->show();
}

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles) {
    using namespace matplot;

    auto f = figure(true);
    f->backend()->run_command("unset warnings");
    f->ioff();
    f->size(900, 900);

    hold(on);
    axis(equal);
    grid(on);

    for (const auto &tri : triangles) {
        std::vector<double> x_coords = {tri.a.x, tri.b.x, tri.c.x, tri.a.x};
        std::vector<double> y_coords = {tri.a.y, tri.b.y, tri.c.y, tri.a.y};

        plot(x_coords, y_coords)->line_width(1.5).color("blue");
    }

    f->show();
}
}  // namespace geometry::visualization
