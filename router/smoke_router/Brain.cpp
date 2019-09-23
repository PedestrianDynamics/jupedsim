#include "Brain.h"

#include "pedestrian/Pedestrian.h"

Brain::Brain()
{

}

Brain::Brain(const Building *b, const Pedestrian *ped, std::unordered_map<const SubRoom *, ptrIntNetwork> *roominternalNetworks)
{
    _b=b;
    _ped=ped;
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

bool Brain::HlineReached() const
{
    const SubRoom* currentSubRoom = _b->GetSubRoomByUID(_ped->GetSubRoomUID());

    for (Hline* hline:currentSubRoom->GetAllHlines())
    {
        // if distance hline to ped lower than 1 m
        if (hline->DistTo(_ped->GetPos())<1)
        {
            return true;
        }
    }

    return false;
}
