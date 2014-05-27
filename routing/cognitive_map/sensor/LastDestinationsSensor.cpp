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

    for(std::vector<const GraphEdge *>::iterator it = destinations.begin(); it != destinations.end(); ++it) {
        GraphEdge * to_edge = (*ng[(*it)->GetSrc()->GetSubRoom()])[(*it)->GetCrossing()];
        if(to_edge != NULL)
            to_edge->SetFactor(100.0, GetName());
        if((*it)->GetSrc() != NULL && (*it)->GetDest() != NULL && (*it)->GetCrossing() != NULL) {
            GraphEdge * back_edge = (*ng[(*it)->GetDest()->GetSubRoom()])[(*it)->GetCrossing()];
            if(back_edge != NULL) {
                back_edge->SetFactor(100.0, GetName());
            }
        }
    }
}
