#include <gtest/gtest.h>
#include "triangulation.hpp"

using namespace geometry;
using namespace geometry::triangulation;

class TriangulationTest : public ::testing::Test {};

TEST_F(TriangulationTest, DelaunayTriangulationBasic) {
    std::vector<Point2D> points = {{0,0}, {100,0}, {100,100}, {0,100}};
    auto triangulation_res = DelaunayTriangulation(points);

    ASSERT_TRUE(triangulation_res.has_value());
    
    auto triangles = triangulation_res.value();
    ASSERT_EQ(triangles.size(), 2);
}