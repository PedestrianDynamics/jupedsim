/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <optional>

class Agent;

struct PedestrianUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    std::optional<Point> waitingPos{};
    Point e0{}; // desired direction
    bool resetTurning{false};
};

class OperationalModel : public Clonable<OperationalModel>
{
    struct Parameters {
    };

public:
    using ParametersID = jps::UniqueID<OperationalModel::Parameters>;
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual PedestrianUpdate ComputeNewPosition(
        double dT,
        const Agent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch& neighborhoodSearch) const = 0;

    virtual void ApplyUpdate(const PedestrianUpdate& update, Agent& agent) const = 0;
    virtual bool ParameterProfileExists(ParametersID id) const = 0;
};
