// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/Location.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <vector>

using RoutingTarget = Point3D; // [RL] TODO: Support more than just points

/// Pure interface for 3D routing engines.
class RoutingEngine
{
public:
    /// @param wallClearance how far a route is held off the wall corners it turns on.
    explicit RoutingEngine(double wallClearance = 0.2) : _wallClearance(wallClearance)
    {
        if(wallClearance < 0.0) {
            throw SimulationError("Wall clearance cannot be negative, got {}.", wallClearance);
        }
    }
    virtual ~RoutingEngine() = default;

    // Non-copyable and non-movable
    RoutingEngine(const RoutingEngine&) = delete;
    RoutingEngine& operator=(const RoutingEngine&) = delete;
    RoutingEngine(RoutingEngine&&) = delete;
    RoutingEngine& operator=(RoutingEngine&&) = delete;

    /// Checks whether the provided location (3D-point or polygon)
    /// is on walkable surface taking wall clearance into account.
    /// @param loc location (Point or Polygon) to check
    /// @return true if the location projects onto the walkable surface
    virtual bool IsValidLocation(const RoutingTarget& loc) const = 0;

    /// Compute the shortest path from @p source to @p target, held off wall corners by the
    /// engine's wall clearance.
    /// @param source where to route from
    /// @param target where to route to
    /// @return the path, including source as first and target as last element
    virtual std::vector<Point3D>
    GetShortestPath(const Point3D& source, const RoutingTarget& target) = 0;

    /// Get orientation to next point of the shortest path from @p source to
    /// @p target, projected to x/y.
    /// @param source where to route from
    /// @param target where to route to
    /// @return 2D orientation to the next waypoint
    virtual Point GetOrientation(const Point3D& source, const RoutingTarget& target) = 0;

    /// The very next point to target from @p from to @p to, projected to x/y.
    ///
    /// Interim: The idea is to move to `GetOrientation`.
    virtual Point ComputeWaypoint(const Location& from, const Location& to) = 0;

    double WallClearance() const { return _wallClearance; }

private:
    double _wallClearance;
};
