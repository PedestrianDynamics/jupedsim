#ifndef CONNECTION_H
#define CONNECTION_H

#include "waypoints.h"
using ptrWaypoint = std::shared_ptr<Waypoint>;

class Connection
{
public:
    Connection(ptrWaypoint waypoint1, ptrWaypoint waypoint2);
    ~Connection();

    std::pair<ptrWaypoint,ptrWaypoint> GetWaypoints() const;

private:
    ptrWaypoint _waypoint1;
    ptrWaypoint _waypoint2;
};

#endif // CONNECTION_H
