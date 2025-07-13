#include <gtest/gtest.h>
#include "shape_utils.hpp"
#include "queries.hpp"

using namespace geometry;
using namespace geometry::utils;

class ShapeUtilsTest : public ::testing::Test {
protected:
    Circle c1{{0,0}, 2}; 
    Rectangle r1{{10, 10}, 5, 10};
    Line l1{{-5, -5}, {5, 5}}; 

    std::vector<Shape> shapes_{c1, r1, l1};
};

TEST_F(ShapeUtilsTest, DISABLED_FindAllCollisions) {
    Circle c2{{1,0}, 2};
    std::vector<Shape> collision_shapes = {c1, c2, r1};
    auto collisions = FindAllCollisions(collision_shapes);
    ASSERT_EQ(collisions.size(), 1);
}

TEST_F(ShapeUtilsTest, DISABLED_FindHighestShape) {
    auto highest_idx = FindHighestShape(shapes_);
    ASSERT_TRUE(highest_idx.has_value());
    EXPECT_EQ(*highest_idx, 1);
    
    std::vector<Shape> empty_shapes;
    auto no_highest = FindHighestShape(empty_shapes);
    EXPECT_FALSE(no_highest.has_value());
}