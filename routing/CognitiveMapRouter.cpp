/**
 * \file        CognitiveMapRouter.cpp
 * \date        Feb 1, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/


#include "CognitiveMapRouter.h"
#include "Router.h"

#include "cognitive_map/CognitiveMapStorage.h"
#include "cognitive_map/CognitiveMap.h"
#include "cognitive_map/NavigationGraph.h"
#include "cognitive_map/sensor/SensorManager.h"

#include "../geometry/SubRoom.h"
#include "../geometry/NavLine.h"
#include "../geometry/Building.h"

#include "../pedestrian/Pedestrian.h"
#include "../tinyxml/tinyxml.h"

CognitiveMapRouter::CognitiveMapRouter()
{
    building=nullptr;
    cm_storage=nullptr;
    sensor_manager=nullptr;

}

CognitiveMapRouter::CognitiveMapRouter(int id, RoutingStrategy s) : Router(id, s)
{
    building=nullptr;
    cm_storage=nullptr;
    sensor_manager=nullptr;
}

CognitiveMapRouter::~CognitiveMapRouter()
{
     delete cm_storage;

}





int CognitiveMapRouter::FindExit(Pedestrian * p)
{
    //check for former goal.
    if((*cm_storage)[p]->HadNoDestination()) {
        sensor_manager->execute(p, SensorManager::INIT);
    }

    //Check if the Pedestrian already has a Dest. or changed subroom and needs a new one.
    if((*cm_storage)[p]->ChangedSubRoom()) {
        //execute periodical sensors
        sensor_manager->execute(p, SensorManager::CHANGED_ROOM);

        int status = FindDestination(p);

        (*cm_storage)[p]->UpdateSubRoom();

        return status;

    }
    return 1;
}

int CognitiveMapRouter::FindDestination(Pedestrian * p)
{
        //check if there is a way to the outside the pedestrian knows (in the cognitive map)
        const GraphEdge * destination = nullptr;
        destination = (*cm_storage)[p]->GetDestination();
        if(destination == nullptr) {
            //no destination was found, now we could start the discovery!
            //1. run the no_way sensors for room discovery.
            sensor_manager->execute(p, SensorManager::NO_WAY);

            //check if this was enough for finding a global path to the exit
            destination = (*cm_storage)[p]->GetDestination();

            if(destination == nullptr) {
                //we still do not have a way. lets take the "best" local edge
                //for this we don't calculate the cost to exit but calculte the cost for the edges at the actual vertex.
                destination = (*cm_storage)[p]->GetLocalDestination();
            }
        }

        //if we still could not found any destination we are lost! Pedestrian will be deleted
        //no destination should just appear in bug case or closed rooms.
        if(destination == nullptr) {
            Log->Write("ERROR: \t One Pedestrian (ID: %i) was not able to find any destination", p->GetID());
            return -1;
        }

        (*cm_storage)[p]->AddDestination(destination);
        sensor_manager->execute(p, SensorManager::NEW_DESTINATION);


        p->SetExitLine(destination->GetCrossing());
        p->SetExitIndex(destination->GetCrossing()->GetUniqueID());
        return 1;
}



bool CognitiveMapRouter::Init(Building * b)
{
     Log->Write("INFO:\tInit the Cognitive Map  Router Engine");
     building = b;

     //Init Cognitive Map Storage, second parameter: decides whether cognitive Map is empty or complete
     cm_storage = new CognitiveMapStorage(building,getOptions().at("CognitiveMap")[0]);
     Log->Write("INFO:\tInitialized CognitiveMapStorage");
     //Init Sensor Manager
     //sensor_manager = SensorManager::InitWithAllSensors(b, cm_storage);
     sensor_manager = SensorManager::InitWithCertainSensors(b, cm_storage, getOptions().at("Sensors"));
     Log->Write("INFO:\tInitialized SensorManager");
     return true;
}


const optStorage &CognitiveMapRouter::getOptions() const
{
    return options;
}

void CognitiveMapRouter::addOption(const std::string &key, const std::vector<std::string> &value)
{
    options.emplace(key,value);
}

