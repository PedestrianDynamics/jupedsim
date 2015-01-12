/**
 * @file SmokeSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 05, 2014
 * @brief detects smoke in the next rooms
 */


#ifndef SMOKESENSOR_H
#define SMOKESENSOR_H 1

#include "AbstractSensor.h"


class SmokeSensor : public AbstractSensor
{

public:
    SmokeSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~SmokeSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;
private:

};

#endif // SMOKESENSOR_H
