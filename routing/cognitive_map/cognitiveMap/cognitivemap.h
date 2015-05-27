#ifndef COGNITIVEMAP_H
#define COGNITIVEMAP_H


#include "associations.h"
#include "waypoints.h"
#include "landmark.h"

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
    void AddLandmark(ptrLandmark landmark);
private:
    void ParseLandmarks(const std::string &geometryfile);


private:
    ptrBuilding _building;
    ptrPed _ped;
    // Navigation graph
    Associations _assoContainer;
    std::vector<ptrLandmark> _landmarks;
    //Waypoints _waypContainer;


};

#endif // COGNITIVEMAP_H
