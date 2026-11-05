// SPDX-License-Identifier: LGPL-3.0-or-later
#include "aabb.hpp"

#include "kernel.hpp"
#include "line_segment.hpp"

static bool intersects_line(const AABB& bounding_box, const LineSegment& line_segment)
{
    const Point base = line_segment.p1;
    const Vector dir = line_segment.p2 - line_segment.p1;
    const Vector n = Vector{dir.y(), -dir.x()};

    const Vector c1 = bounding_box.bottom_left() - base;
    const Vector c2 = bounding_box.top_right() - base;
    const Vector c3 = bounding_box.bottom_right() - base;
    const Vector c4 = bounding_box.top_left() - base;

    const double dp1 = n.scalar_product(c1);
    const double dp2 = n.scalar_product(c2);
    const double dp3 = n.scalar_product(c3);
    const double dp4 = n.scalar_product(c4);

    return (dp1 * dp2 <= 0.) || (dp2 * dp3 <= 0.) || (dp3 * dp4 <= 0.);
}

bool AABB::intersects(const LineSegment& line_segment) const
{
    if(!intersects_line(*this, line_segment)) {
        return false;
    }

    const AABB bb_line_segment({line_segment.p1, line_segment.p2});

    return this->overlap(bb_line_segment);
}
