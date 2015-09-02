#ifndef ASSOCIATIONS_H
#define ASSOCIATIONS_H

#include<memory>
class Waypoint;
using ptrWaypoint = std::shared_ptr<Waypoint>;
class Connection;
using ptrConnection = std::shared_ptr<Connection>;

class Association
{
public:
    Association();
    Association(ptrWaypoint waypoint, ptrWaypoint associated_waypoint, bool connected=false);
    Association(ptrConnection connection);
    ~Association();
    ptrWaypoint GetWaypointAssociation(ptrWaypoint waypoint) const;
    ptrConnection GetConnectionAssoziation() const;
private:
    ptrWaypoint _waypoint;
    ptrWaypoint _associatedWaypoint;
    ptrConnection _connection;
};

#endif // ASSOCIATIONS_H
