/**
 * @file RoomToFloorSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 02, 2014
 * @brief
 */


#ifndef ROOMTOFLOORSENSOR_H
#define ROOMTOFLOORSENSOR_H 1

#include "AbstractSensor.h"


class RoomToFloorSensor : public AbstractSensor
{

public:
    RoomToFloorSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~RoomToFloorSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;
private:

};

#endif // ROOMTOFLOORSENSOR_H
