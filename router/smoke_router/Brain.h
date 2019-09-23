#pragma once

#include "router/smoke_router/cognitiveMap/cognitivemap.h"
#include "router/smoke_router/cognitiveMap/internnavigationnetwork.h"

using ptrIntNetwork = std::shared_ptr<InternNavigationNetwork>;

class Brain
{

public:
    Brain();
    Brain(const Building* b, const Pedestrian* ped, std::unordered_map<const SubRoom*, ptrIntNetwork>* roominternalNetworks);

    CognitiveMap& GetCognitiveMap();

    const NavLine* GetNextNavLine(const NavLine *nextTarget);
    bool HlineReached() const;

private:
    const Building* _b;
    const Pedestrian* _ped;
    CognitiveMap _cMap;
    //whole environment
    // reference of roominternalNetwork
    std::unordered_map<const SubRoom*, ptrIntNetwork>* _intNetworks;
    InternNavigationNetwork _currentIntNetwork;


};
