/**
 * \file        CompleteCognitiveMapCreator.cpp
 * \date        Feb 1, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#include "DiscoverDoorsSensor.h"
#include "JamSensor.h"
#include "LastDestinationsSensor.h"
#include "locater.h"
#include "RoomToFloorSensor.h"

#include "geometry/Building.h"
#include "JPSfire/A_smoke_sensor/SmokeSensor.h"
#include "router/smoke_router/BrainStorage.h"

SensorManager::SensorManager(const Building * /*b*/, BrainStorage * cms)
     : /*building(b), */cm_storage(cms)
{
    _periodicUpdateInterval=1/UPDATE_RATE;
}

SensorManager::~SensorManager()
{
}

void SensorManager::execute(const Pedestrian * ped, EventType event_type)
{
     for(SensorContainer::iterator it = registered_sensors.begin(); it != registered_sensors.end(); ++it) {
          if(event_type & it->second) {
               it->first->execute(ped, (*cm_storage)[ped]->GetCognitiveMap());
          }
     }
}

void SensorManager::Register(AbstractSensor * sensor, EventType events)
{
     registered_sensors.push_back(std::make_pair(sensor, events));
}

SensorManager * SensorManager::InitWithAllSensors(const Building * b, BrainStorage *cm_storage)
{
     SensorManager * sensor_manager = new SensorManager(b, cm_storage);

    //Init and register Sensors
    sensor_manager->Register(new DiscoverDoorsSensor(b),  NO_WAY );
    sensor_manager->Register(new RoomToFloorSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );
    //sensor_manager->Register(new SmokeSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );

    sensor_manager->Register(new LastDestinationsSensor(b), CHANGED_ROOM );
    sensor_manager->Register(new JamSensor(b), PERIODIC | NO_WAY | CHANGED_ROOM );
    sensor_manager->Register(new Locater(b,sensor_manager->GetIntVPeriodicUpdate()),  PERIODIC );

    return sensor_manager;
}
#ifdef JPSFIRE
SensorManager *SensorManager::InitWithCertainSensors(const Building * b, BrainStorage * cm_storage, const optStorage& optSto)
{
    SensorManager * sensor_manager = new SensorManager(b, cm_storage);

    sensor_manager->Register(new DiscoverDoorsSensor(b),  NO_WAY );
    sensor_manager->Register(new LastDestinationsSensor(b), CHANGED_ROOM );
    sensor_manager->Register(new Locater(b,sensor_manager->GetIntVPeriodicUpdate()),  PERIODIC );

    std::vector<std::string> sensors = optSto.at("Sensors");
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
            //std::string smokeFilepath = optSto.at("smokeOptions").at(0);
            //double updatet = std::stod(optSto.at("smokeOptions").at(1));
            //double finalt = std::stod(optSto.at("smokeOptions").at(2));
            sensor_manager->Register(new SmokeSensor(b), INIT | PERIODIC | NO_WAY | CHANGED_ROOM );
        }
    }

    return sensor_manager;
}
#endif
void SensorManager::SetIntVPeriodicUpdate(const double &interval)
{
    _periodicUpdateInterval=interval;
}

const double &SensorManager::GetIntVPeriodicUpdate()
{
    return _periodicUpdateInterval;
}
