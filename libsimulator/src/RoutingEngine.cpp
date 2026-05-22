// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

Point RoutingEngine::ComputeWaypoint(Point currentPosition, Point destination)
{
    return ComputeAllWaypoints(currentPosition, destination)[1];
}
