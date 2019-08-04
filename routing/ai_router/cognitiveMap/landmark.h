#pragma once

#include "associations.h"
#include "geometry/Point.h"

#include <vector>
#include <list>

using AIAssociations = std::vector<AIAssociation>;

class AIRegion;


class AILandmark
{

public:
    AILandmark(const Point& pos);
    AILandmark(const Point &pos, double a, double b, int id=-1);
    ~AILandmark();



    //Setter
    void SetId(int id);
    void SetA(double a);
    void SetB(double b);
    void SetRealPos(const Point& point);
    void SetPosInMap(const Point& point);
    void SetCaption(const std::string& string);
//    void SetPriority(double priority);
    void SetType(const std::string &type);
    //Getter
    int GetId() const;
    const Point& GetRealPos() const;
    const Point& GetPosInMap() const;
    double GetA() const;
    double GetB() const;
    const std::string& GetType() const;
    const std::string& GetCaption() const;
    //double GetPriority() const;
    //Random point somewhere within the waypoint
    Point GetRandomPoint() const;
    // Shortest Distance from waypoint egde (ellipse) to specific point
    Point PointOnShortestRoute(const Point &point) const;
    // Check if Waypoint reached (if YAH-Pointer is in Waypoint)
    //bool LandmarkReached(const Point& currentYAH);

    bool Visited() const;
    void SetVisited(bool stat);

    //check if landmark ellipse in cogmap contains certain point
    bool Contains(const Point &point) const;

    // Associations
    const AIAssociations& GetAssociations() const;
    void AddAssociation(const AIAssociation& asso);

    //Region
    const AIRegion* IsInRegion() const;
    void SetRegion(const AIRegion* region);



private:
    int _id;
    std::string _caption;
    Point _realPos;
    Point _posInMap;
    double _a;
    double _b;
    //double _priority;
    bool _visited;
    std::string _type;
    std::list<int> _connectedWith;
    AIAssociations _assoContainer;
    const AIRegion* _region;
};
