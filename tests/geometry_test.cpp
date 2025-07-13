#include <gtest/gtest.h>
#include "geometry.hpp"

using namespace geometry;

class GeometryTest : public ::testing::Test {
protected:
    Point2D p1_{1.0, 2.0};
    Point2D p2_{4.0, 6.0};
    Line line_{p1_, p2_};
    Circle circle_{{0.0, 0.0}, 5.0};
    Rectangle rect_{{0.0, 0.0}, 10.0, 5.0};
};

TEST_F(GeometryTest, Point2DOperations) {
    Point2D p_sum = p1_ + p2_;
    EXPECT_NEAR(p_sum.x, 5.0, 1e-9);
    EXPECT_NEAR(p_sum.y, 8.0, 1e-9);

    Point2D p_diff = p2_ - p1_;
    EXPECT_NEAR(p_diff.x, 3.0, 1e-9);
    EXPECT_NEAR(p_diff.y, 4.0, 1e-9);

    EXPECT_NEAR(p1_.DistanceTo(p2_), 5.0, 1e-9);
}

TEST_F(GeometryTest, LineProperties) {
    EXPECT_NEAR(line_.Length(), 5.0, 1e-9);
    auto bbox = line_.BoundBox();
    EXPECT_NEAR(bbox.min_x, 1.0, 1e-9);
    EXPECT_NEAR(bbox.min_y, 2.0, 1e-9);
    EXPECT_NEAR(bbox.max_x, 4.0, 1e-9);
    EXPECT_NEAR(bbox.max_y, 6.0, 1e-9);
}

TEST_F(GeometryTest, RectangleProperties) {
    EXPECT_NEAR(rect_.Area(), 50.0, 1e-9);
    Point2D center = rect_.Center();
    EXPECT_NEAR(center.x, 5.0, 1e-9);
    EXPECT_NEAR(center.y, 2.5, 1e-9);
    auto vertices = rect_.Vertices();
    ASSERT_EQ(vertices.size(), 4);
}

TEST_F(GeometryTest, CircleProperties) {
    auto bbox = circle_.BoundBox();
    EXPECT_NEAR(bbox.min_x, -5.0, 1e-9);
    EXPECT_NEAR(bbox.max_x, 5.0, 1e-9);
    EXPECT_NEAR(bbox.Height(), 10.0, 1e-9);
    auto vertices = circle_.Vertices(30);
    ASSERT_EQ(vertices.size(), 30);
}

TEST_F(GeometryTest, PolygonProperties) {
    std::vector<Point2D> points = {{0,0}, {10,0}, {10,10}, {0,10}};
    Polygon poly(points);
    
    auto bbox = poly.BoundBox();
    EXPECT_NEAR(bbox.min_x, 0, 1e-9);
    EXPECT_NEAR(bbox.max_x, 10, 1e-9);
    EXPECT_NEAR(poly.Height(), 10, 1e-9);

    Point2D center = poly.Center();
    EXPECT_NEAR(center.x, 5.0, 1e-9);
    EXPECT_NEAR(center.y, 5.0, 1e-9);
}