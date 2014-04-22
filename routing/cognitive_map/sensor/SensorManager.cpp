/**
 * @file CompleteCognitiveMapCreator.cpp
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */

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
