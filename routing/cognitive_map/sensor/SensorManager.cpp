/**
 * \file        CompleteCognitiveMapCreator.cpp
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
 

#include "SensorManager.h"
#include "AbstractSensor.h"
//sensors
#include "RoomToFloorSensor.h"
#include "DiscoverDoorsSensor.h"
#include "LastDestinationsSensor.h"
#include "SmokeSensor.h"
#include "JamSensor.h"

#include "../../../geometry/Building.h"
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
    sensor_manager->Register(new DiscoverDoorsSensor(b),  NO_WAY );
    sensor_manager->Register(new RoomToFloorSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );
    sensor_manager->Register(new SmokeSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );

    sensor_manager->Register(new LastDestinationsSensor(b), CHANGED_ROOM );
    sensor_manager->Register(new JamSensor(b), PERIODIC | NO_WAY | CHANGED_ROOM );

    return sensor_manager;
}

SensorManager *SensorManager::InitWithCertainSensors(const Building * b, CognitiveMapStorage * cm_storage, std::vector<std::string> sensors)
{
    SensorManager * sensor_manager = new SensorManager(b, cm_storage);

    sensor_manager->Register(new DiscoverDoorsSensor(b),  NO_WAY );
    sensor_manager->Register(new LastDestinationsSensor(b), CHANGED_ROOM );

    for (auto &it : sensors )
    {
        if (it =="Room2Corridor")
        {
            sensor_manager->Register(new RoomToFloorSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );
        }
        else if (it == "Jam")
        {
            sensor_manager->Register(new JamSensor(b), PERIODIC | NO_WAY | CHANGED_ROOM );
        }
        else if (it == "Smoke")
        {
            sensor_manager->Register(new SmokeSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );
        }
    }


    return sensor_manager;
}
