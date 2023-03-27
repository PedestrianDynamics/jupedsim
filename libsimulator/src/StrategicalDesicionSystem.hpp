/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Area.hpp"
#include "Journey.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

template <typename Agent>
class StrategicalDecisionSystem
{
public:
    StrategicalDecisionSystem() = default;
    ~StrategicalDecisionSystem() = default;
    StrategicalDecisionSystem(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem& operator=(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem(StrategicalDecisionSystem&& other) = delete;
    StrategicalDecisionSystem& operator=(StrategicalDecisionSystem&& other) = delete;

    void
    Run(const std::map<Area::Id, Area>& areas,
        const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
        std::vector<Agent>& agents) const;
};

template <typename Agent>
void StrategicalDecisionSystem<Agent>::Run(
    const std::map<Area::Id, Area>& areas,
    const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
    std::vector<Agent>& agents) const
{
    const auto next_waypoint = [](const auto& journey, const auto& agent) -> auto
    {
        for(auto index = agent.currentJourneyStage; index < journey->size(); ++index) {
            const auto [currentWaypoint, maximumDistance] = (*journey)[index];
            const auto distance = (agent.pos - currentWaypoint).Norm();
            if(distance > maximumDistance) {
                return std::make_tuple(index, currentWaypoint);
            }
        }
        return std::make_tuple(journey->size() - 1, agent.pos);
    };
    for(auto& agent : agents) {
        const auto& journey = journeys.at(agent.journeyId);
        const auto [idx, pos] = next_waypoint(journey, agent);
        agent.currentJourneyStage = idx;
        agent.waypoint = pos;
    }
}
