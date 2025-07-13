#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <format>
#include <numbers>
#include <optional>
#include <print>
#include <ranges>
#include <variant>
#include <vector>

namespace geometry {

struct Line;
struct Triangle;
struct Rectangle;
struct RegularPolygon;
struct Circle;
class Polygon;

enum class GeometryError { Unsupported, NoIntersection, InvalidInput, DegenrateCase, InsufficientPoints };

template <typename T>
using GeometryResult = std::expected<T, GeometryError>;

/*
 * В коде везде используется DummyClass. Ваша задача - выбрать наиболее подходящий тип для решения задачи
 */
using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;

/*
 * Добавьте к методам класса Point2D и Lines2DDyn все необходимые аттрибуты и спецификаторы
 * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
 */
struct Point2D {
    double x, y;

    constexpr Point2D() noexcept : x(0), y(0) {}
    constexpr Point2D(double x, double y) noexcept : x(x), y(y) {}

    // Comparison
    [[nodiscard]] bool operator<(const Point2D &other) const noexcept { return x < other.x && y < other.y; }
    [[nodiscard]] bool operator==(const Point2D &other) const noexcept { return x == other.x && y == other.y; }

    // Binary math operators
    [[nodiscard]] Point2D operator+(const Point2D &other) const noexcept { return {x + other.x, y + other.y}; }
    [[nodiscard]] Point2D operator-(const Point2D &other) const noexcept { return {x - other.x, y - other.y}; }
    [[nodiscard]] Point2D operator*(double value) const noexcept { return {x * value, y * value}; }
    [[nodiscard]] Point2D operator/(double value) const noexcept { return {x / value, y / value}; }

    // Binary geometry operations
    [[nodiscard]] double Dot(const Point2D &other) const noexcept { return x * other.x + y * other.y; }
    [[nodiscard]] double Cross(const Point2D &other) const noexcept { return x * other.y - y * other.x; }
    [[nodiscard]] double Length() const noexcept { return std::sqrt(x * x + y * y); }
    [[nodiscard]] double DistanceTo(const Point2D &other) const noexcept { return (*this - other).Length(); }

    [[nodiscard]] Point2D Normalize() const noexcept {
        const double len = Length();
        return len > 0 ? Point2D{x / len, y / len} : Point2D{0, 0};
    }
};

template <size_t N>
struct Lines2D {
    std::array<double, N> x;
    std::array<double, N> y;
};

struct Lines2DDyn {
    std::vector<double> x;
    std::vector<double> y;

    void Reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }
    void PushBack(Point2D p) {
        x.push_back(p.x);
        y.push_back(p.y);
    }
    void PushBack(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }
    Point2D Front() { return {x.front(), y.front()}; }
};

struct BoundingBox {
    double min_x, min_y, max_x, max_y;

    /* ваш код здесь */
    bool Overlaps(const BoundingBox& other) const {
        if(max_x < other.min_x || min_x > other.max_x 
            || max_y < other.min_y || min_y > other.max_y){
            return false;
        }
        return true;
    }

    double Width() const { return std::abs(max_x - min_x);}
    double Height() const { return std::abs(max_y - min_y);}
    
    GeometryResult<Point2D> Center() const { 
        auto is_zero = [e = 1e-5](const auto& value){
            return std::abs(value) < e;
        };

        double width = Width();
        if(is_zero(width)){
            return std::unexpected{GeometryError::InvalidInput};
        }
        double height = Height();
        if(is_zero(width)){
            return std::unexpected{GeometryError::InvalidInput};
        }

        return GeometryResult<Point2D>{{
            min_x + width / 2, 
            min_y + height / 2
        }};
    }
};

struct Line {
    Point2D start, end;

    double Length() const { return start.DistanceTo(end); }
    Point2D Center() const { return (start - end) / 2; }
    Point2D Direction() const {
        return start - end;
    }
    BoundingBox BoundBox() const { 
        namespace vs = std::views;
        namespace rs = std::ranges;

        auto [min_x, max_x] = rs::minmax(start.x, end.x);
        auto [min_y, max_y] = rs::minmax(start.y, end.y);

        return BoundingBox{
            .min_x = min_x,
            .min_y = min_y,
            .max_x = max_x,
            .max_y = max_y
        };
    } 

    std::vector<Point2D> Vertices() const{ // std::reference_wrapper mb
        return {start, end};
    }

    Lines2DDyn Lines(size_t N = 30) const {
        Lines2DDyn lines{};
        if (N == 0) {
            return lines;
        }
        lines.Reserve(N);

        if (N == 1) {
            lines.PushBack(start);
            return lines;
        }

        namespace vs = std::views;
        namespace rs = std::ranges;

        const Point2D step = (end - start) / static_cast<double>(N - 1);

        auto points_view = vs::iota(0u, N)
                         | vs::transform([this, step](size_t i){
                               return start + step * static_cast<double>(i);
                           });

        rs::for_each(points_view, [&](const Point2D& p){ lines.PushBack(p); });

        return lines;
    }

    /* ваш код здесь */
};

struct Triangle {
    Point2D a, b, c;

    /* ваш код здесь */
    double Area() const { 
        return 0.5 * std::abs(a.x * (b.y - c.y) + b.x * (c.y - a.y) +  c.x * (a.y - b.y));
    }
    double Height() const { return {}; }
    Point2D Center() const { return (a + b + c) / 3; }
    BoundingBox BoundBox() const { 
        namespace rs = std::ranges;

        auto [min_x, max_x] = rs::minmax({a.x, b.x, c.x});
        auto [min_y, max_y] = rs::minmax({a.y, b.y, c.y});

        return BoundingBox{
            .min_x = min_x,
            .min_y = min_y,
            .max_x = max_x,
            .max_y = max_y
        };
    } 

    std::vector<Point2D> Vertices() const{ // std::reference_wrapper mb
        return {a, b, c};
    }

    Lines2DDyn Lines(size_t N = 30) const {
        Lines2DDyn lines{};
        if (N == 0) return lines;
        lines.Reserve(N + 1);

        const std::array<Line, 3> sides = {{ {a, b}, {b, c}, {c, a} }};
        const size_t points_per_side = N / 3;

        namespace rs = std::ranges;
        namespace vs = std::views;

        rs::for_each(sides | vs::enumerate,
            [&](const auto& enumerated_side) {
                auto&& [i, side] = enumerated_side;

                const size_t current_points = (i == 2) ? (N - 2 * points_per_side) : points_per_side;
                if (current_points == 0) return; 

                const Point2D step = (side.end - side.start) / static_cast<double>(current_points);

                auto generated_points = vs::iota(0u, current_points) |
                                        vs::transform([=](size_t j) {
                                            return side.start + step * static_cast<double>(j);
                                        });
                rs::for_each(generated_points, [&](const auto& p){ lines.PushBack(p); });
            }
        );

        lines.PushBack(a); 
        return lines;
    }
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    double Area() const { 
        return width * height;
    }
    double Height() const { return height; }
    Point2D Center() const { return {bottom_left.x + width / 2, bottom_left.y + height / 2}; }
    BoundingBox BoundBox() const { 
        return BoundingBox{
            .min_x = bottom_left.x,
            .min_y = bottom_left.y,
            .max_x = bottom_left.x + width,
            .max_y = bottom_left.y + height
        };
    } 

    std::vector<Point2D> Vertices() const{ // std::reference_wrapper mb
        return {
            bottom_left, 
            { bottom_left.x, bottom_left.y + height},
            { bottom_left.x + width, bottom_left.y + height},
            { bottom_left.x + width, bottom_left.y},
        };
    }

    Lines2DDyn Lines(size_t N = 30) const {
        Lines2DDyn lines{};
        const auto verts = Vertices();
        const size_t num_sides = verts.size(); 
        if (N == 0 || num_sides == 0) return lines;
        lines.Reserve(N + 1);

        const size_t points_per_side = N / num_sides;
        namespace rs = std::ranges;
        namespace vs = std::views;

        auto sides = vs::iota(0u, num_sides)
                | vs::transform([&verts, num_sides](size_t i){ return Line{verts[i], verts[(i + 1) % num_sides]}; });

        rs::for_each(sides | vs::enumerate,
            [&](const auto& enumerated_side) {
                auto&& [i, side] = enumerated_side;
                
                const size_t current_points = (static_cast<size_t>(i) == num_sides - 1) ? (N - (num_sides - 1) * points_per_side) : points_per_side;

                if (current_points == 0) return;

                const Point2D step = (side.end - side.start) / static_cast<double>(current_points);
                auto generated_points = vs::iota(0u, current_points) | vs::transform([=](size_t j) { return side.start + step * static_cast<double>(j); });
                rs::for_each(generated_points, [&](const auto& p){ lines.PushBack(p); });
            }
        );
        lines.PushBack(verts.front()); 
        return lines;
    }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {}
    Point2D Center() const noexcept {
        return center_p;
    }
    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (int i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }
        return points;
    }

    Lines2DDyn Lines(size_t points_per_side = 10) const {
        Lines2DDyn lines{};
        const auto verts = Vertices();
        const size_t num_sides = verts.size();
        if (num_sides == 0) return lines;
        lines.Reserve(num_sides * points_per_side + 1);

        namespace rs = std::ranges;
        namespace vs = std::views;

        auto sides = vs::iota(0u, num_sides)
                   | vs::transform([&verts, num_sides](size_t i){ return Line{verts[i], verts[(i + 1) % num_sides]}; });

        rs::for_each(sides, [&](const Line& side) {
            const Point2D step = (side.end - side.start) / static_cast<double>(points_per_side);
            auto generated_points = vs::iota(0u, points_per_side) | vs::transform([=](size_t j) { return side.start + step * static_cast<double>(j); });
            rs::for_each(generated_points, [&](const auto& p){ lines.PushBack(p); });
        });

        lines.PushBack(verts.front()); 
        return lines;
    }
};

struct Circle {
    Point2D center_p;
    double radius;

    constexpr Circle(Point2D center, double radius) : center_p(center), radius(radius) {}

    BoundingBox BoundBox() {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }
    double Height() { return center_p.y + radius; }
    Point2D Center() { return center_p; }

    //
    // Должны быть сделана по аналогии с RegularPolygon::Vertices
    //
    std::vector<Point2D> Vertices(size_t N = 30) const {
        std::vector<Point2D> points;
        if (N == 0) return points;
        points.reserve(N);

        namespace vs = std::views;
        namespace rs = std::ranges;

        auto points_view = vs::iota(0u, N)
                         | vs::transform([this, N](size_t i) {
                               const double angle = 2.0 * std::numbers::pi * i / N;
                               return Point2D{center_p.x + radius * std::cos(angle),
                                              center_p.y + radius * std::sin(angle)};
                           });

        rs::copy(points_view, std::back_inserter(points));
        return points;
    }
    
    Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn lines{};
        if (N == 0) return lines;
        lines.Reserve(N + 1);
        
        namespace vs = std::views;
        namespace rs = std::ranges;

        auto points_view = vs::iota(0u, N)
                         | vs::transform([this, N](size_t i) {
                               const double angle = 2.0 * std::numbers::pi * i / N;
                               return Point2D{center_p.x + radius * std::cos(angle),
                                              center_p.y + radius * std::sin(angle)};
                           });

        rs::for_each(points_view, [&](const auto& p){ lines.PushBack(p); });
        
        lines.PushBack(lines.Front());
        return lines;
    }
};

class Polygon {
public:
   explicit Polygon(std::vector<Point2D> points) : points_(std::move(points)) {
        if (points_.empty()) {
            bounding_box_ = {0,0,0,0};
            return;
        }

        namespace vs = std::views;
        namespace rs = std::ranges;

        auto x_coords = points_ | vs::transform([](const Point2D& p){ return p.x; });
        auto y_coords = points_ | vs::transform([](const Point2D& p){ return p.y; });
        
        const auto [min_x, max_x] = rs::minmax(x_coords);
        const auto [min_y, max_y] = rs::minmax(y_coords);

        bounding_box_ = {.min_x = min_x, .min_y = min_y, .max_x = max_x, .max_y = max_y};
    }

    const std::vector<Point2D>& Vertices() const noexcept {
        return points_;
    }

    BoundingBox BoundBox() const noexcept {
        return bounding_box_;
    }

    double Height() const noexcept {
        return bounding_box_.Height();
    }

    Point2D Center() const noexcept {
        return bounding_box_.Center().value_or(Point2D{});
    }

    Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn lines{};
        const size_t num_sides = points_.size();
        if (N == 0 || num_sides == 0) return lines;
        lines.Reserve(N + 1);

        const size_t points_per_side = N / num_sides;
        namespace rs = std::ranges;
        namespace vs = std::views;

        auto sides = vs::iota(0u, num_sides)
                   | vs::transform([this, num_sides](size_t i){ return Line{points_[i], points_[(i + 1) % num_sides]}; });

        rs::for_each(sides | vs::enumerate,
            [&](const auto& enumerated_side) {
                auto&& [i, side] = enumerated_side;
                
                const size_t current_points = (static_cast<size_t>(i) == num_sides - 1) ? (N - (num_sides - 1) * points_per_side) : points_per_side;

                if (current_points == 0) return;

                const Point2D step = (side.end - side.start) / static_cast<double>(current_points);
                auto generated_points = vs::iota(0u, current_points) | vs::transform([=](size_t j) { return side.start + step * static_cast<double>(j); });
                rs::for_each(generated_points, [&](const auto& p){ lines.PushBack(p); });
            }
        );
        lines.PushBack(points_.front()); 
        return lines;
    }

private:
    std::vector<Point2D> points_;
    BoundingBox bounding_box_;
};
}  // namespace geometry

template <>
struct std::formatter<geometry::Point2D> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Point2D &p, FormatContext &ctx) const {
        return format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
    }
};
template <>
struct std::formatter<std::vector<geometry::Point2D>> {
    mutable bool use_new_line = false;

    constexpr auto parse(std::format_parse_context &ctx) const {
        auto it = ctx.begin(), end = ctx.end();

        auto spec_end = it;
        while (spec_end != end && *spec_end != '}') {
            ++spec_end;
        }

        std::string_view spec(it, spec_end);
        if (spec == "new_line") {
            use_new_line = true;
        } else if (!spec.empty()) {
            throw std::format_error("invalid format specifier for vector<Point2D>");
        }

        return spec_end;
    }

    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) const {
        auto out = ctx.out();
        if (use_new_line) {
            for (const auto &p : v) {
                out = std::format_to(out, "\n\t{}", p);
            }
        } else {
            out = std::format_to(out, "[");
            if (!v.empty()) {
                out = std::format_to(out, "{}", v.front());
                for (const auto &p : v | std::views::drop(1)) {
                    out = std::format_to(out, ", {}", p);
                }
            }
            out = std::format_to(out, "]");
        }
        return out;
    }
};

template <>
struct std::formatter<geometry::Line> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Line &l, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Line({}, {})", l.start, l.end);
    }
};

template <>
struct std::formatter<geometry::Circle> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Circle &c, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.center_p, c.radius);
    }
};

template <>
struct std::formatter<geometry::Rectangle> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Rectangle &r, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.bottom_left, r.width,
                              r.height);
    }
};

template <>
struct std::formatter<geometry::RegularPolygon> {
    constexpr auto parse(std::format_parse_context &ctx)const{ return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::RegularPolygon &p, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.center_p, p.radius,
                              p.sides);
    }
};
template <>
struct std::formatter<geometry::Triangle> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Triangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.a, t.b, t.c);
    }
};
template <>
struct std::formatter<geometry::Polygon> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Polygon &poly, FormatContext &ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "Polygon[{} points]: [", poly.Vertices().size());

        for (const auto &p : poly.Vertices()) {
            out = std::format_to(out, "{} ", p);
        }

        return std::format_to(out, "]");
    }
};
