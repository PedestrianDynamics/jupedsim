/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"
#include "Polygon.hpp"
#include "UniqueID.hpp"

#include <vector>

class Simulation;
struct GenericAgent;

class Stage
{
public:
    virtual ~Stage() = default;
    virtual bool IsCompleted(const GenericAgent& agent) const = 0;
    virtual Point Target() const = 0;
};

class Waypoint : public Stage
{
    Point position;
    double distance;

public:
    Waypoint(Point position_, double distance_);
    ~Waypoint() override = default;
    bool IsCompleted(const GenericAgent& agent) const override;
    Point Target() const override;
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public Stage
{
    Polygon area;
    std::vector<jps::UniqueID<GenericAgent>>& toRemove;

public:
    Exit(Polygon area, std::vector<jps::UniqueID<GenericAgent>>& toRemove_);
    ~Exit() override = default;
    bool IsCompleted(const GenericAgent& agent) const override;
    Point Target() const override;
};
