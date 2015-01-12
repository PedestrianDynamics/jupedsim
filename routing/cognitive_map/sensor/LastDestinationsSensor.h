/**
 * @file LastDestinationsSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 04, 2014
 * @brief
 */


#ifndef LASTDESTINATIONSSENSOR_H
#define LASTDESTINATIONSSENSOR_H 1

#include "AbstractSensor.h"


class LastDestinationsSensor : public AbstractSensor
{

public:
    LastDestinationsSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~LastDestinationsSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;
private:

};

#endif // LASTDESTINATIONSSENSOR_H
