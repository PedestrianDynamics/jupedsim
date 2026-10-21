// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "floorfield_cxx/lib.h"
#include "geometry/area_piece.hpp"
#include "geometry/location.hpp"
#include "point.hpp"
#include "rust/cxx.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class Geometry;

struct FloorfieldOptions {
    double cell_size = 0.2; // m
    // The speed field value is linearly decreased from this influence radius (in meter) to the
    // boundary. Reducing the speed field near walls has the effect that agents are not routed
    // unphysically close to the wall.
    double wall_influence_radius = 0.5;
};

/// Travel times and walking directions to one destination over one region's floor field grid.
/// All regions share one lattice, so grids of neighbouring regions line up cell for cell.
struct TravelTimeGrid {
    /// Lower-left corner of cell (0, 0); cell (col, row) is centred at
    /// origin + ((col + 0.5), (row + 0.5)) * cell_size.
    Point origin;
    std::uint32_t width;
    std::uint32_t height;
    double cell_size;
    /// Row-major, height * width values in seconds: zero inside the destination, infinite where
    /// the cell is not walkable.
    std::vector<double> values;
    /// Row-major, height * width unit vectors: the direction an agent at the cell's centre is
    /// sent along. Zero inside the destination, NaN where the cell is not walkable.
    std::vector<Point> directions;
};

/// Routes to registered areas along static floor fields over the region graph.
class FloorfieldRoutingEngine
{
public:
    /// Throws if @p geometry has no region graph, i.e. was built from a raw mesh. The engine
    /// keeps no reference to @p geometry.
    explicit FloorfieldRoutingEngine(const Geometry& geometry, FloorfieldOptions options);

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
    /// Travel times and directions to destination @p id over the grid of @p region. Solves the
    /// destination on first use.
    TravelTimeGrid travel_times(std::size_t region, std::size_t id);

private:
    rust::Box<jupedsim::floorfield::MultiRegionFloorfield> _field;
};