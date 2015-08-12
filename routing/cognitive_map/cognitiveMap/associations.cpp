#include "associations.h"

Association::Association()
{
    _waypoint=nullptr;
    _associatedWaypoint=nullptr;

}

Association::Association(ptrWaypoint waypoint, ptrWaypoint associated_waypoint)
{
    _waypoint=waypoint;
    _associatedWaypoint=associated_waypoint;

}

Association::~Association()
{

}

ptrWaypoint Association::GetAssociation(ptrWaypoint waypoint)
{
    if (_waypoint==waypoint)
    {
        return _associatedWaypoint;
    }
    else
        return nullptr;

}

