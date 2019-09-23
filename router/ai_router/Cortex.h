#pragma once

#include "router/ai_router/cognitiveMap/cognitivemap.h"
#include "router/ai_router/perception/visibleenvironment.h"
#include "router/ai_router/cognitiveMap/internnavigationnetwork.h"
#include "router/ai_router/perception/visualsystem.h"

using ptrIntNetwork = std::unique_ptr<InternNavigationNetwork>;

class Cortex
{

public:
    Cortex();
    Cortex(const Building* b,const Pedestrian* ped, const VisibleEnvironment* env, std::unordered_map<const SubRoom*, ptrIntNetwork>* roominternalNetworks);

    AICognitiveMap& GetCognitiveMap();
    VisualSystem& GetPerceptionAbilities();

    //to enable hline handling
    const NavLine* GetNextNavLine(const NavLine *nextTarget);
    bool HlineReached() const;

    // Sort crossings by considering general knowledge
    std::vector<const NavLine*> SortConGeneralKnow(const std::vector<const NavLine *> &navLines);

    //select appropriate crossing to unknown area
    const NavLine FindApprCrossing(const std::vector<NavLine> &navLines);
    // Select appropriate visible crossing
    const NavLine FindApprVisibleCrossing(const NavLine& navLine, const std::vector<NavLine> &navLines);
    // Get Operational target (correct visible crossing)
    const NavLine TargetToHead(const NavLine *visibleCrossing) const;

    //implement sign instruction
    // Sort crossings by considering percepted signs
    std::vector<const NavLine *> SortConSignage(const std::vector<const NavLine *>& navLines);
    const Sign* DetermineDecisiveSign(const std::vector<const Sign *> &signs);
    double OrthogonalDistanceFromTo(const Sign& sign1, const Sign& sign2) const;
    double OrthogonalDistanceFromTo(const NavLine* navLine, const Sign &sign) const;
    double OrthogonalDistanceFromTo(const NavLine* navLine, const Point& pos, const double& angle) const;

private:
    const Building* _b;
    const Pedestrian* _ped;
    AICognitiveMap _cMap;
    //whole environment
    const VisibleEnvironment* _wholeEnvironment;
    // reference of roominternalNetwork
    std::unordered_map<const SubRoom*, ptrIntNetwork>* _intNetworks;
    InternNavigationNetwork _currentIntNetwork;
    //perception abilities
    VisualSystem _perceptionAbilities;
    NavLine _lastBestVisNavLine;
    bool _statPreferToGoDown;
    std::vector<const Sign*> _notUsedSigns;



};

bool IsInsideRectangle(const Point& point, const Point& leftUp, const Point& rightDown);
