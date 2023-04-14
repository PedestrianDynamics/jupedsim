/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"

#include "Events.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "TemplateHelper.hpp"

#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////
Journey::Journey(
    const std::vector<StageDescription>& stageDescriptions,
    std::vector<GenericAgent::ID>& toRemove)
{
    stages.reserve(stageDescriptions.size());
    std::transform(
        std::begin(stageDescriptions),
        std::end(stageDescriptions),
        std::back_inserter(stages),
        [&toRemove, this](const auto& stageDesc) {
            std::unique_ptr<Stage> result{};
            std::visit(
                [&result, &toRemove, this](auto&& var) {
                    using T = std::decay_t<decltype(var)>;
                    if constexpr(std::is_same_v<T, WaypointDescription>) {
                        result = std::make_unique<Waypoint>(var.position, var.distance);
                    } else if constexpr(std::is_same_v<T, ExitDescription>) {
                        result = std::make_unique<Exit>(var.polygon, toRemove);
                    } else if constexpr(std::is_same_v<T, NotifiableWaitingSetDescription>) {
                        result = std::make_unique<NotifiableWaitingSet>(var.slots, id);
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
        throw std::runtime_error(
            fmt::format("Journey {} has no stage {}", id.getID(), evt.stageIdx));
    }
    auto stage = dynamic_cast<NotifiableWaitingSet*>(stages[evt.stageIdx].get());
    if(stage == nullptr) {
        throw std::runtime_error(fmt::format(
            "Journey {} has no NotiafiableWaitingSet at stage {}", id.getID(), evt.stageIdx));
    }
    stage->State(evt.newState);
}
