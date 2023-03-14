/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Area.hpp"
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

    void
    Run(const std::map<Area::Id, Area> areas,
        std::vector<Agent>& agents,
        std::vector<uint64_t>& removedAgentIds) const;
};

template <typename Agent>
void AgentExitSystem<Agent>::Run(
    const std::map<Area::Id, Area> areas,
    std::vector<Agent>& agents,
    std::vector<uint64_t>& removedAgentIds) const
{
    removedAgentIds.clear();
    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&areas, &removedAgentIds](const GenericAgent& agent) {
            for(const auto& [k, v] : areas) {
                if(v.labels.find("exit") != v.labels.end()) {
                    auto inside = v.polygon.Inside(agent.pos);
                    if(inside) {
                        removedAgentIds.emplace_back(agent.id.getID());
                        return true;
                    }
                }
            }
            return false;
        });
    agents.erase(iter, std::end(agents));
}
