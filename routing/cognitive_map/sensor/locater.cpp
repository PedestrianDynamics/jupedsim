#include "locater.h"
#include "../cognitiveMap/cognitivemap.h"
#include "../../../pedestrian/Pedestrian.h"

Locater::Locater(const Building *b): AbstractSensor(b)
{

}

Locater::~Locater()
{

}

std::string Locater::GetName() const
{
    return "Locater";
}

void Locater::execute(const Pedestrian * ped, CognitiveMap * cogMap) const
{
   //Log->Write("INFO:\t Update Pointer");
   cogMap->UpdateYAHPointer(ped->GetPos());
}

