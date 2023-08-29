// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Journey.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

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
        auto&& agents) const
    {
        for(auto& agent : agents) {
            const auto [target, idx, id] = journeys.at(agent.journeyId)->Target(agent);
            agent.waypoint = target;
            agent.currentJourneyStageIdx = idx;
            agent.stageId = id;
        }
    }
};
