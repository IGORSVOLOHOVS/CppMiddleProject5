#include <gtest/gtest.h>
#include "convex_hull.hpp"
#include <vector>

using namespace geometry;
using namespace geometry::convex_hull;

class ConvexHullTest : public ::testing::Test {};

TEST_F(ConvexHullTest, GrahamScanSquare) {
    std::vector<Point2D> points = {{0,0}, {10,0}, {10,10}, {0,10}, {5,5}};
    auto hull_res = GrahamScan(points);

    ASSERT_TRUE(hull_res.has_value());
    auto hull = hull_res.value();
    
    ASSERT_EQ(hull.size(), 4);
}

TEST_F(ConvexHullTest, GrahamScanInsufficientPoints) {
    std::vector<Point2D> points = {{0,0}, {10,0}};
    auto hull_res = GrahamScan(points);

    ASSERT_FALSE(hull_res.has_value());
    EXPECT_EQ(hull_res.error(), GeometryError::InsufficientPoints);
}