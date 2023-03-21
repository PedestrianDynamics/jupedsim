/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "IteratorPair.hpp"

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

    void Run(std::vector<Agent>& agents, std::vector<GenericAgent::ID>& removedAgentIds) const;
};

template <typename Agent>
void AgentExitSystem<Agent>::Run(
    std::vector<Agent>& agents,
    std::vector<GenericAgent::ID>& removedAgentIds) const
{
    auto iter = std::remove_if(
        std::begin(agents), std::end(agents), [&removedAgentIds](const GenericAgent& agent) {
            return std::find(std::begin(removedAgentIds), std::end(removedAgentIds), agent.id) !=
                   std::end(removedAgentIds);
        });
    agents.erase(iter, std::end(agents));
    removedAgentIds.clear();
}
