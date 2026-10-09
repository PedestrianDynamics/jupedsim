// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Destination.hpp"

#include "FloorfieldRoutingEngine.hpp"

bool Destination::contains(const Location& where) const
{
    return _engine->contains(where, _id);
}

Point Destination::orientation(const Location& from) const
{
    return _engine->get_orientation(from, _id);
}
