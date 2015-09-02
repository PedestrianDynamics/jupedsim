#include "waypoints.h"
#include <stdlib.h>
#include <math.h>

Waypoint::Waypoint(Point pos, ptrRoom room)
{
    _exactPos=pos;
    _room=room;
    _visited=false;
}

Waypoint::Waypoint(Point pos, double a, double b, ptrRoom room)
{
    _exactPos=pos;
    _a=a;
    _b=b;
    _room=room;
    _priority=1.0;
    _visited=false;
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

void Waypoint::SetPriority(double priority)
{
    _priority=priority;
}

const int &Waypoint::GetId() const
{
    return _id;
}

const Point &Waypoint::GetPos() const
{
    return _exactPos;
}

const double &Waypoint::GetA() const
{
    return _a;
}

const double &Waypoint::GetB() const
{
    return _b;
}


ptrRoom Waypoint::GetRoom() const
{
    return _room;
}

const std::string &Waypoint::GetCaption() const
{
    return _caption;
}

const double &Waypoint::GetPriority() const
{
    return _priority;
}

Point Waypoint::GetRandomPoint() const
{
    const double pi = std::acos(-1);
    int alpha1 = std::rand() % 360;
    int a = std::rand() % (int)_a;
    double x = _exactPos.GetX()+std::cos(alpha1*pi/180.0)*a;
    int alpha2 = std::rand() % 360;
    int b = std::rand() % (int)_b;
    double y = _exactPos.GetY()+std::sin(alpha2*pi/180.0)*b;

    //Log->Write(std::to_string(x));
    //Log->Write(std::to_string(y));
    return Point(x,y);
}

Point Waypoint::PointOnShortestRoute(const Point& point) const
{
    const double pi = std::acos(-1);
    double distance;
    int alpha_min=1;
    double t_min=0;
    double min=std::sqrt(std::pow((_exactPos.GetX()+t_min*_a*std::cos(pi/180.0))-point.GetX(),2)+std::pow((_exactPos.GetX()+t_min*_b*std::sin(pi/180.0))-point.GetY(),2));

    for (double t=0.2; t<=1; t+=0.2)
    {
        for (int alpha=11; alpha<=360; alpha+=10)
        {
            distance=std::sqrt(std::pow((_exactPos.GetX()+t*_a*std::cos(alpha*pi/180.0))-point.GetX(),2)+std::pow((_exactPos.GetY()+t*_b*std::sin(alpha*pi/180.0))-point.GetY(),2));
            if (distance<min)
            {
                min=distance;
                alpha_min=alpha;
                t_min=t;
            }
        }
    }
    //Log->Write(std::to_string(min));
    return Point(_exactPos.GetX()+_a*std::cos(alpha_min*pi/180.0),_exactPos.GetY()+_b*std::sin(alpha_min*pi/180.0));
}

bool Waypoint::WaypointReached(const Point& currentYAH)
{
    if (std::abs(_exactPos.GetX()-currentYAH.GetX())<0.75*_a && std::abs(_exactPos.GetY()-currentYAH.GetY())<0.75*_b)
    {
        Log->Write("INFO:\t Waypoint reached");
        Log->Write(std::to_string(currentYAH.GetX())+" "+std::to_string(currentYAH.GetY()));
        _visited=true;
        return true;
    }
    return false;
}

bool Waypoint::Visited() const
{
    return _visited;
}

void Waypoint::SetVisited(bool stat)
{
    _visited=stat;
}




