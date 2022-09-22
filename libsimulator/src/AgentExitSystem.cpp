/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "AgentExitSystem.hpp"
#include "IteratorPair.hpp"
#include <memory>

void AgentExitSystem::Run(
    const std::map<Area::Id, Area> areas,
    std::vector<std::unique_ptr<Agent>>& agents,
    std::vector<uint64_t>& removedAgentIds) const
{
    removedAgentIds.clear();
    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&areas, &removedAgentIds](const std::unique_ptr<Agent>& agent) {
            for(const auto& [k, v] : areas) {
                if(v.labels.find("exit") != v.labels.end()) {
                    auto inside = v.polygon.Inside(agent->pos);
                    if(inside) {
                        removedAgentIds.emplace_back(agent->id.getID());
                        return true;
                    }
                }
            }
            return false;
        });
    agents.erase(iter, std::end(agents));
}
