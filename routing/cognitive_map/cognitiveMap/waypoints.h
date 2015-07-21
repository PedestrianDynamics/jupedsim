#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <vector>
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Ellipse.h"


using ptrRoom = SubRoom*;

class Waypoint
{
public:
    Waypoint(Point pos, ptrRoom room=nullptr);
    Waypoint(Point pos, double a, double b, ptrRoom room=nullptr);
    ~Waypoint();

    ///Setter
    void SetId(int id);
    void SetA(double a);
    void SetB(double b);
    void SetPos(const Point& point);
    void SetRoom(ptrRoom room);
    void SetCaption(const std::string& string);
    void SetPriority(double priority);
    ///Getter
    const int& GetId() const;
    const Point& GetPos() const;
    const double& GetA() const;
    const double& GetB() const;
    ptrRoom GetRoom() const;
    const std::string& GetCaption() const;
    const double &GetPriority() const;
    ///Random point somewhere within the waypoint
    Point GetRandomPoint() const;
    // Shortest Distance from waypoint egde (ellipse) to arbitrary point
    //double ShortestDistance(Point point) const;
    /// Check if Waypoint reached (if YAH-Pointer is in Waypoint)
    bool WaypointReached() const;


private:
    int _id;
    std::string _caption;
    Point _exactPos;
    double _a;
    double _b;
    ptrRoom _room;
    double _priority;
};

#endif // WAYPOINTS_H
