#pragma once

#include "geometry/Point.h"
#include "pedestrian/Ellipse.h"
#include "geometry/Building.h"

#include <vector>
#include <list>

using ptrRoom = SubRoom*;

class Association;
using ptrAssociation = std::shared_ptr<Association>;
using Associations = std::vector<ptrAssociation>;

class Region;
using ptrRegion = std::shared_ptr<Region>;

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
    void SetRealPos(const Point& point);
    void SetPosInMap(const Point& point);
    void SetRoom(ptrRoom room);
    void SetCaption(const std::string& string);
    void SetPriority(double priority);
    void SetType(const std::string &type);
    //Getter
    const int& GetId() const;
    const Point& GetRealPos() const;
    const Point& GetPosInMap() const;
    const double& GetA() const;
    const double& GetB() const;
    const std::string& GetType() const;
    ptrRoom GetRoom() const;
    const std::string& GetCaption() const;
    const double &GetPriority() const;
    //Random point somewhere within the waypoint
    Point GetRandomPoint() const;
    // Shortest Distance from waypoint egde (ellipse) to specific point
    Point PointOnShortestRoute(const Point &point) const;
    // Check if Waypoint reached (if YAH-Pointer is in Waypoint)
    //bool LandmarkReached(const Point& currentYAH);

    bool Visited() const;
    void SetVisited(bool stat);

    //check if landmark ellipse in cogmap contains certain point
    bool Contains(const Point &point);

    // Associations
    Associations GetAssociations() const;
    void AddAssociation(ptrAssociation asso);

    //Region
    const ptrRegion IsInRegion() const;
    void SetRegion(ptrRegion region);



private:
    int _id;
    std::string _caption;
    Point _realPos;
    Point _posInMap;
    double _a;
    double _b;
    ptrRoom _room;
    double _priority;
    bool _visited;
    std::string _type;
    std::list<int> _connectedWith;
    Associations _assoContainer;
    ptrRegion _region;
    JEllipse _ellipse;
};
