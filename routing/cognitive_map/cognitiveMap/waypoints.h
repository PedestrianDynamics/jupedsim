#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include <vector>
#include "../../../geometry/Point.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Ellipse.h"

//

class Association;
using ptrAssociation = std::shared_ptr<Association>;
using Associations = std::vector<ptrAssociation>;

using ptrRoom = std::shared_ptr<const Room>;

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
    ///Getter
    const int& GetId();
    const Point& GetPos();
    const double& GetA();
    const double& GetB();
    ptrRoom GetRoom();
    const std::string& GetCaption();
    /// Associations
    void AddAssociation(Association asso);

private:
    int _id;
    std::string _caption;
    Point _exactPos;
    double _a;
    double _b;
    Room _room;
    Associations _assoContainer;
};

#endif // WAYPOINTS_H
