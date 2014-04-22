/**
 * @file CompleteCognitiveMapCreator.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */

#include "CompleteCognitiveMapCreator.h"
#include "CognitiveMap.h"
#include <vector>
#include <map>
#include "../../geometry/Room.h"
#include "../../geometry/SubRoom.h"
#include "../../geometry/Building.h"
#include "../../geometry/Crossing.h"
#include "../../geometry/Transition.h"

CompleteCognitiveMapCreator::~CompleteCognitiveMapCreator()
{
    return;
}

CognitiveMap * CompleteCognitiveMapCreator::CreateCognitiveMap(const Pedestrian * ped)
{
    CognitiveMap * cm = new CognitiveMap(building, ped);

    //adding all SubRooms as Vertex
    const std::vector<Room *>& rooms = building->GetAllRooms();
    for(std::vector<Room *>::const_iterator it = rooms.begin(); it != rooms.end(); ++it)
    {
        const std::vector<SubRoom *>& sub_rooms = (*it)->GetAllSubRooms();
        for(std::vector<SubRoom *>::const_iterator it = sub_rooms.begin(); it != sub_rooms.end(); ++it)
        {
            cm->Add((*it));
        }
    }
    //Add crossings as edges
    const std::map<int, Crossing *>& crossings = building->GetAllCrossings();
    for(std::map<int, Crossing*>::const_iterator it = crossings.begin(); it != crossings.end(); ++it)
    {
        cm->Add(it->second);
    }
    //Add transitions as edges
    const std::map<int, Transition *>& transitions = building->GetAllTransitions();
    for(std::map<int, Transition *>::const_iterator it = transitions.begin(); it != transitions.end(); ++it)
    {
        if(it->second->IsExit()) {
            cm->AddExit(it->second);
        } else {
            cm->Add(it->second);
        }
    }

    return cm;
}
