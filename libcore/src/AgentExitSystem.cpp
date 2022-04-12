#include "AgentExitSystem.hpp"
#include <memory>

void AgentExitSystem::Run(
    const std::map<Area::Id, Area> areas,
    std::vector<std::unique_ptr<Pedestrian>>& agents) const
{
    agents.erase(
        std::remove_if(
            std::begin(agents),
            std::end(agents),
            [&areas](const auto& agent) {
                if(!agent->goal) {
                    return false;
                }
                if(const auto area_iter = areas.find(*agent->goal); area_iter != std::end(areas)) {
                    return area_iter->second.polygon.Inside(agent->GetPos());
                }
                throw std::runtime_error("Invalid goal id in Pedestrian found");
            }),
        std::end(agents));
}
