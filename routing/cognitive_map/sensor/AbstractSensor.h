/**
 * @file AbstractSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */


#ifndef ABSTRACTSENSOR_H
#define ABSTRACTSENSOR_H 1

#include <string>
class Building;
class Pedestrian;
class CognitiveMap;



class AbstractSensor {
public:
     AbstractSensor(const Building * b) : building(b) {}
     virtual ~AbstractSensor();

     virtual std::string GetName() const = 0;
     virtual void execute(const Pedestrian *, CognitiveMap *) const = 0;
protected:
     const Building * const building;
};

#endif // ABSTRACTSENSOR_H
