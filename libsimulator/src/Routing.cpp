// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Routing.hpp"

#include "CfgCgal.hpp"
#include "Unreachable.hpp"

#include <memory.h>

distance::Point<double> cvt_pt(const Poly::Point_2& p)
{
    return distance::Point<double>{CGAL::to_double(p.x()), CGAL::to_double(p.y())};
};

distance::Polygon<double> cvt_poly(const Poly& cgalContainer)
{
    distance::Polygon<double> poly;
    poly.points.reserve(cgalContainer.size());
    std::transform(
        std::begin(cgalContainer),
        std::end(cgalContainer),
        std::back_inserter(poly.points),
        cvt_pt);
    return poly;
};

Routing::Routing(std::unique_ptr<CollisionGeometry> geometry)
    : _geometry(std::move(geometry))
    , _routingEngine(std::make_unique<RoutingEngine>(_geometry->Polygon()))
{
}

void Routing::AddDistanceMapForStage(const BaseStage::ID id, const Point& p)
{
    auto builder = PrepareDistanceMapBuilder();
    builder->AddExitLine({{p.x, p.y}, {p.x, p.y}});
    _distanceMaps.emplace(id, builder->Build());
    // HACK(kkratz):
    // distance::DumpDistanceMapMatplotlibCSV(*_distanceMaps[id], id.getID());
}

void Routing::AddDistanceMapForStage(const BaseStage::ID id, const Polygon& p)
{
    auto builder = PrepareDistanceMapBuilder();
    builder->AddExitPolygon(cvt_poly(p));
    _distanceMaps.emplace(id, builder->Build());
    // HACK(kkratz):
    // distance::DumpDistanceMapMatplotlibCSV(*_distanceMaps[id], id.getID());
}

std::unique_ptr<Routing::DMapBuilder> Routing::PrepareDistanceMapBuilder() const
{
    auto builder = std::make_unique<DMapBuilder>();

    const auto poly = _geometry->Polygon();
    builder->AddOutlinePolygon(cvt_poly(poly.outer_boundary()));
    for(const auto& hole : poly.holes()) {
        builder->AddFilledPolygon(cvt_poly(hole));
    }

    return builder;
}

Point Routing::ComputeWaypoint(Point currentPosition, BaseStage::Location destination) const
{
    return std::visit(
        overloaded{
            [this, currentPosition](const Point& destination) {
                return _routingEngine->ComputeWaypoint(currentPosition, destination);
            },
            [this, currentPosition](const BaseStage::ID id) {
                const auto val =
                    _distanceMaps.at(id)->GetNextTarget({currentPosition.x, currentPosition.y});
                return Point{val.x, val.y};
            }},
        destination);
}
