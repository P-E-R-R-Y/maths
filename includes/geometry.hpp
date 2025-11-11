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

#include "Type.hpp"
#include <cmath>
#include <limits>

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
    double cross = BA.cross(BC);
        
    // 1. if it's a clockwise direction, then the cross product should be negative to be a convex angle
    // 2. if it's a counter clockwise direction, then the cross product should be positive to be a convex angle
    // 180 degree in this case is an exception, it's neither convex nor concave, but we can consider it as concave.
    if (std::abs(cross) < epsilon<double>()) 
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