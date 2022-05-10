#pragma once

/// Computes the cross product 2D of vectors apex->a (A) and apex->b (B)
/// Geometrically the absolute value of the cross product in 2d is also the area of the
/// parallelogram of A and B. The sign of the cross product determines the relation of A and B.
/// If AxB < 0 A is left of B
/// if AxB > 0 A is right of B
/// otherwise A and B are parallel
#include <numeric>
template <typename PointType>
inline auto triarea2d(PointType apex, PointType a, PointType b)
{
    const float ax = a.x - apex.x;
    const float ay = a.y - apex.y;
    const float bx = b.x - apex.x;
    const float by = b.y - apex.y;
    return bx * ay - ax * by;
}
