// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "geometry/location.hpp"
#include "point.hpp"

#include <fmt/format.h>

#include <cstddef>
#include <string>

class FloorfieldRoutingEngine;

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

private:
    friend std::string format_as(const Destination& d)
    {
        return fmt::format("destination {}", d._id);
    }
};
