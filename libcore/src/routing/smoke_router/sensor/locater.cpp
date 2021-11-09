#include "locater.h"

#include "pedestrian/Pedestrian.h"
#include "routing/smoke_router/cognitiveMap/cognitivemap.h"

Locater::Locater(const Building * b, const double & updateInterval) : AbstractSensor(b)
{
    _updateInterval = updateInterval;
}

Locater::~Locater() {}

std::string Locater::GetName() const
{
    return "Locater";
}

void Locater::execute(const Pedestrian * ped, CognitiveMap & cogMap, double /*time*/) const
{
    cogMap.UpdateYAHPointer(ped->GetV() * _updateInterval);
}
