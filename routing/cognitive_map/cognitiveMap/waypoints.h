#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <vector>
#include <list>
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"


using ptrRoom = SubRoom*;

class Association;
using ptrAssociation = std::shared_ptr<Association>;
using Associations = std::vector<ptrAssociation>;
using ptrWaypoint = std::shared_ptr<Waypoint>;
using Waypoints = std::vector<ptrWaypoint>;

class Landmark
{

public:
    Landmark(Point pos, ptrRoom room=nullptr);
    Landmark(Point pos, double a, double b, int id=-1, ptrRoom room=nullptr);
    ~Landmark();

    //Setter
    void SetId(int id);
    void SetA(double a);
    void SetB(double b);
    void SetPos(const Point& point);
    void SetRoom(ptrRoom room);
    void SetCaption(const std::string& string);
    void SetPriority(double priority);
    //Getter
    const int& GetId() const;
    const Point& GetPos() const;
    const double& GetA() const;
    const double& GetB() const;
    ptrRoom GetRoom() const;
    const std::string& GetCaption() const;
    const double &GetPriority() const;
    //Random point somewhere within the waypoint
    Point GetRandomPoint() const;
    // Shortest Distance from waypoint egde (ellipse) to specific point
    Point PointOnShortestRoute(const Point &point) const;
    // Check if Waypoint reached (if YAH-Pointer is in Waypoint)
    bool LandmarkReached(const Point& currentYAH);

    bool Visited() const;
    void SetVisited(bool stat);


    // Associations
    Associations GetAssociations() const;
    void AddAssociation(ptrAssociation asso);






private:
    int _id;
    std::string _caption;
    Point _exactPos;
    double _a;
    double _b;
    ptrRoom _room;
    double _priority;
    bool _visited;
    std::list<int> _connectedWith;
    Associations _assoContainer;
};

#endif // WAYPOINTS_H
