// SPDX-License-Identifier: LGPL-3.0-or-later
#include "destination.hpp"

#include "floorfield_routing_engine.hpp"

bool Destination::contains(const Location& where) const
{
    return _engine->contains(where, _id);
}

Point Destination::orientation(const Location& from) const
{
    return _engine->get_orientation(from, _id);
}
