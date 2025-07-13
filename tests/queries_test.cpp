#include <gtest/gtest.h>
#include "queries.hpp"


using namespace geometry;
using namespace geometry::queries;

class QueriesTest : public ::testing::Test {
protected:
    Circle circle_{{10, 0}, 5};
    Line line_{{0, 0}, {5, 0}};
    Point2D point_near_circle_{16, 0};
    Point2D point_on_line_{2, 0};
    Shape shape_circle_ = circle_;
    Shape shape_line_ = line_;
};

TEST_F(QueriesTest, DISABLED_DistanceToPoint) {
    double dist_to_circle = DistanceToPoint(shape_circle_, point_near_circle_);
    EXPECT_NEAR(dist_to_circle, 1.0, 1e-9);

    double dist_to_line = DistanceToPoint(shape_line_, point_on_line_);
    EXPECT_NEAR(dist_to_line, 0.0, 1e-9);
}

TEST_F(QueriesTest, DISABLED_GetBoundBox) {
    BoundingBox bbox = GetBoundBox(shape_circle_);
    EXPECT_NEAR(bbox.min_x, 5.0, 1e-9);
    EXPECT_NEAR(bbox.max_x, 15.0, 1e-9);
}

TEST_F(QueriesTest, DISABLED_GetHeight) {
    double height = GetHeight(shape_circle_);
    EXPECT_NEAR(height, 10.0, 1e-9);
}

TEST_F(QueriesTest, DISABLED_BoundingBoxesOverlap) {
    Circle c1{{0,0}, 2};
    Circle c2{{3,0}, 2};
    Circle c3{{5,0}, 2};
    Shape s1 = c1, s2 = c2, s3 = c3;

    EXPECT_TRUE(BoundingBoxesOverlap(s1, s2));
    EXPECT_FALSE(BoundingBoxesOverlap(s1, s3));
}