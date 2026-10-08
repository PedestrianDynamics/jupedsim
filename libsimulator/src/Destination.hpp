// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Geometry/Location.hpp"
#include "Point.hpp"

#include <cstddef>

class FloorfieldRoutingEngine;

/// An area agents are routed to. The engine that registered it has to outlive it.
class Destination
{
    FloorfieldRoutingEngine* _engine;
    std::size_t _id;

public:
    Destination(FloorfieldRoutingEngine& engine, std::size_t id) : _engine(&engine), _id(id) {}

    /// Unit vector from @p from along the route to the destination. Zero once inside it.
    Point orientation(const Location& from) const;
};
