#ifndef LOCATER_H
#define LOCATER_H


#include "AbstractSensor.h"
//class Point;

class Locater : public AbstractSensor
{
public:
    Locater(const Building * b);
    ~Locater();

    std::string GetName() const;
    void execute(const Pedestrian *ped, CognitiveMap *cogMap) const;
};

#endif // LOCATER_H
