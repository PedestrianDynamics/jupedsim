// SPDX-License-Identifier: LGPL-3.0-or-later
#include "routing_engine.hpp"

#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "point.hpp"
#include "routing_target.hpp"
#include "visitor.hpp"

#include <variant>
#include <vector>

RoutingEngine::RoutingEngine(const Geometry& geometry, FloorfieldOptions options)
    : _floorfield(geometry, options), _shortest_path(geometry)
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
