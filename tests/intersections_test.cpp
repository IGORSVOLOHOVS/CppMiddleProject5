#include <gtest/gtest.h>
#include "intersections.hpp"

using namespace geometry;
using namespace geometry::intersections;

class IntersectionTest : public ::testing::Test {
protected:
    Circle c1_{{0, 0}, 5};
    Circle c2_{{8, 0}, 5};
    Circle c3_{{20, 20}, 1};
    Line l1_{{-10, 0}, {10, 0}};
    Line l2_{{0, -10}, {0, 10}};
    Line l3_{{20, 20}, {30, 30}};
};

TEST_F(IntersectionTest, LineLineIntersection) {
    auto intersection_point = GetIntersectPoint(l1_, l2_);
    ASSERT_TRUE(intersection_point.has_value());
    EXPECT_NEAR(intersection_point->x, 0.0, 1e-9);
    EXPECT_NEAR(intersection_point->y, 0.0, 1e-9);

    auto no_intersection = GetIntersectPoint(l1_, l3_);
    EXPECT_FALSE(no_intersection.has_value());
}

TEST_F(IntersectionTest, CircleCircleIntersection) {
    auto intersection_point = GetIntersectPoint(c1_, c2_);
    ASSERT_TRUE(intersection_point.has_value());
    EXPECT_NEAR(intersection_point->x, 4.0, 1e-9);
    EXPECT_NEAR(std::abs(intersection_point->y), 3.0, 1e-9);

    auto no_intersection = GetIntersectPoint(c1_, c3_);
    EXPECT_FALSE(no_intersection.has_value());
}

TEST_F(IntersectionTest, LineCircleIntersection) {
    auto intersection_point = GetIntersectPoint(l1_, c1_);
    ASSERT_TRUE(intersection_point.has_value());
    EXPECT_NEAR(intersection_point->Length(), 5.0, 1e-9);

    auto no_intersection = GetIntersectPoint(l3_, c1_);
    EXPECT_FALSE(no_intersection.has_value());
}

TEST_F(IntersectionTest, UnsupportedIntersection) {
    Rectangle r{{0,0}, 1, 1};
    EXPECT_THROW(GetIntersectPoint(c1_, r), std::logic_error);
}