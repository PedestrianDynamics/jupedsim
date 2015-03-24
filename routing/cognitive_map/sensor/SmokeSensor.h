/**
 * @file SmokeSensor.h
 * @author David H. <d.haensel@fz-juelich.de>
 * @date 05, 2014
 * @brief detects smoke in the next rooms
 */


#ifndef SMOKESENSOR_H
#define SMOKESENSOR_H 1

#include "AbstractSensor.h"
#include <memory>

class Point;
class FireMeshStorage;

class SmokeSensor : public AbstractSensor
{

public:
    SmokeSensor(const Building * b, const std::string &filepath, const double &updateintervall, const double &finalTime);

    virtual ~SmokeSensor();

    std::string GetName() const;
    void execute(const Pedestrian *, CognitiveMap *) const;

    void set_FMStorage(const std::shared_ptr<FireMeshStorage> fmStorage);
    const std::shared_ptr<FireMeshStorage> get_FMStorage();


private:

    std::shared_ptr<FireMeshStorage> _FMStorage;
};

#endif // SMOKESENSOR_H
