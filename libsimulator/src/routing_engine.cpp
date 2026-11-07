// SPDX-License-Identifier: LGPL-3.0-or-later
#include "routing_engine.hpp"

#include "destination.hpp"
#include "floorfield_routing_engine.hpp"
#include "geometry/area_piece.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "kernel.hpp"
#include "routing_target.hpp"
#include "visitor.hpp"

#include <cstddef>
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

GridInfo RoutingEngine::grid_info(std::size_t region) const
{
    return _floorfield.grid_info(region);
}

SpeedGrid RoutingEngine::default_speed(std::size_t region) const
{
    return _floorfield.default_speed(region);
}

void RoutingEngine::set_default_speed(std::size_t region, const std::vector<double>& values)
{
    _floorfield.set_default_speed(region, values);
}

Vector RoutingEngine::get_orientation(const Location& from, const RoutingTarget& to)
{
    return std::visit(
        Overloaded{
            [&](const Location& place) { return _shortest_path.get_orientation(from, place); },
            [&](const Destination& d) { return d.orientation(from); }},
        to);
}

double RoutingEngine::travel_time(const Location& from, const Destination& to) const
{
    return to.travel_time(from);
}
