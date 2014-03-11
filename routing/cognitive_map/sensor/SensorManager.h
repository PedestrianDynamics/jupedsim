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


class SensorManager {
public:
    typedef std::vector<AbstractSensor *> SensorContainer;
    /****************************
     * Constructors & Destructors
     ****************************/
    SensorManager(const Building * building, CognitiveMapStorage *);
    virtual ~SensorManager();

    void Register(AbstractSensor * sensor);
    void execute(const Pedestrian *);

    static SensorManager * InitWithAllSensors(const Building *, CognitiveMapStorage *);

private:
    const Building * const building;
    CognitiveMapStorage * cm_storage;

    SensorContainer registered_sensors;
};


#endif /* SENSORMANAGER_H_ */
