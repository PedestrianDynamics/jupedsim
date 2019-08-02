#pragma once

#include "associations.h"
#include "region.h"
#include "geometry/NavLine.h"

#include <memory>
#include <vector>


class Pedestrian;
class VisualSystem;
using AIRegions = std::vector<AIRegion>;


class AICognitiveMap
{
public:
    AICognitiveMap();
    AICognitiveMap(const Pedestrian* ped, const VisualSystem* perceptionalAbilities);
    //Map Updates
    void UpdateMap();
    //Regions
    void AddRegions(const AIRegions &regions);
    void AddRegion(const AIRegion &region);
    const AIRegion* GetRegionByID(int regionID) const;
    // Landmarks
    void AddLandmarkInRegion(const AILandmark& landmark, AIRegion& region);

    // Associations
    std::vector<const AILandmark *> TriggerAssociations(const std::vector<const AILandmark*> &landmarks);

    std::vector<const NavLine *> SortConLeastAngle(const AILandmark* landmark, const std::vector<NavLine> &navLines, const NavLine& focus);

    Point ShortestBeeLine(const Point& pos, const AILandmark* landmark);

    //Tools
    double MakeItFuzzy(double mean, double std);

    //Find region/landmarks/connections
    std::vector<const AILandmark *> GetLandmarksConnectedWith(const AILandmark *landmark) const;
    const AIRegion* GetRegionContaining(const AILandmark* landmark) const;

    //Locater
    void FindCurrentRegion();
    void CheckIfLandmarksReached();

    //Find targets
    const AILandmark* FindConnectionPoint(const AIRegion* currentRegion, const AIRegion* targetRegion) const;
    void FindMainDestination();
    void FindNextTarget();
    const AILandmark *GetNextTarget() const;
    void FindShortCut();
    const AILandmark *FindNearLandmarkConnectedToTarget(const AILandmark *target);
    std::vector<const AILandmark *> FindLandmarksConnectedToTarget(const AILandmark *target);
    const AILandmark *FindBestRouteFromOneOf(const std::vector<const AILandmark *> &nearLandmarks);
    const AILandmark *GetNearestMainTarget(const std::vector<const AILandmark *> &mainTargets);

    void SetBestChoice(const NavLine &navLine);
    const NavLine *GetBestChoice() const;

    //Init LandmarkNetworks
    void InitLandmarkNetworksInRegions();


private:
    const Pedestrian* _ped;
    const VisualSystem* _perceptAbis;

    std::vector<const AILandmark*> _landmarksRecentlyVisited;

    const AILandmark* _mainDestination;
    const AILandmark* _nextTarget;
    AIRegions _regions;
    const AIRegion* _currentRegion;
    const AIRegion* _targetRegion;

    NavLine _lastBestChoice;
    double _lastMinAngleDiff;



};

Point GetCloserPoint(const Point &origin, const Point &target1, const Point& target2);
