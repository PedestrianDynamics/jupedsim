// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Events.hpp"
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "UniqueID.hpp"

#include <memory>
#include <tuple>
#include <vector>

class Journey
{
public:
    using ID = jps::UniqueID<Journey>;

private:
    ID id{};
    std::vector<Stage*> stages{};

public:
    ~Journey() = default;

    Journey(std::vector<Stage*> stages);

    ID Id() const { return id; }

    std::tuple<Point, size_t, Stage::ID> Target(const GenericAgent& agent) const;

    void HandleNofifyWaitingSetEvent(NotifyWaitingSet evt) const;
    void HandleNofifyQueueEvent(NotifyQueue evt) const;
    size_t CountStages() const { return stages.size(); }
    Stage* StageAt(size_t idx) { return stages.at(idx); }
};
