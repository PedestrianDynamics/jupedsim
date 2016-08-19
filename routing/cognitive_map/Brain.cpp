#include "Brain.h"
#include "./cognitiveMap/internnavigationnetwork.h"
#include "../../pedestrian/Pedestrian.h"

Brain::Brain()
{

}

Brain::Brain(ptrBuilding b, ptrPed ped, ptrEnv env, ptrIntNetworks roominternalNetworks)
{
    _b=b;
    _ped=ped;
    _wholeEnvironment=env;
    _intNetworks=roominternalNetworks;
    _cMap=CognitiveMap(_b,_ped);
}

CognitiveMap &Brain::GetCognitiveMap()
{
    return _cMap;
}

const NavLine* Brain::GetNextNavLine(const NavLine* nextTarget)
{
    ptrSubRoom currentSubRoom = std::shared_ptr<const SubRoom>(_b->GetSubRoomByUID(_ped->GetSubRoomID()));

    _currentIntNetwork=_intNetworks->operator [](currentSubRoom);

    return (_currentIntNetwork->GetNextNavLineOnShortestPathToTarget(_ped->GetPos(),std::shared_ptr<const NavLine>(nextTarget))).get();
}


