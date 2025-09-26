#include <gtest/gtest.h>
#include "geometry.hpp"

// Use a simple alias for convenience
using Vec2 = Vector2f;

// ----------- cross product tests -----------

TEST(GeometryTest, CrossProduct) {
    Vec2 a{1.0, 0.0};
    Vec2 b{0.0, 1.0};
    double result = cross_product(a, b);
    EXPECT_DOUBLE_EQ(result, 1.0);

    Vec2 c{-1.0, 0.0};
    result = cross_product(c, b);
    EXPECT_DOUBLE_EQ(result, -1.0);
}

// ----------- point in triangle tests -----------

TEST(GeometryTest, PointInsideTriangle) {
    Vec2 A{0,0};
    Vec2 B{1,0};
    Vec2 C{0,1};
    Vec2 P_inside{0.25,0.25};
    Vec2 P_outside{1.0,1.0};

    EXPECT_TRUE(is_inside_triangle(P_inside, A, B, C));
    EXPECT_FALSE(is_inside_triangle(P_outside, A, B, C));
}

// ----------- convex / concave tests -----------

TEST(GeometryTest, ConvexAngle) {
    Vec2 A{0,0};
    Vec2 B{1,0};
    Vec2 C{1,1};

    EXPECT_TRUE(is_convex(A,B,C,true));
    EXPECT_FALSE(is_concave(A,B,C,true));

    EXPECT_FALSE(is_convex(A,B,C,false));
    EXPECT_TRUE(is_concave(A,B,C,false));
}

TEST(GeometryTest, StraightAngleEdgeCase) {
    Vec2 A{0,0};
    Vec2 B{1,0};
    Vec2 C{2,0};

    // Straight line, treat as concave per your function
    EXPECT_FALSE(is_convex(A,B,C,true));
    EXPECT_FALSE(is_concave(A,B,C,true));
}