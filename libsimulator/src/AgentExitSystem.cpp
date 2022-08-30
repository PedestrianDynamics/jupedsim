#include "AgentExitSystem.hpp"
#include "IteratorPair.hpp"
#include <memory>

void AgentExitSystem::Run(
    const std::map<Area::Id, Area> areas,
    std::vector<std::unique_ptr<Pedestrian>>& agents,
    std::vector<uint64_t>& removedAgentIds) const
{
    removedAgentIds.clear();
    auto iter = std::remove_if(
        std::begin(agents),
        std::end(agents),
        [&areas, &removedAgentIds](const std::unique_ptr<Pedestrian>& agent) {
            for(const auto& [k, v] : areas) {
                if(v.labels.find("exit") != v.labels.end()) {
                    auto inside = v.polygon.Inside(agent->GetPos());
                    if(inside) {
                        removedAgentIds.emplace_back(agent->GetUID().getID());
                        return true;
                    }
                }
            }
            return false;
        });
    agents.erase(iter, std::end(agents));
}
