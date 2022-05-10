#include "AgentCreator.hpp"

#include "Pedestrian.hpp"
#include "SimulationClock.hpp"
#include "geometry/Building.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"
#include "pedestrian/PedDistributor.hpp"

#include <iterator>
#include <map>
#include <memory>
#include <tuple>

std::multimap<size_t, std::unique_ptr<Pedestrian>>
CreateAllPedestrians(Configuration* configuration, Building* building, double max_time)
{
    using AgentVec = std::vector<std::unique_ptr<Pedestrian>>;
    AgentVec agents;

    PedDistributor pd(configuration, &agents);
    pd.Distribute(building);
    std::multimap<size_t, std::unique_ptr<Pedestrian>> result{};

    for(auto&& ped : agents) {
        result.emplace(std::make_pair(0, std::move(ped)));
    }
    agents.clear();

    AgentsSourcesManager mgr(building);
    for(const auto& src : pd.GetAgentsSources()) {
        mgr.AddSource(src);
    }
    mgr.GenerateAgents();
    SimulationClock clock(configuration->dT);

    do {
        auto agents = mgr.ProcessAllSources(clock.ElapsedTime());
        for(auto&& ped : agents) {
            result.emplace(std::make_pair(clock.Iteration(), std::move(ped)));
        }
        clock.Advance();
    } while(!mgr.IsCompleted() && clock.ElapsedTime() < max_time);
    return result;
}
