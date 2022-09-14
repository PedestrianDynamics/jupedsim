/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CGALPolygon.hpp"

CGALPolygonBuilder& CGALPolygonBuilder::AddPoint(Point p)
{
    _points.emplace_back(p);
    return *this;
}

CGALPolygonBuilder& CGALPolygonBuilder::AddHole(CGALPolygonWithoutHoles p)
{
    _holes.emplace_back(std::move(p));
    return *this;
}

CGALPolygonWithHoles CGALPolygonBuilder::Build()
{
    Poly poly;
    for(const auto& p : _points) {
        poly.push_back({p.x, p.y});
    }
    PolyWithHoles poly_with_holes{poly};
    for(const auto& hole : _holes) {
        poly_with_holes.add_hole(hole.GetBacking());
    }
    return CGALPolygonWithHoles(poly_with_holes);
}
