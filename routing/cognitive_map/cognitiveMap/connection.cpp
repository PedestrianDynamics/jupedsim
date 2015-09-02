#include "connection.h"
#include <utility>

Connection::Connection(ptrWaypoint waypoint1, ptrWaypoint waypoint2)
{
    _waypoint1=waypoint1;
    _waypoint2=waypoint2;
}

Connection::~Connection()
{

}

std::pair<ptrWaypoint, ptrWaypoint> Connection::GetWaypoints() const
{
    return std::make_pair(_waypoint1, _waypoint2);

}

