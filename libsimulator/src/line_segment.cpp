// SPDX-License-Identifier: LGPL-3.0-or-later
#include "line_segment.hpp"

#include "kernel.hpp"
#include "macros.hpp"

#include <CGAL/Distance_2/Point_2_Segment_2.h>

#include <cmath>
#include <utility>

LineSegment::LineSegment(Point p1, Point p2) : p1(std::move(p1)), p2(std::move(p2))
{
}

bool LineSegment::operator==(const LineSegment& other) const
{
    return p1 == other.p1 && p2 == other.p2;
}

bool LineSegment::operator!=(const LineSegment& other) const
{
    return !(*this == other);
}

bool LineSegment::operator<(const LineSegment& other) const
{
    if(p1 < other.p1)
        return true;
    if((p1 == other.p1) && (p2 < other.p2))
        return true;
    return false;
}

Vector LineSegment::normal_vec() const
{
    const Vector r = (p2 - p1);
    return Vector(-r.y(), r.x()).normalized();
}

double LineSegment::normal_comp(const Vector& v) const
{
    // Normierte Vectoren
    Vector l = (p2 - p1).normalized();
    const Vector& n = normal_vec();

    double alpha;

    if(fabs(l.x()) < j_eps) {
        alpha = v.x() / n.x();
    } else if(fabs(l.y()) < j_eps) {
        alpha = v.y() / n.y();
    } else {
        alpha = l.cross_product(v) / n.cross_product(l);
    }

    return fabs(alpha);
}

Point LineSegment::shortest_point(const Point& p) const
{
    if(p1 == p2)
        return p1;

    const Vector& t = p1 - p2;
    double lambda = (p - p2).scalar_product(t) / t.scalar_product(t);
    if(lambda < 0)
        return p2;
    else if(lambda > 1)
        return p1;
    else
        return p2 + t * lambda;
}

double LineSegment::dist_to(const Point& p) const
{
    return sqrt(CGAL::squared_distance(p.cgal(), K::Segment_2(p1.cgal(), p2.cgal())));
}

double LineSegment::length_square() const
{
    return (p1 - p2).norm_square();
}
