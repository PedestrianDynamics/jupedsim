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
            if(!agent->goal) {
                return false;
            }
            if(const auto area_iter = areas.find(*agent->goal); area_iter != std::end(areas)) {
                auto inside = area_iter->second.polygon.Inside(agent->GetPos());
                if(inside) {
                    removedAgentIds.emplace_back(agent->GetUID().getID());
                }
                return inside;
            }
            throw std::runtime_error("Invalid goal id in Pedestrian found");
        });
    agents.erase(iter, std::end(agents));
}
