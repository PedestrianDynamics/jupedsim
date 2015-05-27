#include "waypoints.h"

Waypoint::Waypoint(Point pos, JEllipse ellipse, ptrRoom room)
{

}

Waypoint::~Waypoint()
{

}

void Waypoint::SetId(const int id)
{
    _id=id;
}

void Waypoint::SetPos(const Point &point)
{
    _point=point;
}

void Waypoint::SetEllipse(const JEllipse &ellipse)
{
    _catchmentArea=ellipse;
}

void Waypoint::SetRoom(ptrRoom room)
{
    _room=room;
}

void Waypoint::SetCaption(const std::string &string)
{
    _caption=string;
}

const int &Waypoint::GetId()
{
    return _id;
}

const Point &Waypoint::GetPos()
{
    return _pos;
}

const JEllipse &Waypoint::GetEllipse()
{
    return _catchmentArea;
}

ptrRoom Waypoint::GetRoom()
{
    return _room;
}

const std::string &Waypoint::GetCaption()
{
    return _caption;
}

