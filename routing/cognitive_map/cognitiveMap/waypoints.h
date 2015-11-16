#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <vector>
#include <list>
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"


using ptrRoom = SubRoom*;


class Waypoint
{
    using ptrWaypoint = std::shared_ptr<Waypoint>;
    using Waypoints = std::vector<ptrWaypoint>;
public:
    Waypoint(Point pos, ptrRoom room=nullptr);
    Waypoint(Point pos, double a, double b, int id=-1, ptrRoom room=nullptr);
    ~Waypoint();

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
    bool WaypointReached(const Point& currentYAH);

    bool Visited() const;
    void SetVisited(bool stat);







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
};

#endif // WAYPOINTS_H
