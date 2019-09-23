#include "locater.h"

#include "pedestrian/Pedestrian.h"
#include "router/smoke_router/cognitiveMap/cognitivemap.h"

Locater::Locater(const Building *b, const double &updateInterval): AbstractSensor(b)
{
    _updateInterval=updateInterval;
}

Locater::~Locater()
{

}

std::string Locater::GetName() const
{
    return "Locater";
}

void Locater::execute(const Pedestrian * ped, CognitiveMap &cogMap) const
{
   //Log->Write("INFO:\t Execute Locater");
   //cogMap->UpdateDirection();

   //std::cout << std::to_string(ped->GetGlobalTime()) << std::endl;
   cogMap.UpdateYAHPointer(ped->GetV()*_updateInterval);

//   if (std::fmod(ped->GetGlobalTime(),5.0)==0.0)
//   {
//       cogMap->SetNewWaypoint();
//   }

   //cogMap->WriteToFile();

}
