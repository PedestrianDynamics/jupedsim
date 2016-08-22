#include "Brain.h"
#include "../../pedestrian/Pedestrian.h"

Brain::Brain()
{

}

Brain::Brain(const Building *b, const Pedestrian *ped, const VisibleEnvironment *env, std::unordered_map<const SubRoom *, ptrIntNetwork> *roominternalNetworks)
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
    const SubRoom* currentSubRoom = _b->GetSubRoomByUID(_ped->GetSubRoomUID());

    _currentIntNetwork=*(_intNetworks->operator [](currentSubRoom));

    return _currentIntNetwork.GetNextNavLineOnShortestPathToTarget(_ped->GetPos(),nextTarget);
}


