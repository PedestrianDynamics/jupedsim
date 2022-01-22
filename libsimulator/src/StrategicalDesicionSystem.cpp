#include "StrategicalDesicionSystem.hpp"

#include <stdexcept>

void StrategicalDecisionSystem::Run(
    const std::map<Area::Id, Area>& areas,
    std::vector<std::unique_ptr<Pedestrian>>& agents) const
{
    // TODO(kkratz): Right now we just pick the first exit area and tell all agents to go there
    auto find_first_exit = [](const auto& areas) {
        for(const auto& [k, v] : areas) {
            if(v.labels.count("exit") == 1) {
                return k;
            }
        }
        throw std::runtime_error("No exit defined");
    };

    const auto exit_id = find_first_exit(areas);

    for(auto& agent : agents) {
        if(!agent->goal) {
            agent->goal = exit_id;
        }
    }
}
