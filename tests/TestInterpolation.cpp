#include <gtest/gtest.h>
#include <vector>
#include <array>
#include "interpolation.hpp"  // your interpolation template

// ----------- linear interpolation tests -----------

TEST(InterpolationTest, LinearScalar) {
    double start = 0.0;
    double end = 10.0;
    double factor = 0.5;

    double result = Interpolate<1>::linear(factor, start, end);
    EXPECT_DOUBLE_EQ(result, 5.0);
}

TEST(InterpolationTest, CubicScalar) {
    double start = 0.0;
    double end = 10.0;
    double factor = 0.5;

    double result = Interpolate<1>::cubic(factor, start, end);
    EXPECT_NEAR(result, 5.0, 1e-9);
}

// ----------- vector interpolation tests -----------

TEST(InterpolationTest, LinearVector) {
    std::vector<std::array<double, 2>> vec = {{0.0, 0.0}, {10.0, 20.0}};
    double factor = 0.5;

    std::array<double, 2> result = Interpolate<2>::linear(factor, vec);

    EXPECT_DOUBLE_EQ(result[0], 5.0);
    EXPECT_DOUBLE_EQ(result[1], 10.0);
}