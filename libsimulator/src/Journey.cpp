/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"
#include <memory>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////

void Journey::AddWaypoint(Point p, double distance)
{
    waypoints.emplace_back(std::make_tuple(p, distance));
}

std::unique_ptr<Journey> Journey::Clone() const
{
    return std::make_unique<Journey>(*this);
}
