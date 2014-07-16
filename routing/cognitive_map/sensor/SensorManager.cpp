/**
 * \file        CompleteCognitiveMapCreator.cpp
 * \date        Feb 1, 2014
 * \version     v0.5
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
 

#include "SensorManager.h"
#include "AbstractSensor.h"
#include "../../../geometry/Building.h"
#include "RoomToFloorSensor.h"
#include "../CognitiveMapStorage.h"
#include "../navigation_graph/GraphVertex.h"
#include "../NavigationGraph.h"
#include "../CognitiveMap.h"

SensorManager::SensorManager(const Building * b, CognitiveMapStorage * cms)
     : building(b), cm_storage(cms)
{

}

SensorManager::~SensorManager()
{
}

void SensorManager::execute(const Pedestrian * ped, EventType event_type)
{
     for(SensorContainer::iterator it = registered_sensors.begin(); it != registered_sensors.end(); ++it) {
          if(event_type & it->second) {
               it->first->execute(ped, (*cm_storage)[ped]);
          }
     }
}

void SensorManager::Register(AbstractSensor * sensor, EventType events)
{
     registered_sensors.push_back(std::make_pair(sensor, events));
}

SensorManager * SensorManager::InitWithAllSensors(const Building * b, CognitiveMapStorage * cm_storage)
{
     SensorManager * sensor_manager = new SensorManager(b, cm_storage);

     //Init and register Sensors
     sensor_manager->Register(new RoomToFloorSensor(b), INIT | PERIODIC | NO_WAY );

     return sensor_manager;
}
