#include "waypoints.h"

Waypoint::Waypoint(Point pos, ptrRoom room)
{
    _exactPos=pos;
    _room=room;
}

Waypoint::Waypoint(Point pos, double a, double b, ptrRoom room)
{
    _exactPos=pos;
    _a=a;
    _b=b;
    _room=room;
}

Waypoint::~Waypoint()
{

}

void Waypoint::SetId(int id)
{
    _id=id;
}

void Waypoint::SetPos(const Point &point)
{
    _exactPos=point;
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
    return _exactPos;
}

const double &Waypoint::GetA()
{
    return _a;
}

const double &Waypoint::GetB()
{
    return _b;
}


ptrRoom Waypoint::GetRoom()
{
    return _room;
}

const std::string &Waypoint::GetCaption()
{
    return _caption;
}

void Waypoint::AddAssociation(Association asso)
{
    if (std::find(Associations.begin(), Associations.end(), asso)!=Associations.end())
        return;
    else
        Associations.push_back(asso);
}

