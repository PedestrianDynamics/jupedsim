/**
 * @file   CognitiveMapRouter.cpp
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Routing Engine for Cognitive Map
 *
 */

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
}

CognitiveMapRouter::~CognitiveMapRouter()
{
    delete cm_storage;

}

int CognitiveMapRouter::FindExit(Pedestrian * p)
{
    //check for former goal.
    if(p->GetLastDestination() == -1) {
        //no former goal. so initial route has to be choosen
        //this is needed for initialisation
        p->ChangedSubRoom();
    }

    if(p->GetNextDestination() == -1 || p->ChangedSubRoom()) {
        // execute the sensors
        sensor_manager->execute(p);

        // check for a destination
        const NavLine * destination = (*cm_storage)[p]->GetDestination();
        if(destination != NULL) {
            p->SetExitLine(destination);
            p->SetExitIndex(destination->GetUniqueID());
        } else {
            // no destination was found, now we could start the discovery!

        }

    }

    return 1;

}

void CognitiveMapRouter::Init(Building * b)
{
    Log->Write("INFO:\tInit the Cognitive Map  Router Engine");
    building = b;

    //Init Cognitive Map Storage
    cm_storage = new CognitiveMapStorage(building);
    Log->Write("INFO:\tInitialized CognitiveMapStorage");
    //Init Sensor Manager
    sensor_manager = SensorManager::InitWithAllSensors(b, cm_storage);
    Log->Write("INFO:\tInitialized SensorManager");
}
