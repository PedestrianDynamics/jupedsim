#ifndef COGNITIVEMAP_H
#define COGNITIVEMAP_H

#ifndef UPDATE_RATE
#define UPDATE_RATE 2.0
#endif


#include "associations.h"
#include "region.h"
#include "../GraphNetwork.h"
#include "youareherepointer.h"
#include "cogmapoutputhandler.h"
#include "connection.h"


#include <queue>
#include <memory>
#include <vector>


class Pedestrian;
class Building;




class priorityCheck
{
public:
  priorityCheck(){}
  bool operator() (const ptrLandmark& lhs, const ptrLandmark& rhs) const
  {
    if (lhs->GetPriority() > rhs->GetPriority())
      return true;
    else
        return false;
  }
};


using SortedLandmarks = std::priority_queue<ptrLandmark,std::vector<ptrLandmark>,priorityCheck>;
using Landmarks = std::vector<ptrLandmark>;
using Regions = std::vector<ptrRegion>;
using ptrBuilding = const Building*;
using ptrPed = const Pedestrian*;
using ptrGraphNetwork = std::shared_ptr<GraphNetwork>;
using ptrOutputHandler = std::shared_ptr<CogMapOutputHandler>;
using ptrConnection = std::shared_ptr<Connection>;
using Connections = std::list<ptrConnection>;


class CognitiveMap
{
public:
    CognitiveMap();
    CognitiveMap(ptrBuilding b, ptrPed ped);
    ~CognitiveMap();
    //Map Updates
    void UpdateMap();
    void UpdateDirection();
    void UpdateYAHPointer(const Point &move);
    //Regions
    void AddRegions(Regions regions);
    void AddRegion(ptrRegion region);
    ptrRegion GetRegionByID(const int& regionID) const;
    // Landmarks
    void AddLandmarksSC(std::vector<ptrLandmark> landmarks);
    void AddLandmarks(std::vector<ptrLandmark> landmarks);
    void AddLandmarkInRegion(ptrLandmark landmark, ptrRegion region);
    std::vector<ptrLandmark> LookForLandmarks();
    // Associations
    Landmarks TriggerAssociations(const std::vector<ptrLandmark> &landmarks);
    void AddAssociatedLandmarks(Landmarks landmarks);
    void AssessDoors();
    // Calculations
    std::vector<GraphEdge *> SortConShortestPath(ptrLandmark landmark, const GraphVertex::EdgesContainer edges);
    //bool IsAroundLandmark(const Landmark& landmark, GraphEdge* edge) const;
    ptrGraphNetwork GetGraphNetwork() const;
    double ShortestPathDistance(const GraphEdge *edge, const ptrLandmark landmark);
    const Point& GetOwnPos();
    //WriteXML
    void WriteToFile();
    //evaluate Landmarks

    // Set new Landmarks
    void SetNewLandmark();

    void LandmarkReached(ptrLandmark landmark);

    //Connections
    std::vector<ptrConnection> GetAllConnections() const;
    void AddConnection(const ptrConnection &connection);
    void AddConnection(const ptrLandmark& landmark1, const ptrLandmark& landmark2);
    void RemoveConnections(const ptrLandmark& landmark);
    Landmarks ConnectedWith(const ptrLandmark& landmark) const;

    //Locater

    void FindCurrentRegion();


    //Find targets

    const ptrLandmark FindConnectionPoint(const ptrRegion& regionA, const ptrRegion& regionB) const;
    void FindMainDestination();
    void FindNextTarget();
    const ptrLandmark FindNearLandmarkConnectedToTarget(const ptrLandmark& target);
    Landmarks FindLandmarksConnectedToTarget(const ptrLandmark& target);



private:
    ptrBuilding _building;
    ptrPed _ped;
    ptrGraphNetwork _network;
    Associations _assoContainer;
    std::vector<ptrLandmark> _landmarksSubConcious;
    std::vector<ptrLandmark> _landmarks;
    SortedLandmarks _waypContainerTargetsSorted;
    std::vector<ptrLandmark> _landmarksRecentlyVisited;
    Landmarks _waypContainer;
    YouAreHerePointer _YAHPointer;
    ptrOutputHandler _outputhandler;
    Connections _connections;
    ptrLandmark _mainDestination;
    ptrLandmark _nextTarget;
    Regions _regions;
    ptrRegion _currentRegion;
    ptrRegion _targetRegion;


    int _frame;
    int _createdWayP;


};

#endif // COGNITIVEMAP_H
