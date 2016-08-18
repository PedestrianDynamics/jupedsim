#include "brain.h"

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


