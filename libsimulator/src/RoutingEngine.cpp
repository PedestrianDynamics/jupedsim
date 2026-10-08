// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

#include "Visitor.hpp"

#include <variant>

RoutingEngine::RoutingEngine(const Geometry& geometry)
    : _floorfield(geometry), _shortestPath(geometry)
{
}

Destination RoutingEngine::RegisterDestination(const std::vector<AreaPiece>& pieces)
{
    return _floorfield.RegisterDestination(pieces);
}

Point RoutingEngine::GetOrientation(const Location& from, const RoutingTarget& to)
{
    return std::visit(
        overloaded{
            [&](const Location& place) { return _shortestPath.GetOrientation(from, place); },
            [&](const Destination& d) { return d.orientation(from); }},
        to);
}
