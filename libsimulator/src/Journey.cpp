/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include "Events.hpp"
#include "GenericAgent.hpp"
#include "RoutingEngine.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "TemplateHelper.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "fmt/ranges.h"

////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////
Journey::Journey(
    const std::vector<StageDescription>& stageDescriptions,
    std::vector<GenericAgent::ID>& toRemove,
    const RoutingEngine& routingEngine)
{
    stages.reserve(stageDescriptions.size());
    std::transform(
        std::begin(stageDescriptions),
        std::end(stageDescriptions),
        std::back_inserter(stages),
        [&toRemove, &routingEngine, this](const auto& stageDesc) {
            std::unique_ptr<Stage> result{};
            std::visit(
                [&result, &toRemove, &routingEngine, this](auto&& var) {
                    using T = std::decay_t<decltype(var)>;
                    if constexpr(std::is_same_v<T, WaypointDescription>) {
                        if(!routingEngine.IsRoutable(var.position)) {
                            throw SimulationError(
                                "Error creating Journey: Waypoint {} out of accessible area",
                                var.position);
                        }
                        result = std::make_unique<Waypoint>(var.position, var.distance);
                    } else if constexpr(std::is_same_v<T, ExitDescription>) {
                        // TODO(kkratz): Add check to ensure polygon is inside routable area
                        result = std::make_unique<Exit>(var.polygon, toRemove);
                    } else if constexpr(std::is_same_v<T, NotifiableWaitingSetDescription>) {
                        std::vector<Point> pointsOutside{};
                        std::copy_if(
                            std::begin(var.slots),
                            std::end(var.slots),
                            std::back_inserter(pointsOutside),
                            [&routingEngine](const auto& p) {
                                return !routingEngine.IsRoutable(p);
                            });
                        if(!pointsOutside.empty()) {
                            throw SimulationError(
                                "Error creating Journey: NotifiableWaitingSet contais waiting "
                                "points outside of accessible area, {}",
                                pointsOutside);
                        }
                        result = std::make_unique<NotifiableWaitingSet>(var.slots, id);
                    } else if constexpr(std::is_same_v<T, NotifiableQueueDescription>) {
                        std::vector<Point> pointsOutside{};
                        std::copy_if(
                            std::begin(var.slots),
                            std::end(var.slots),
                            std::back_inserter(pointsOutside),
                            [&routingEngine](const auto& p) {
                                return !routingEngine.IsRoutable(p);
                            });
                        if(!pointsOutside.empty()) {
                            throw SimulationError(
                                "Error creating Journey: NotifiableQueue contais waiting "
                                "points outside of accessible area, {}",
                                pointsOutside);
                        }
                        result = std::make_unique<NotifiableQueue>(var.slots, id);
                    } else {
                        static_assert(always_false_v<T>, "non-exhaustive visitor!");
                    }
                },
                stageDesc);
            return result;
        });
}

std::tuple<Point, size_t> Journey::Target(const GenericAgent& agent) const
{
    for(size_t idx = agent.currentJourneyStage; idx < stages.size(); ++idx) {
        if(stages[idx]->IsCompleted(agent)) {
            continue;
        }
        return std::make_tuple(stages[idx]->Target(agent), idx);
    }
    return std::make_tuple(stages.back()->Target(agent), stages.size() - 1);
}

void Journey::HandleNofifyWaitingSetEvent(NotifyWaitingSet evt) const
{
    if(evt.stageIdx >= stages.size()) {
        throw SimulationError("Journey {} has no stage {}", id.getID(), evt.stageIdx);
    }
    auto stage = dynamic_cast<NotifiableWaitingSet*>(stages[evt.stageIdx].get());
    if(stage == nullptr) {
        throw SimulationError(
            "Journey {} has no NotiafiableWaitingSet at stage {}", id.getID(), evt.stageIdx);
    }
    stage->State(evt.newState);
}

void Journey::HandleNofifyQueueEvent(NotifyQueue evt) const
{
    if(evt.stageIdx >= stages.size()) {
        throw SimulationError("Journey {} has no stage {}", id.getID(), evt.stageIdx);
    }
    auto stage = dynamic_cast<NotifiableQueue*>(stages[evt.stageIdx].get());
    if(stage == nullptr) {
        throw SimulationError(
            "Journey {} has no NotiafiableWaitingSet at stage {}", id.getID(), evt.stageIdx);
    }
    stage->Pop(evt.count);
}
