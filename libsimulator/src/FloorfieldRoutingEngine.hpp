// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Destination.hpp"
#include "Geometry/AreaPiece.hpp"
#include "Geometry/Location.hpp"
#include "Point.hpp"
#include "floorfield_cxx/lib.h"
#include "rust/cxx.h"

#include <cstddef>
#include <vector>

class Geometry;

/// Routes to registered areas along static floor fields over the region graph.
class FloorfieldRoutingEngine
{
public:
    /// Throws if @p geometry has no region graph, i.e. was built from a raw mesh. The engine
    /// keeps no reference to @p geometry.
    explicit FloorfieldRoutingEngine(const Geometry& geometry);

    // Destinations point at their engine.
    FloorfieldRoutingEngine(const FloorfieldRoutingEngine&) = delete;
    FloorfieldRoutingEngine& operator=(const FloorfieldRoutingEngine&) = delete;
    FloorfieldRoutingEngine(FloorfieldRoutingEngine&&) = delete;
    FloorfieldRoutingEngine& operator=(FloorfieldRoutingEngine&&) = delete;

    /// Registers @p pieces as one destination. Routing heads for whichever piece is nearest in
    /// travel time.
    Destination register_destination(const std::vector<AreaPiece>& pieces);

    bool contains(const Location& where, std::size_t id);
    Point get_orientation(const Location& from, std::size_t id);

private:
    rust::Box<jupedsim::floorfield::MultiRegionFloorfield> _field;
};
