#include "Brain.h"
#include "../../pedestrian/Pedestrian.h"

Brain::Brain()
{

}

Brain::Brain(const Building *b, const Pedestrian *ped, const VisibleEnvironment *env, std::unordered_map<std::shared_ptr<const SubRoom>, ptrIntNetwork>* roominternalNetworks)
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
    std::shared_ptr<const SubRoom> currentSubRoom = std::shared_ptr<const SubRoom>(_b->GetSubRoomByUID(_ped->GetSubRoomUID()));

    _currentIntNetwork=*(_intNetworks->operator [](currentSubRoom));

    return _currentIntNetwork.GetNextNavLineOnShortestPathToTarget(_ped->GetPos(),nextTarget);
}


