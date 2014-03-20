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

void SensorManager::execute(const Pedestrian * ped )
{
    for(SensorContainer::iterator sensor = registered_sensors.begin(); sensor != registered_sensors.end(); ++sensor) {
        (*sensor)->execute(ped, (*cm_storage)[ped]);
    }

}

void SensorManager::Register(AbstractSensor * sensor)
{
    registered_sensors.push_back(sensor);
}

SensorManager * SensorManager::InitWithAllSensors(const Building * b, CognitiveMapStorage * cm_storage)
{
    SensorManager * sensor_manager = new SensorManager(b, cm_storage);

    //Init and register Sensors
    sensor_manager->Register(new RoomToFloorSensor(b));

    return sensor_manager;
}
