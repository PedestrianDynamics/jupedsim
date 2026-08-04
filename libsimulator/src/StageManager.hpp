// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "Visitor.hpp"

#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace detail
{
/// Put a stage's representative point on the surface. @p what names the stage kind in the
/// error message.
inline Location locate_stage_point(const Geometry3D& geometry, Point point, std::string_view what)
{
    const auto location = geometry.get_location(point.x, point.y, 0.0);
    if(!location) {
        throw SimulationError("{} {} not inside walkable area", what, point);
    }
    return *location;
}

inline std::vector<Location>
locate_slots(const Geometry3D& geometry, const std::vector<Point>& slots, std::string_view what)
{
    std::vector<Location> located{};
    located.reserve(slots.size());
    for(const auto& slot : slots) {
        located.push_back(locate_stage_point(geometry, slot, what));
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
        const Geometry3D& geometry)
    {
        std::unique_ptr<BaseStage> stage = std::visit(
            overloaded{
                [&geometry](const WaypointDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<Waypoint>(
                        detail::locate_stage_point(geometry, d.position, "WayPoint"), d.distance);
                },
                [&removedAgentsInLastIteration,
                 &geometry](const ExitDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<Exit>(
                        d.polygon,
                        detail::locate_stage_point(geometry, d.polygon.Centroid(), "Exit"),
                        removedAgentsInLastIteration);
                },
                [&geometry](
                    const NotifiableWaitingSetDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableWaitingSet>(
                        detail::locate_slots(geometry, d.slots, "NotifiableWaitingSet point"));
                },
                [&geometry](const NotifiableQueueDescription& d) -> std::unique_ptr<BaseStage> {
                    return std::make_unique<NotifiableQueue>(
                        detail::locate_slots(geometry, d.slots, "NotifiableQueue point"));
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
