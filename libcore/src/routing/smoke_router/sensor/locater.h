#pragma once

#include "AbstractSensor.h"

class Locater : public AbstractSensor
{
public:
    Locater(const Building * b, const double & updateInterval = 1.0);
    ~Locater();

    std::string GetName() const override;
    void execute(const Pedestrian * ped, CognitiveMap & cogMap, double time) const override;

private:
    double _updateInterval;
};
