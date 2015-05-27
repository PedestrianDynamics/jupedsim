#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <vector>
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Ellipse.h"

//

class Association;
using Associations = std::vector<Association*>;
using ptrRoom = std::shared_ptr<const Room>;

class Waypoint
{
public:
    Waypoint(Point pos, JEllipse ellipse, ptrRoom room);
    ~Waypoint();

    ///Setter
    void SetId(const int id);
    void SetPos(const Point& point);
    void SetEllipse(const JEllipse& ellipse);
    void SetRoom(ptrRoom room);
    void SetCaption(const std::string& string);
    ///Getter
    const int& GetId();
    const Point& GetPos();
    const JEllipse& GetEllipse();
    ptrRoom GetRoom();
    const std::string& GetCaption();

private:
    int _id;
    std::string _caption;
    Point _exactPos;
    JEllipse _catchmentArea;
    Room _room;
    Associations _assoContainer;
};

#endif // WAYPOINTS_H
