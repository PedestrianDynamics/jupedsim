// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/location.hpp"
#include "point.hpp"

#include <fmt/format.h>

#include <cstddef>
#include <string>
#include <vector>

class FloorfieldRoutingEngine;
struct SpeedGrid;
struct TravelTimeGrid;

/// An area agents are routed to. The engine that registered it has to outlive it.
class Destination
{
    FloorfieldRoutingEngine* _engine;
    std::size_t _id;

public:
    Destination(FloorfieldRoutingEngine& engine, std::size_t id) : _engine(&engine), _id(id) {}

    /// Whether @p where lies in the destination, to the engine's resolution.
    bool contains(const Location& where) const;

    /// Unit vector from @p from along the route to the destination. Zero once inside it.
    Point orientation(const Location& from) const;

    /// Travel times to the destination over the grid of @p region.
    TravelTimeGrid travel_times(std::size_t region) const;

    /// The speed field the destination is routed with in @p region: its override there, else the
    /// region's default.
    SpeedGrid speed(std::size_t region) const;
    bool has_speed_override(std::size_t region) const;
    /// Routes to the destination with @p values in @p region instead of the default, see
    /// FloorfieldRoutingEngine::set_speed_override.
    void set_speed_override(std::size_t region, const std::vector<double>& values) const;
    /// Routes to the destination with the default speed field of @p region again.
    void clear_speed_override(std::size_t region) const;

private:
    friend std::string format_as(const Destination& d)
    {
        return fmt::format("destination {}", d._id);
    }
};
