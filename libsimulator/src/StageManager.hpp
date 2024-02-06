// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Stage.hpp"
#include "StageDescription.hpp"
#include "Visitor.hpp"
#include <unordered_map>

class StageManager
{
private:
    std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>> stages;

public:
    StageManager() {}
    ~StageManager() = default;
    StageManager(const StageManager& other) = delete;
    StageManager& operator=(const StageManager& other) = delete;
    StageManager(StageManager&& other) = delete;
    StageManager& operator=(StageManager&& other) = delete;

    BaseStage::ID AddStage(
        const StageDescription stageDescription,
        std::vector<GenericAgent::ID>& removedAgentsInLastIteration)
    {
        std::unique_ptr<BaseStage> stage = std::visit(
            overloaded{
                [](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<Waypoint>(d.position, d.distance);
                },
                [&removedAgentsInLastIteration](
                    const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<Exit>(d.polygon, removedAgentsInLastIteration);
                },
                [](const NotifiableWaitingSetDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableWaitingSet>(d.slots);
                },
                [](const NotifiableQueueDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableQueue>(d.slots);
                }},
            stageDescription);
        if(stages.find(stage->Id()) != stages.end()) {
            throw SimulationError("Internal error, stage id already in use.");
        }
        const auto id = stage->Id();
        stages.emplace(id, std::move(stage));
        return id;
    }

    void MigrateAgent(BaseStage::ID prevTarget, BaseStage::ID newTarget)
    {
        stages.at(newTarget)->IncreaseTargeting();
        stages.at(prevTarget)->DecreaseTargeting();
    }

    void HandleNewAgent(BaseStage::ID stageId) { stages.at(stageId)->IncreaseTargeting(); }
    void HandleRemoveAgent(BaseStage::ID stageId) { stages.at(stageId)->DecreaseTargeting(); }

    BaseStage* Stage(BaseStage::ID stageId) const
    {
        const auto iter = stages.find(stageId);
        if(iter == std::end(stages)) {
            throw SimulationError("Unknown stage id ({}) provided in journey.", stageId.getID());
        }
        return iter->second.get();
    }

    BaseStage* Stage(BaseStage::ID stageId)
    {
        auto iter = stages.find(stageId);
        if(iter == std::end(stages)) {
            throw SimulationError("Unknown stage id ({}) provided in journey.", stageId.getID());
        }
        return iter->second.get();
    }

    std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>>& Stages() { return stages; }

    const std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>>& Stages() const
    {
        return stages;
    }
};
