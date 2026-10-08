// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Destination.hpp"

#include "FloorfieldRoutingEngine.hpp"

Point Destination::orientation(const Location& from) const
{
    return _engine->GetOrientation(from, _id);
}
