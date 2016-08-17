#ifndef BRAIN_H
#define BRAIN_H

#include "cognitiveMap/cognitivemap.h"
#include "./perception/visibleenvironment.h"

class InternNavigationNetwork;

using ptrIntNetwork = std::shared_ptr<InternNavigationNetwork>;
using ptrEnvironment = std::shared_ptr<VisibleEnvironment>;
using ptrPed = const Pedestrian*;

class Brain
{   

public:
    Brain();
    Brain(ptrPed ped, ptrEnvironment env, const std::map<const SubRoom*,ptrIntNetwork>& _roominternalNetworks);

private:
    ptrPed _ped;
    CognitiveMap _cMap;
    //whole environment
    ptrEnvironment _wholeEnvironment;


};

#endif // BRAIN_H
