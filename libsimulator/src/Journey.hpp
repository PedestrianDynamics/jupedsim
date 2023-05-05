/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
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
    ID id;
    std::vector<std::unique_ptr<Stage>> stages{};

public:
    ~Journey() = default;

    Journey(
        const std::vector<StageDescription>& stageDescriptions,
        std::vector<GenericAgent::ID>& toRemove,
        const RoutingEngine& routingEngine);

    ID Id() const { return id; }

    std::tuple<Point, size_t> Target(const GenericAgent& agent) const;

    void HandleNofifyWaitingSetEvent(NotifyWaitingSet evt) const;
    void HandleNofifyQueueEvent(NotifyQueue evt) const;

    template <typename T>
    void Update(const NeighborhoodSearch<T>& neighborhoodSearch);

    size_t CountStages() const { return stages.size(); }
};

template <typename T>
void Journey::Update(const NeighborhoodSearch<T>& neighborhoodSearch)
{
    for(auto& stage : stages) {
        if(auto waitingSet = dynamic_cast<NotifiableWaitingSet*>(stage.get()); waitingSet) {
            waitingSet->Update(neighborhoodSearch);
        } else if(auto queue = dynamic_cast<NotifiableQueue*>(stage.get()); queue) {
            queue->Update(neighborhoodSearch);
        }
    }
}
