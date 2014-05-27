/**
 * @file JamSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 05, 2014
 * @brief detects smoke in the next rooms
 */


#ifndef JAMSENSOR_H
#define JAMSENSOR_H 1

#include "AbstractSensor.h"


class JamSensor : public AbstractSensor
{

public:
    JamSensor(const Building * b) : AbstractSensor(b) { }

    virtual ~JamSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;
private:

};

#endif // JAMSENSOR_H
