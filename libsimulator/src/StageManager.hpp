// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry/Geometry.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "Visitor.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace detail
{
inline std::vector<Location> locate_slots(
    const Geometry& geometry,
    const std::vector<Point>& slots,
    std::optional<std::size_t> region_id)
{
    std::vector<Location> located{};
    located.reserve(slots.size());
    for(const auto& slot : slots) {
        located.push_back(geometry.get_location(slot.x, slot.y, region_id));
    }
    return located;
}
} // namespace detail

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
        std::vector<GenericAgent::ID>& removedAgentsInLastIteration,
        const Geometry& geometry,
        std::optional<std::size_t> region_id)
    {
        std::unique_ptr<BaseStage> stage = std::visit(
            overloaded{
                [&geometry, region_id](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<Waypoint>(
                        geometry.get_location(d.position.x, d.position.y, region_id), d.distance);
                },
                [&removedAgentsInLastIteration, &geometry, region_id](
                    const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                    const auto centroid = d.polygon.Centroid();
                    return std::make_unique<Exit>(
                        d.polygon,
                        geometry.get_location(centroid.x, centroid.y, region_id),
                        removedAgentsInLastIteration);
                },
                [&geometry, region_id](
                    const NotifiableWaitingSetDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableWaitingSet>(
                        detail::locate_slots(geometry, d.slots, region_id));
                },
                [&geometry,
                 region_id](const NotifiableQueueDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableQueue>(
                        detail::locate_slots(geometry, d.slots, region_id));
                },
                [](const DirectSteeringDescription&) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<DirectSteering>();
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
