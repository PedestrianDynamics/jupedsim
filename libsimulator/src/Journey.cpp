/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Journey.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include <iterator>
#include <memory>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
/// SimpleJourney
////////////////////////////////////////////////////////////////////////////////
template <class>
inline constexpr bool always_false_v = false;

Journey::Journey(
    const std::vector<StageDescription>& stageDescriptions,
    std::vector<GenericAgent::ID>& toRemove)
{
    stages.reserve(stageDescriptions.size());
    std::transform(
        std::begin(stageDescriptions),
        std::end(stageDescriptions),
        std::back_inserter(stages),
        [&toRemove](const auto& stageDesc) {
            std::unique_ptr<Stage> result{};
            std::visit(
                [&result, &toRemove](auto&& var) {
                    using T = std::decay_t<decltype(var)>;
                    if constexpr(std::is_same_v<T, WaypointDescription>) {
                        result = std::make_unique<Waypoint>(var.position, var.distance);
                    } else if constexpr(std::is_same_v<T, ExitDescription>) {
                        result = std::make_unique<Exit>(var.lineloop, toRemove);
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
        return std::make_tuple(stages[idx]->Target(), idx);
    }
    return std::make_tuple(stages.back()->Target(), stages.size() - 1);
}
