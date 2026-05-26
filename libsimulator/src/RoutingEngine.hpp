// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <string>
#include <vector>

class CollisionGeometry;

class RoutingEngine : public Clonable<RoutingEngine>
{
public:
    using ID = jps::UniqueID<RoutingEngine>;

private:
    ID _id{};

public:
    ~RoutingEngine() override = default;

    RoutingEngine() = default;
    RoutingEngine(const RoutingEngine&) = delete;
    RoutingEngine& operator=(const RoutingEngine&) = delete;
    RoutingEngine(RoutingEngine&&) = default;
    RoutingEngine& operator=(RoutingEngine&&) = default;

    ID Id() const { return _id; }

    virtual std::string name() const = 0;
    virtual void SetGeometry(const CollisionGeometry& geometry) = 0;
    virtual std::vector<Point> ComputeAllWaypoints(Point from, Point destination) = 0;
    virtual bool IsRoutable(Point p) const = 0;

    Point ComputeWaypoint(Point currentPosition, Point destination);
};
