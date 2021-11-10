#include "AgentCreator.h"

#include "SimulationClock.h"
#include "geometry/Building.h"
#include "pedestrian/AgentsSourcesManager.h"
#include "pedestrian/Pedestrian.h"

#include <iterator>
#include <map>
#include <memory>
#include <tuple>


std::multimap<size_t, std::unique_ptr<Pedestrian>>
CreateAllPedestrians(Configuration * configuration, Building * building)
{
    using AgentVec = std::vector<std::unique_ptr<Pedestrian>>;
    AgentVec agents;
    building->SetAgents(&agents);

    configuration->GetDirectionManager()->Init(building);

    PedDistributor pd(configuration, &agents);
    pd.Distribute(building);
    std::multimap<size_t, std::unique_ptr<Pedestrian>> result{};

    for(auto && ped : agents) {
        result.emplace(std::make_pair(0, std::move(ped)));
    }
    agents.clear();

    AgentsSourcesManager mgr(building);
    for(const auto & src : pd.GetAgentsSources()) {
        mgr.AddSource(src);
    }
    mgr.GenerateAgents();
    SimulationClock clock(configuration->Getdt());

    do {
        building->SetAgents(&agents);
        auto agents = mgr.ProcessAllSources(clock.ElapsedTime());
        for(auto && ped : agents) {
            result.emplace(std::make_pair(clock.Iteration(), std::move(ped)));
        }
        clock.Advance();
    } while(!mgr.IsCompleted());

    building->SetAgents(nullptr);
    return result;
}
