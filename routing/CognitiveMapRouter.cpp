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
     //Checks if the Pedestrian once got a destination and calls init functions if needed (if no dest before)
     CheckAndInitPedestrian(p);

     //Check if the Pedestrian already has a Dest. or changed subroom and needs a new one.
     if(p->GetNextDestination() == -1 || p->ChangedSubRoom()) {
          //execute periodical sensors
          sensor_manager->execute(p, SensorManager::PERIODIC);

          //check if there is a way to the outside the pedestrian knows (in the cognitive map)
          const NavLine * destination = NULL;
          destination = (*cm_storage)[p]->GetDestination();
          if(destination == NULL) {
               //no destination was found, now we could start the discovery!
               //1. run the no_way sensors for room discovery.
               sensor_manager->execute(p, SensorManager::NO_WAY);

               //check if this was enough for finding a global path to the exit
               destination = (*cm_storage)[p]->GetDestination();

               if(destination == NULL) {
                    //we still do not have a way. lets take the "best" local edge
                    //for this we don't calculate the cost to exit but calculte the cost for the edges at the actual vertex.
                    destination = (*cm_storage)[p]->GetLocalDestination();
               }
          }

          //if we still could not found any destination we are lost! Pedestrian will be deleted
          //no destination should just appear in bug case or closed rooms.
          if(destination == NULL) {
               Log->Write("ERROR: \t One Pedestrian (ID: %i) was not able to find any destination", p->GetID());
               return -1;
          }

          p->SetExitLine(destination);
          p->SetExitIndex(destination->GetUniqueID());
     }
     return 1;
}

void CognitiveMapRouter::CheckAndInitPedestrian(Pedestrian * p)
{
     //check for former goal.
     if(p->GetLastDestination() == -1) {
          //no former goal. so initial route has to be choosen
          //this is needed for initialisation
          p->ChangedSubRoom();
          sensor_manager->execute(p, SensorManager::INIT);
     }
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
