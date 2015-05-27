#include "associations.h"

Association::Association()
{
    _waypoint1=nullptr;
    _waypoint2=nullptr;

}

Association::Association(ptrWaypoint waypoint, ptrWaypoint associated_waypoint)
{
    _waypoint=waypoint1;
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

