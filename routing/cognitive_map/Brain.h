#ifndef BRAIN_H
#define BRAIN_H

#include "cognitiveMap/cognitivemap.h"
#include "./perception/visibleenvironment.h"

class InternNavigationNetwork;

using ptrIntNetwork = std::shared_ptr<InternNavigationNetwork>;
using ptrEnv = std::shared_ptr<const VisibleEnvironment>;

using ptrSubRoom = std::shared_ptr<const SubRoom>;
using ptrIntNetworks = std::shared_ptr<const std::map<ptrSubRoom, ptrIntNetwork>>;

class Brain
{   

public:
    Brain();
    Brain(ptrBuilding b,ptrPed ped, ptrEnv env, ptrIntNetworks roominternalNetworks);

    CognitiveMap& GetCognitiveMap();

private:
    ptrBuilding _b;
    ptrPed _ped;
    CognitiveMap _cMap;
    //whole environment
    ptrEnv _wholeEnvironment;
    // reference of roominternalNetwork
    ptrIntNetworks _intNetworks;


};

#endif // BRAIN_H
