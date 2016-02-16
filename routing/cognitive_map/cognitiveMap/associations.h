#ifndef ASSOCIATIONS_H
#define ASSOCIATIONS_H

#include<memory>
class Waypoint;
using ptrWaypoint = std::shared_ptr<Waypoint>;

class Association
{
public:
    Association();
    Association(ptrWaypoint waypoint, ptrWaypoint associated_waypoint);
    ~Association();
    ptrWaypoint GetAssociation(ptrWaypoint waypoint);
private:
    ptrWaypoint _waypoint;
    ptrWaypoint _associatedWaypoint;
};

#endif // ASSOCIATIONS_H
