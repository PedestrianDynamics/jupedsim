// SPDX-License-Identifier: LGPL-3.0-or-later
#include "routing_engine.hpp"

#include "visitor.hpp"

#include <variant>

RoutingEngine::RoutingEngine(const Geometry& geometry)
    : _floorfield(geometry), _shortest_path(geometry)
{
}

Destination RoutingEngine::register_destination(const std::vector<AreaPiece>& pieces)
{
    return _floorfield.register_destination(pieces);
}

Point RoutingEngine::get_orientation(const Location& from, const RoutingTarget& to)
{
    return std::visit(
        Overloaded{
            [&](const Location& place) { return _shortest_path.get_orientation(from, place); },
            [&](const Destination& d) { return d.orientation(from); }},
        to);
}
