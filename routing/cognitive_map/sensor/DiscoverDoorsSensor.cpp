/**
 * @file DiscoverDoorsSensor.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 03, 2014
 * @brief
 */

#include "DiscoverDoorsSensor.h"


#include "../NavigationGraph.h"
#include "../../../geometry/Building.h"
#include "../CognitiveMap.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"
#include <vector>

DiscoverDoorsSensor::~DiscoverDoorsSensor()
{
}

std::string DiscoverDoorsSensor::GetName() const
{
    return "DiscoverDoorsSensor";
}

void DiscoverDoorsSensor::execute(const Pedestrian * pedestrian, CognitiveMap * cognitive_map) const
{
    SubRoom * sub_room = building->GetRoom(pedestrian->GetRoomID())->GetSubRoom(pedestrian->GetSubRoomID());

    const std::vector<Crossing *>& crossings = sub_room->GetAllCrossings();
    for(std::vector<Crossing*>::const_iterator it = crossings.begin(); it != crossings.end(); ++it)
    {
        cognitive_map->Add((*it));
    }
    //Add transitions as edges
    const std::vector<Transition *>& transitions = sub_room->GetAllTransitions();
    for(std::vector<Transition *>::const_iterator it = transitions.begin(); it != transitions.end(); ++it)
    {
        if((*it)->IsExit()) {
            cognitive_map->AddExit((*it));
        } else {
            cognitive_map->Add((*it));
        }
    }
}
