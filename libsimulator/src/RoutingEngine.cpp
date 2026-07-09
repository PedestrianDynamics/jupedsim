// SPDX-License-Identifier: LGPL-3.0-or-later
#include "RoutingEngine.hpp"

Point RoutingEngine::compute_waypoint(Point currentPosition, Point destination)
{
    return compute_waypoints(currentPosition, destination)[1];
}
