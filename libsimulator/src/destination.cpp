// SPDX-License-Identifier: LGPL-3.0-or-later
#include "destination.hpp"

#include "floorfield_routing_engine.hpp"
#include "geometry/location.hpp"
#include "point.hpp"

#include <cstddef>

bool Destination::contains(const Location& where) const
{
    return _engine->contains(where, _id);
}

Point Destination::orientation(const Location& from) const
{
    return _engine->get_orientation(from, _id);
}

TravelTimeGrid Destination::travel_times(std::size_t region) const
{
    return _engine->travel_times(region, _id);
}
