/**
 * @file DiscoverDoorsSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 04, 2014
 * @brief
 */


#ifndef DISCOVERDOORSSENSOR_H
#define DISCOVERDOORSSENSOR_H 1

#include "AbstractSensor.h"


class DiscoverDoorsSensor : public AbstractSensor
{

public:
    DiscoverDoorsSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~DiscoverDoorsSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;
private:

};

#endif // DISCOVERDOORSSENSOR_H
