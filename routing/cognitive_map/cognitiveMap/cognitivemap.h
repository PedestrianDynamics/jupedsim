#ifndef COGNITIVEMAP_H
#define COGNITIVEMAP_H

#ifndef UPDATE_RATE
#define UPDATE_RATE 2.0
#endif


#include "associations.h"
#include "waypoints.h"
#include "landmark.h"
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
  bool operator() (const ptrWaypoint& lhs, const ptrWaypoint& rhs) const
  {
    if (lhs->GetPriority() > rhs->GetPriority())
      return true;
    else
        return false;
  }
};


using SortedWaypoints = std::priority_queue<ptrWaypoint,std::vector<ptrWaypoint>,priorityCheck>;
using Waypoints = std::vector<ptrWaypoint>;
using ptrBuilding = const Building*;
using ptrPed = const Pedestrian*;
using ptrLandmark = std::shared_ptr<Landmark>;
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
    // Landmarks
    void AddLandmarksSC(std::vector<ptrLandmark> landmarks);
    void AddLandmarks(std::vector<ptrLandmark> landmarks);
    std::vector<ptrLandmark> LookForLandmarks();
    // Waypoints and associations
    Waypoints TriggerAssociations(const std::vector<ptrLandmark> &landmarks);
    void AddWaypoints(Waypoints waypoints);
    void AssessDoors();
    // Calculations
    std::vector<GraphEdge *> SortConShortestPath(ptrWaypoint waypoint, const GraphVertex::EdgesContainer edges);
    //bool IsAroundWaypoint(const Waypoint& waypoint, GraphEdge* edge) const;
    ptrGraphNetwork GetGraphNetwork() const;
    double ShortestPathDistance(const GraphEdge *edge, const ptrWaypoint waypoint);
    const Point& GetOwnPos();
    //WriteXML
    void WriteToFile();
    //evaluate waypoints

    //Connections
    std::vector<ptrConnection> GetAllConnections() const;
    void AddConnection(const ptrConnection &connection);
    void AddConnection(const ptrWaypoint& waypoint1, const ptrWaypoint& waypoint2);
    void RemoveConnections(const ptrWaypoint& waypoint);
    Waypoints ConnectedWith(const ptrWaypoint& waypoint) const;

private:
    ptrBuilding _building;
    ptrPed _ped;
    ptrGraphNetwork _network;
    Associations _assoContainer;
    std::vector<ptrLandmark> _landmarksSubConcious;
    std::vector<ptrLandmark> _landmarks;
    SortedWaypoints _waypContainerSorted;
    Waypoints _waypContainer;
    YouAreHerePointer _YAHPointer;
    ptrOutputHandler _outputhandler;
    Connections _connections;

    int _frame;


};

#endif // COGNITIVEMAP_H
