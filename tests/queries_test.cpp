#include <gtest/gtest.h>
#include "queries.hpp"

using namespace geometry;
using namespace geometry::queries;

class QueriesTest : public ::testing::Test {
protected:
    Circle circle_{{10, 0}, 5};
    Line line_{{0, 0}, {5, 0}};
    Rectangle rect_{{10, 10}, 20, 10}; 
    Triangle tri_{{0,0}, {10,0}, {5, 10}};

    Point2D point_near_circle_{16, 0};
    Point2D point_on_line_{2, 0};
    Point2D point_near_rect_{35, 15};
    Point2D point_on_tri_edge_{5, 0};

    Shape shape_circle_ = circle_;
    Shape shape_line_ = line_;
    Shape shape_rect_ = rect_;
    Shape shape_tri_ = tri_;
};

TEST_F(QueriesTest, DistanceToPoint) {
    double dist_to_circle = DistanceToPoint(shape_circle_, point_near_circle_);
    EXPECT_NEAR(dist_to_circle, 1.0, 1e-9);

    double dist_to_line = DistanceToPoint(shape_line_, point_on_line_);
    EXPECT_NEAR(dist_to_line, 0.0, 1e-9);

    double dist_to_rect = DistanceToPoint(shape_rect_, point_near_rect_);
    EXPECT_NEAR(dist_to_rect, 5.0, 1e-9); // Расстояние до правого края (30,15)

    double dist_to_tri = DistanceToPoint(shape_tri_, point_on_tri_edge_);
    EXPECT_NEAR(dist_to_tri, 0.0, 1e-9);
}

TEST_F(QueriesTest, GetBoundBox) {
    BoundingBox bbox_c = GetBoundBox(shape_circle_);
    EXPECT_NEAR(bbox_c.min_x, 5.0, 1e-9);
    EXPECT_NEAR(bbox_c.max_x, 15.0, 1e-9);
    EXPECT_NEAR(bbox_c.min_y, -5.0, 1e-9);
    EXPECT_NEAR(bbox_c.max_y, 5.0, 1e-9);

    BoundingBox bbox_r = GetBoundBox(shape_rect_);
    EXPECT_NEAR(bbox_r.min_x, 10.0, 1e-9);
    EXPECT_NEAR(bbox_r.max_x, 30.0, 1e-9);
}

TEST_F(QueriesTest, GetHeight) {
    double height_c = GetHeight(shape_circle_);
    EXPECT_NEAR(height_c, 10.0, 1e-9);

    double height_r = GetHeight(shape_rect_);
    EXPECT_NEAR(height_r, 10.0, 1e-9);
}

TEST_F(QueriesTest, BoundingBoxesOverlap) {
    Circle c1{{0,0}, 2};
    Circle c2{{3,0}, 2}; 
    Circle c3{{5,0}, 2}; 
    Shape s1 = c1, s2 = c2, s3 = c3;

    EXPECT_TRUE(BoundingBoxesOverlap(s1, s2));
    EXPECT_FALSE(BoundingBoxesOverlap(s1, s3));
}