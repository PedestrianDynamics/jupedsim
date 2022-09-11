#include "StrategicalDesicionSystem.hpp"

#include <stdexcept>

void StrategicalDecisionSystem::Run(
    const std::map<Area::Id, Area>& areas,
    std::vector<std::unique_ptr<Agent>>& agents) const
{
    for(auto& agent : agents) {
        agent->waypoint = agent->behaviour->DesiredDestination(agent->GetPos());
    }
}
