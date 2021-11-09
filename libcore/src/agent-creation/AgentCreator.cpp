#include "AgentCreator.h"

#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <memory>

std::vector<std::unique_ptr<Pedestrian>> CreateAllPedestrians(Building * building)
{
    std::vector<std::unique_ptr<Pedestrian>> agents{};
}

std::vector<std::unique_ptr<Pedestrian>>
CreateInitialPedestrians(Configuration & configuration, Building * building)
{
    std::vector<std::unique_ptr<Pedestrian>> agents{};
    PedDistributor pd(&configuration, &agents);
    pd.Distribute(building);
    return agents;
}
