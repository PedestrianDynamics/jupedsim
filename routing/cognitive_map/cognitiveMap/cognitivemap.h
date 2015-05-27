#ifndef COGNITIVEMAP_H
#define COGNITIVEMAP_H


#include "associations.h"
#include "waypoints.h"

#include <queue>
#include<memory>
#include <vector>

class Pedestrian;
class Building;
class JEllipse;


//class priorityCheck
//{
//public:
//  priorityCheck(){}
//  bool operator() (const Waypoint& lhs, const Waypoint& rhs) const
//  {
//    if (lhs.getPriority <= rhs.getPriority)
//      return true;
//    else
//        return false;
//  }
//};





//using Waypoints = std::priority_queue<Waypoint,std::vector<Waypoint>,priorityCheck>;
using ptrBuilding = std::shared_ptr<const Building>;
using ptrPed = std::shared_ptr<const Pedestrian>;
using ptrLandmark = std::shared_ptr<Landmark>;


class CognitiveMap
{
public:
    CognitiveMap();
    CognitiveMap(ptrBuilding b, ptrPed ped);
    ~CognitiveMap();
private:
    void ParseLandmarks();


private:
    ptrBuilding _ped;
    ptrPed _building;
    // Navigation graph
    Associations _assoContainer;
    //Waypoints _waypContainer;


};

#endif // COGNITIVEMAP_H
