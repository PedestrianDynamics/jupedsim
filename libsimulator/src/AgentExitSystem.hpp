// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "IteratorPair.hpp"
#include "StageManager.hpp"

#include <map>
#include <vector>

template <typename Agent>
class AgentExitSystem
{
public:
    AgentExitSystem() = default;
    ~AgentExitSystem() = default;
    AgentExitSystem(const AgentExitSystem& other) = delete;
    AgentExitSystem& operator=(const AgentExitSystem& other) = delete;
    AgentExitSystem(AgentExitSystem&& other) = delete;
    AgentExitSystem& operator=(AgentExitSystem&& other) = delete;

    void
    Run(std::vector<Agent>& agents,
        std::vector<GenericAgent::ID>& removedAgentIds,
        StageManager& stageManager) const;
};

template <typename Agent>
void AgentExitSystem<Agent>::Run(
    std::vector<Agent>& agents,
    std::vector<GenericAgent::ID>& removedAgentIds,
    StageManager& stageManager) const
{

    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&removedAgentIds, &stageManager](const GenericAgent& agent) {
            auto found =
                std::find(std::begin(removedAgentIds), std::end(removedAgentIds), agent.id) !=
                std::end(removedAgentIds);
            if(found) {
                stageManager.HandleRemoveAgent(agent.stageId);
            }
            return found;
        });
    agents.erase(iter, std::end(agents));

    removedAgentIds.clear();
}
