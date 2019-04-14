#ifndef BRAIN_H
#define BRAIN_H

#include "cognitiveMap/cognitivemap.h"
#include "./cognitiveMap/internnavigationnetwork.h"


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

#endif // BRAIN_H
