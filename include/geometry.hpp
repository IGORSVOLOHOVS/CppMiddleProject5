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
#include <limits>

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

// Позволяет переопределить эпсилон при компиляции (например, -DGEOMETRY_EPSILON=1e-12)
#ifndef GEOMETRY_EPSILON
#define GEOMETRY_EPSILON 1e-10
#endif

inline constexpr double EPSILON = GEOMETRY_EPSILON;  

[[nodiscard]] constexpr bool is_zero(double value) noexcept {
    return std::abs(value) < EPSILON;
}

[[nodiscard]] constexpr bool are_equal(double a, double b) noexcept {
    return is_zero(a - b);
}

[[nodiscard]] constexpr bool is_less(double a, double b) noexcept {
    return (b - a) > EPSILON;
}

[[nodiscard]] constexpr bool is_greater(double a, double b) noexcept {
    return (a - b) > EPSILON;
}


/*
 * Добавьте к методам класса Point2D и Lines2DDyn все необходимые аттрибуты и спецификаторы
 * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
 */
struct Point2D {
    double x, y;

    constexpr Point2D() noexcept : x(0), y(0) {}
    constexpr Point2D(double x, double y) noexcept : x(x), y(y) {}

    // Comparison
    [[nodiscard]] bool operator<(const Point2D &other) const noexcept { return std::tie(x, y) < std::tie(other.x, other.y); }
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
    bool Overlaps(const BoundingBox &other) const {
        if (is_less(max_x, other.min_x) || is_greater(min_x, other.max_x) ||
            is_less(max_y, other.min_y) || is_greater(min_y, other.max_y)) {
            return false;
        }
        return true;
    }

    double Width() const { return std::abs(max_x - min_x); }
    double Height() const { return std::abs(max_y - min_y); }

    GeometryResult<Point2D> Center() const {
        return std::expected<const BoundingBox*, GeometryError>{this}
            .and_then([&](const BoundingBox* box) -> std::expected<const BoundingBox*, GeometryError> {
                if (is_zero(box->Width())) {
                    return std::unexpected{GeometryError::InvalidInput};
                }
                return box;
            })
            .and_then([&](const BoundingBox* box) -> std::expected<const BoundingBox*, GeometryError> {
                if (is_zero(box->Height())) {
                    return std::unexpected{GeometryError::InvalidInput};
                }
                return box;
            })
            .transform([](const BoundingBox* box) -> Point2D {
                return {box->min_x + box->Width() / 2, box->min_y + box->Height() / 2};
        });
    }
};

struct Line {
    Point2D start, end;
    
    [[nodiscard]] double Height() const noexcept { return BoundBox().Height(); }
    [[nodiscard]] double Length() const noexcept { return start.DistanceTo(end); }
    [[nodiscard]] Point2D Center() const noexcept { return (start + end) / 2; }
    [[nodiscard]] Point2D Direction() const noexcept { return end - start; }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        namespace vs = std::views;
        namespace rs = std::ranges;

        auto [min_x, max_x] = rs::minmax(start.x, end.x);
        auto [min_y, max_y] = rs::minmax(start.y, end.y);

        return BoundingBox{.min_x = min_x, .min_y = min_y, .max_x = max_x, .max_y = max_y};
    }

    [[nodiscard]] std::vector<Point2D> Vertices() const noexcept {  // std::reference_wrapper mb
        return {start, end};
    }

    [[nodiscard]] Lines2DDyn Lines(size_t N = 2) const {
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

        auto points_view =
            vs::iota(0u, N) | vs::transform([this, step](size_t i) { return start + step * static_cast<double>(i); });

        rs::for_each(points_view, [&](const Point2D &p) { lines.PushBack(p); });

        return lines;
    }

    /* ваш код здесь */
};

[[nodiscard]] inline Lines2DDyn GenerateOutlineFromVertices(std::span<const Point2D> vertices, size_t N) {
    Lines2DDyn lines{};
    const size_t num_sides = vertices.size();
    if (N == 0 || num_sides == 0) {
        return lines;
    }
    lines.Reserve(N + 1);

    if (num_sides == 1) {
        lines.PushBack(vertices.front());
        return lines;
    }
    
    const size_t points_per_side = N / num_sides;
    namespace rs = std::ranges;
    namespace vs = std::views;

    auto sides = vs::iota(0u, num_sides) | vs::transform([&](size_t i) {
        return Line{vertices[i], vertices[(i + 1) % num_sides]};
    });

    rs::for_each(sides | vs::enumerate, [&](const auto &enumerated_side) {
        auto &&[i, side] = enumerated_side;
        const size_t current_points = (static_cast<size_t>(i) == num_sides - 1) ? (N - (num_sides - 1) * points_per_side) : points_per_side;
        if (current_points == 0) return;

        const Point2D step = (side.end - side.start) / static_cast<double>(current_points);
        auto generated_points = vs::iota(0u, current_points) |
                                vs::transform([=](size_t j) { return side.start + step * static_cast<double>(j); });
        rs::for_each(generated_points, [&](const auto &p) { lines.PushBack(p); });
    });

    lines.PushBack(vertices.front());
    return lines;
}

struct Triangle {
    Point2D a, b, c;

    /* ваш код здесь */
    [[nodiscard]] double Area() const noexcept {
        const Point2D v1 = b - a;
        const Point2D v2 = c - a;
        return 0.5 * std::abs(v1.Cross(v2));
    }
    [[nodiscard]] double Height() const noexcept { return BoundBox().Height(); }
    [[nodiscard]] Point2D Center() const noexcept { return (a + b + c) / 3; }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        namespace rs = std::ranges;

        auto [min_x, max_x] = rs::minmax({a.x, b.x, c.x});
        auto [min_y, max_y] = rs::minmax({a.y, b.y, c.y});

        return BoundingBox{.min_x = min_x, .min_y = min_y, .max_x = max_x, .max_y = max_y};
    }

    [[nodiscard]] std::array<Line, 3> Edges() const noexcept {
        return {{{a, b}, {b, c}, {c, a}}};
    }

    [[nodiscard]] std::vector<Point2D> Vertices() const noexcept {  // std::reference_wrapper mb
        return {a, b, c};
    }

    [[nodiscard]] Lines2DDyn Lines(size_t N = 3) const {
        return GenerateOutlineFromVertices(Vertices(), N);
    }
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    [[nodiscard]] double Area() const noexcept { return std::abs(width * height); }
    [[nodiscard]] double Width() const noexcept { return std::abs(width); }
    [[nodiscard]] double Height() const noexcept { return std::abs(height); }
    
    [[nodiscard]] Point2D Center() const noexcept { return {bottom_left.x + width / 2, bottom_left.y + height / 2}; }
    
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        const auto [min_x, max_x] = std::minmax(bottom_left.x, bottom_left.x + width);
        const auto [min_y, max_y] = std::minmax(bottom_left.y, bottom_left.y + height);
        
        return BoundingBox{.min_x = min_x, .min_y = min_y, .max_x = max_x, .max_y = max_y};
    }

    [[nodiscard]] std::array<Line, 4> Edges() const noexcept {
        const auto verts = Vertices();
        return {{{verts[0], verts[1]}, {verts[1], verts[2]}, {verts[2], verts[3]}, {verts[3], verts[0]}}};
    }

    [[nodiscard]] std::vector<Point2D> Vertices() const noexcept {
        const auto [min_x, max_x] = std::minmax(bottom_left.x, bottom_left.x + width);
        const auto [min_y, max_y] = std::minmax(bottom_left.y, bottom_left.y + height);

        return {
            {min_x, min_y}, 
            {max_x, min_y}, 
            {max_x, max_y}, 
            {min_x, max_y}, 
        }; 
    }

    [[nodiscard]] Lines2DDyn Lines(size_t N = 4) const {
        return GenerateOutlineFromVertices(Vertices(), N);
    }
};


struct Circle {
    Point2D center_p;
    double radius;

    constexpr Circle(Point2D center, double radius) : center_p(center), radius(radius) {}

    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        const double r = std::abs(radius);
        return {center_p.x - r, center_p.y - r, center_p.x + r, center_p.y + r};
    }

    [[nodiscard]] double Height() const noexcept { return 2 * std::abs(radius); }
    
    [[nodiscard]] Point2D Center() const noexcept { return center_p; }

    [[nodiscard]] std::vector<Point2D> Vertices(size_t N = 30) const {
        const double r = std::abs(radius);
        std::vector<Point2D> points;
        if (N == 0)
            return points;
        points.reserve(N);

        namespace vs = std::views;
        namespace rs = std::ranges;

        auto points_view =
            vs::iota(0u, N) | vs::transform([this, N, r](size_t i) {
                const double angle = 2.0 * std::numbers::pi * i / N;
                return Point2D{center_p.x + r * std::cos(angle), center_p.y + r * std::sin(angle)};
            });

        rs::copy(points_view, std::back_inserter(points));
        return points;
    }

    [[nodiscard]] Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn result_lines{};
        const auto verts = Vertices(N); 
        if (verts.empty()) {
            return result_lines;
        }

        result_lines.Reserve(verts.size() + 1);
        for (const auto& p : verts) {
            result_lines.PushBack(p);
        }
        result_lines.PushBack(verts.front());

        return result_lines;
    }

};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {}
    [[nodiscard]] Point2D Center() const noexcept { return center_p; }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        const auto verts = Vertices();
        if (verts.empty()) {
            return {0, 0, 0, 0};
        }

        namespace rs = std::ranges;
        namespace vs = std::views;

        auto x_coords = verts | vs::transform([](const Point2D &p) { return p.x; });
        auto y_coords = verts | vs::transform([](const Point2D &p) { return p.y; });

        const auto [min_x, max_x] = rs::minmax(x_coords);
        const auto [min_y, max_y] = rs::minmax(y_coords);

        return {min_x, min_y, max_x, max_y};
    }
    [[nodiscard]] std::vector<Point2D> Vertices() const {
        return Circle{center_p, radius}.Vertices(std::clamp(sides, 0, std::numeric_limits<int>::max()));
    }

    [[nodiscard]] Lines2DDyn Lines(size_t points_per_side = 10) const { 
        const size_t num_sides = std::clamp(sides, 0, std::numeric_limits<int>::max());
        if (num_sides == 0) return {};
        return GenerateOutlineFromVertices(Vertices(), points_per_side * num_sides);
    }
};


class Polygon {
public:
    explicit Polygon(std::vector<Point2D> points) : points_(std::move(points)) {
        if (points_.empty()) {
            bounding_box_ = {0, 0, 0, 0};
            return;
        }

        namespace vs = std::views;
        namespace rs = std::ranges;

        auto x_coords = points_ | vs::transform([](const Point2D &p) { return p.x; });
        auto y_coords = points_ | vs::transform([](const Point2D &p) { return p.y; });

        const auto [min_x, max_x] = rs::minmax(x_coords);
        const auto [min_y, max_y] = rs::minmax(y_coords);

        bounding_box_ = {.min_x = min_x, .min_y = min_y, .max_x = max_x, .max_y = max_y};
    }

    [[nodiscard]] const std::vector<Point2D> &Vertices() const noexcept { return points_; }

    [[nodiscard]] BoundingBox BoundBox() const noexcept { return bounding_box_; }

    [[nodiscard]] double Height() const noexcept { return bounding_box_.Height(); }

    [[nodiscard]] Point2D Center() const noexcept { return bounding_box_.Center().value_or(Point2D{}); }
    [[nodiscard]] std::vector<Line> Edges() const {
        if (points_.size() < 2) {
            return {};
        }
        namespace rs = std::ranges;
        namespace vs = std::views;
        return vs::iota(0u, points_.size())
                        | vs::transform([this](size_t i) {
                            return Line{points_[i], points_[(i + 1) % points_.size()]};
                        })
                        | rs::to<std::vector>();
    }
    Lines2DDyn Lines(size_t N = 100) const {
        return GenerateOutlineFromVertices(points_, N);
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
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

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
template <>
struct std::formatter<geometry::Shape> {
    constexpr auto parse(std::format_parse_context &ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Shape &shape, FormatContext &ctx) const {
        return std::visit([&ctx](const auto &s) { return std::format_to(ctx.out(), "{}", s); }, shape);
    }
};
