/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "Point.hpp"

#include <UniqueID.hpp>

#include <tuple>
#include <vector>

class Journey : public Clonable<Journey>
{
public:
    using ID = jps::UniqueID<Journey>;

private:
    ID id;
    using Waypoint = std::tuple<Point, double>;
    std::vector<Waypoint> waypoints{};

public:
    virtual ~Journey() = default;

    ID Id() const { return id; }

    void AddWaypoint(Point p, double distance);

    const Waypoint& operator[](size_t index) const { return waypoints[index]; }

    size_t size() const { return waypoints.size(); }

    std::unique_ptr<Journey> Clone() const override;
};
