#pragma once

#include "geometry/Point.hpp"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/join.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Intersections_2/Line_2_Segment_2.h>
#include <CGAL/Intersections_3/Iso_cuboid_3_Ray_3.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <vector>

template <typename BackingType>
class CGALPolygon
{
    BackingType _poly;

public:
    /// Do not use the constructor directly, use the PolygonBuilder to create an instance of this
    /// type.
    explicit CGALPolygon(BackingType p) : _poly(std::move(p)) {}
    /// Users of this type shall not call this function. This is only used by associated code during
    /// construction.
    const BackingType& GetBacking() const;
};

using CGALPolygonWithHoles =
    CGALPolygon<CGAL::Polygon_with_holes_2<CGAL::Exact_predicates_exact_constructions_kernel>>;
using CGALPolygonWithoutHoles =
    CGALPolygon<CGAL::Polygon_2<CGAL::Exact_predicates_exact_constructions_kernel>>;

class CGALPolygonBuilder
{
    using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
    using Poly = CGAL::Polygon_2<Kernel>;
    using PolyWithHoles = CGAL::Polygon_with_holes_2<Kernel>;
    using PolyWithHolesList = std::list<PolyWithHoles>;
    std::vector<Point> _points;
    std::vector<CGALPolygonWithoutHoles> _holes;

public:
    CGALPolygonWithHoles Build();
    CGALPolygonBuilder& AddPoint(Point p);
    CGALPolygonBuilder& AddHole(CGALPolygonWithoutHoles p);
};
