/**
 * @file geometry.cpp
 * @author perry chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2025-02-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef GEOMETRY_HPP_
    #define GEOMETRY_HPP_

#include "type.hpp"
#include <cmath>
#include <limits>

constexpr double epsilon = 10 * std::numeric_limits<double>::epsilon();

/**
 * @brief give the cross product of two vectors
 * 
 * @param a 
 * @param b 
 * @return double 
 */
inline double cross_product(Vector2f a, Vector2f b) {
    //cross product formula
    return a.x * b.y - a.y * b.x;
}

/**
 * @brief calculate if a point is inside a triangle
 * 
 * @param P 
 * @param A 
 * @param B 
 * @param C 
 * @return true 
 * @return false 
 */
inline bool is_inside_triangle(Vector2f P, Vector2f A, Vector2f B, Vector2f C) {
    //To calculate the area of a triangle given three points (x, y), you can use the formula for the area of a triangle formed by three vertices (x1, y1), (x2, y2), and (x3, y3):
    //Area = 0.5 * |(x1(y2 - y3) + x2(y3 - y1) + x3(y1 - y2))|

    const double abc = 0.5 * abs(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));

    const double abp = 0.5 * abs(A.x * (B.y - P.y) + B.x * (P.y - A.y) + P.x * (A.y - B.y));
    const double bcp = 0.5 * abs(B.x * (C.y - P.y) + C.x * (P.y - B.y) + P.x * (B.y - C.y));
    const double cap = 0.5 * abs(C.x * (A.y - P.y) + A.x * (P.y - C.y) + P.x * (C.y - A.y));

    //if sum of abp, bcp, cap is equal to abc, then the point is inside the triangle, otherwise it's outside.
    return std::abs(abc - (abp + bcp + cap)) < epsilon;
}

/**
 * @brief check if the angle is convex or concave
 * 
 * @param A 
 * @param B 
 * @param C 
 * @param clockwise 
 * @return true 
 * @return false 
 * 
 * Polygon order     | cross < 0 | cross > 0 | cross = 0
 * Clockwise         | Convex    | Concave   | Straight
 * Counter-clockwise | Concave   | Convex    | Straight
 * 
 */
inline bool is_convex(Vector2f A, Vector2f B, Vector2f C, bool clockwise = true) {
    //calculate the vector from B to A and B to C
    Vector2f BA = {A.x - B.x, A.y - B.y};
    Vector2f BC = {C.x - B.x, C.y - B.y};
    //calculate the cross product of BA and BC
    double cross = cross_product(BA, BC);
        
    // 1. if it's a clockwise direction, then the cross product should be negative to be a convex angle
    // 2. if it's a counter clockwise direction, then the cross product should be positive to be a convex angle
    // 180 degree in this case is an exception, it's neither convex nor concave, but we can consider it as concave.
    if (std::abs(cross) < epsilon) 
        return false; // straight line → neither convex nor concave
    else if (clockwise) {
        return cross < 0; // is convex 
    } else {
        return cross > 0;
    }

    return clockwise ? (cross < 0) : (cross > 0);
}

/**
 * @brief check if the angle is concave
 * 
 * @param A 
 * @param B 
 * @param C 
 * @param clockwise 
 * @return true 
 * @return false 
 * 
 * 
 * Polygon order     | cross < 0 | cross > 0 | cross = 0
 * Clockwise         | Convex    | Concave   | Straight
 * Counter-clockwise | Concave   | Convex    | Straight
 *
 */
inline bool is_concave(Vector2f A, Vector2f B, Vector2f C, bool clockwise = true) {
    //if it's not convex, then it's concave
    //prefere to inverse clockwise rather than inverse the result of is_convex to handle the 180 degree case
    return is_convex(A, B, C, !clockwise);
}

#endif /* !GEOMETRY_HPP_ */