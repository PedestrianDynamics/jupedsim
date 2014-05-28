/**
 * @file LastDestinationsSensor.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 03, 2014
 * @brief
 */

#include "LastDestinationsSensor.h"


#include "../NavigationGraph.h"
#include "../../../geometry/Building.h"
#include "../CognitiveMap.h"
#include "../../../pedestrian/Pedestrian.h"
#include <vector>
#include <set>

LastDestinationsSensor::~LastDestinationsSensor()
{
}

std::string LastDestinationsSensor::GetName() const
{
    return "LastDestinationsSensor";
}

void LastDestinationsSensor::execute(const Pedestrian * pedestrian, CognitiveMap * cognitive_map) const
{
    NavigationGraph & ng = (*cognitive_map->GetNavigationGraph());
    std::vector<const GraphEdge *> & destinations = cognitive_map->GetDestinations();
    int i = 1;

    std::set<const GraphEdge *> rated;

    for(std::vector<const GraphEdge *>::reverse_iterator it = destinations.rbegin(); it != destinations.rend(); ++it) {
        GraphEdge * to_edge = (*ng[(*it)->GetSrc()->GetSubRoom()])[(*it)->GetCrossing()];

        if(to_edge != NULL && rated.find(to_edge) == rated.end()) {
            to_edge->SetFactor(3000.0 / (i*3) + 100, GetName());
            rated.insert(to_edge);
        }

        if((*it)->GetSrc() != NULL && (*it)->GetDest() != NULL && (*it)->GetCrossing() != NULL) {
            GraphEdge * back_edge = (*ng[(*it)->GetDest()->GetSubRoom()])[(*it)->GetCrossing()];
            if(back_edge != NULL && rated.find(back_edge) == rated.end()) {
                back_edge->SetFactor(3000.0 /(i*3) + 100, GetName());
                rated.insert(back_edge);
            }
        }
        i++;
    }
}
