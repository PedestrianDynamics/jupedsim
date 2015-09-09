#include "associations.h"
#include "connection.h"



Association::Association()
{
    _waypoint=nullptr;
    _associatedWaypoint=nullptr;

}

Association::Association(ptrWaypoint waypoint, ptrWaypoint associated_waypoint, bool connected)
{
    _waypoint=waypoint;
    _associatedWaypoint=associated_waypoint;

    if (connected)
        _connection = std::make_shared<Connection>(_waypoint, _associatedWaypoint);

    else
        _connection=nullptr;


}

Association::Association(ptrConnection connection)
{
    _connection=connection;
    _waypoint=nullptr;
    _associatedWaypoint=nullptr;
}

Association::~Association()
{

}

ptrWaypoint Association::GetWaypointAssociation(ptrWaypoint waypoint) const
{
    if (waypoint==nullptr)
        return nullptr;
    if (_waypoint==waypoint)
    {
        return _associatedWaypoint;
    }
    else
        return nullptr;

}

ptrConnection Association::GetConnectionAssoziation() const
{
    return _connection;
}

