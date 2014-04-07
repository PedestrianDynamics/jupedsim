/**
 * @file   SensorManager.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   January, 2014
 * @brief  The Sensor Manager cares about Sensor Registration and Execution
 *
 */

#ifndef SENSORMANAGER_H_
#define SENSORMANAGER_H_

class Building;
class CognitiveMapStorage;
class AbstractSensor;
class Pedestrian;


#include <vector>
#include <set>

class SensorManager {
public:
    typedef int EventType;
    static const EventType NONE = 0;
    static const EventType INIT = 1;
    static const EventType PERIODIC = 2;
    static const EventType NO_WAY = 4;
    static const EventType CHANGED_ROOM = 8;


    typedef std::vector<std::pair<AbstractSensor *, EventType>> SensorContainer;

    /****************************
     * Constructors & Destructors
     ****************************/
    SensorManager(const Building * building, CognitiveMapStorage *);
    virtual ~SensorManager();

    void Register(AbstractSensor *, EventType);
    void execute(const Pedestrian *, EventType);

    static SensorManager * InitWithAllSensors(const Building *, CognitiveMapStorage *);

private:
    const Building * const building;
    CognitiveMapStorage * cm_storage;

    SensorContainer registered_sensors;
};


#endif /* SENSORMANAGER_H_ */
