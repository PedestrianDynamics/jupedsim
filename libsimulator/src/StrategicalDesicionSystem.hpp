/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

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
    Run(const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
        std::vector<Agent>& agents) const;
};

template <typename Agent>
void StrategicalDecisionSystem<Agent>::Run(
    const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
    std::vector<Agent>& agents) const
{
    for(auto& agent : agents) {
        const auto [target, idx] = journeys.at(agent.journeyId)->Target(agent);
        agent.waypoint = target;
        agent.currentJourneyStage = idx;
    }
}
