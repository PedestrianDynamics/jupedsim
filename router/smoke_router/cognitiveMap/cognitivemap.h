#pragma once

#ifndef UPDATE_RATE
#define UPDATE_RATE 1.0
#endif

#include "associations.h"
#include "region.h"
#include "router/smoke_router/GraphNetwork.h"
#include "youareherepointer.h"
#include "cogmapoutputhandler.h"

#include <queue>
#include <memory>
#include <vector>

class Pedestrian;
class Building;

//class priorityCheck
//{
//public:
//  priorityCheck(){}
//  bool operator() (const ptrLandmark& lhs, const ptrLandmark& rhs) const
//  {
//    if (lhs->GetPriority() > rhs->GetPriority())
//      return true;
//    else
//        return false;
//  }
//};


//using SortedLandmarks = std::priority_queue<ptrLandmark,std::vector<ptrLandmark>,priorityCheck>;
using Landmarks = std::vector<ptrLandmark>;
using Regions = std::vector<ptrRegion>;
using ptrGraphNetwork = std::shared_ptr<GraphNetwork>;
using ptrOutputHandler = std::shared_ptr<CogMapOutputHandler>;


class CognitiveMap
{
public:
    CognitiveMap();
    CognitiveMap(const Building* b, const Pedestrian* ped);
    ~CognitiveMap();
    //Map Updates
    void UpdateMap();
    void UpdateYAHPointer(const Point &move);
    //Regions
    void AddRegions(Regions regions);
    void AddRegion(ptrRegion region);
    ptrRegion GetRegionByID(const int& regionID) const;
    // Landmarks
    void AddLandmarksSC(std::vector<ptrLandmark> landmarks);
    //void AddLandmarks(std::vector<ptrLandmark> landmarks);
    void AddLandmarkInRegion(ptrLandmark landmark, ptrRegion region);
    //std::vector<ptrLandmark> LookForLandmarks();
    // Associations
    Landmarks TriggerAssociations(const std::vector<ptrLandmark> &landmarks);
    //void AddAssociatedLandmarks(Landmarks landmarks);
    void AssessDoors();
    // Calculations
    std::vector<GraphEdge *> SortConShortestPath(ptrLandmark landmark, const GraphVertex::EdgesContainer edges);
    //bool IsAroundLandmark(const Landmark& landmark, GraphEdge* edge) const;
    ptrGraphNetwork GetGraphNetwork() const;
    //shortest path calculations
    double ShortestPathDistance(const GraphEdge *edge, const ptrLandmark landmark);
    bool LineIntersectsPolygon(const std::pair<Point,Point> &line, const boost::geometry::model::polygon<Point> &polygon);
    //Own position
    const Point& GetOwnPos();
    //WriteXML
    void WriteToFile();

    //Tools
    double MakeItFuzzy(const double& mean, const double& std);
    //std::vector<Point> StartFromLLCorner(std::vector<Point>&  polygon);

    // Set new Landmarks
    //void SetNewLandmark();

    //Find region/landmarks/connections
    Landmarks GetLandmarksConnectedWith(const ptrLandmark& landmark) const;
    const ptrRegion GetRegionContaining(const ptrLandmark& landmark) const;

    //Locater
    void FindCurrentRegion();
    void CheckIfLandmarksReached();

    //Find targets
    const ptrLandmark FindConnectionPoint(const ptrRegion& currentRegion, const ptrRegion& targetRegion) const;
    void FindMainDestination();
    void FindNextTarget();
    void FindShortCut();
    const ptrLandmark FindNearLandmarkConnectedToTarget(const ptrLandmark& target);
    Landmarks FindLandmarksConnectedToTarget(const ptrLandmark& target);
    const ptrLandmark FindBestRouteFromOneOf(const Landmarks& nearLandmarks);
    const ptrLandmark GetNearestMainTarget(const Landmarks& mainTargets);

    //Init LandmarkNetworks
    void InitLandmarkNetworksInRegions();


private:
    const Building* _building;
    const Pedestrian* _ped;
    ptrGraphNetwork _network;
    Associations _assoContainer;
    std::vector<ptrLandmark> _landmarksSubConcious;
    //std::vector<ptrLandmark> _landmarks;
    //SortedLandmarks _waypContainerTargetsSorted;
    std::vector<ptrLandmark> _landmarksRecentlyVisited;
    Landmarks _waypContainer;
    YouAreHerePointer _YAHPointer;
    ptrOutputHandler _outputhandler;

    ptrLandmark _mainDestination;
    ptrLandmark _nextTarget;
    Regions _regions;
    ptrRegion _currentRegion;
    ptrRegion _targetRegion;

    // for shortest path calculations
    std::vector<Point> _outerBoundary;


    int _frame;
    int _createdWayP;


};
