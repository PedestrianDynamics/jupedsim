#include "landmark.h"

#include "associations.h"

Landmark::Landmark(Point pos, ptrRoom room)
{
    _realPos=pos;
    _room=room;
    _visited=false;
}

Landmark::Landmark(Point pos, double a, double b, int id, ptrRoom room)
{
    _realPos=pos;
    _a=a;
    _b=b;
    _room=room;
    _priority=1.0;
    _visited=false;
    _id=id;
}

Landmark::~Landmark()
{

}

void Landmark::SetId(int id)
{
    _id=id;
}

void Landmark::SetA(double a)
{
    _a=a;
}

void Landmark::SetB(double b)
{
    _b=b;
}

void Landmark::SetRealPos(const Point &point)
{
    _realPos=point;
}

void Landmark::SetPosInMap(const Point &point)
{
    _posInMap=point;
}


void Landmark::SetRoom(ptrRoom room)
{
    _room=room;
}

void Landmark::SetCaption(const std::string &string)
{
    _caption=string;
}

void Landmark::SetPriority(double priority)
{
    _priority=priority;
}

void Landmark::SetType(const std::string& type)
{
    _type=type;
}

const int &Landmark::GetId() const
{
    return _id;
}

const Point &Landmark::GetRealPos() const
{
    return _realPos;
}

const Point &Landmark::GetPosInMap() const
{
    return _posInMap;
}

const double &Landmark::GetA() const
{
    return _a;
}

const double &Landmark::GetB() const
{
    return _b;
}

const std::string& Landmark::GetType() const
{
    return _type;
}


ptrRoom Landmark::GetRoom() const
{
    return _room;
}

const std::string &Landmark::GetCaption() const
{
    return _caption;
}

const double &Landmark::GetPriority() const
{
    return _priority;
}

Point Landmark::GetRandomPoint() const
{
    const double pi = M_PI;//std::acos(-1);
    int alpha1 = std::rand() % 360;
    double factor_a = (std::rand() % 100)/100.0;
    double x = _posInMap._x+std::cos(alpha1*pi/180.0)*factor_a*_a;
    int alpha2 = std::rand() % 360;
    double factor_b = (std::rand() % 100)/100.0;
    double y = _posInMap._y+std::sin(alpha2*pi/180.0)*factor_b*_b;

    return Point(x,y);
}

Point Landmark::PointOnShortestRoute(const Point& point) const
{
    const double pi = std::acos(-1);
    double distance;
    int alpha_min=1;
    double t_min=0;
    double min=std::sqrt(std::pow((_posInMap._x+t_min*_a*std::cos(pi/180.0))-point._x,2)+std::pow((_posInMap._x+t_min*_b*std::sin(pi/180.0))-point._y,2));

    for (double t=0.2; t<=1; t+=0.2)
    {
        for (int alpha=11; alpha<=360; alpha+=10)
        {
            distance=std::sqrt(std::pow((_posInMap._x+t*_a*std::cos(alpha*pi/180.0))-point._x,2)+std::pow((_posInMap._y+t*_b*std::sin(alpha*pi/180.0))-point._y,2));
            if (distance<min)
            {
                min=distance;
                alpha_min=alpha;
                t_min=t;
            }
        }
    }
    //Log->Write(std::to_string(min));
    return Point(_posInMap._x+_a*std::cos(alpha_min*pi/180.0),_posInMap._y+_b*std::sin(alpha_min*pi/180.0));
}

//bool Landmark::LandmarkReached(const Point& currentYAH)
//{
//    if (std::abs(_realPos._x-currentYAH._x)<0.75*_a && std::abs(_realPos._y-currentYAH._y)<0.75*_b)
//    {
//        Log->Write("INFO:\t Landmark reached");
//        Log->Write(std::to_string(currentYAH._x)+" "+std::to_string(currentYAH._y));
//        _visited=true;
//        return true;
//    }
//    return false;
//}

bool Landmark::Visited() const
{
    return _visited;
}

void Landmark::SetVisited(bool stat)
{
    _visited=stat;
}

bool Landmark::Contains(const Point &point)
{
    double x = _a-(std::fabs(this->GetPosInMap()._x-point._x));
    double y = _b-(std::fabs(this->GetPosInMap()._y-point._y));

    if ((std::fabs(this->GetPosInMap()._x-point._x)<=_a && std::pow((1-std::pow(x,2)/std::pow(_a,2)),0.5)*_b<=_b)
            || (std::fabs(this->GetPosInMap()._y-point._y)<=_b && std::pow((1-std::pow(y,2)/std::pow(_b,2)),0.5)*_a<=_a ))
        return true;

    return false;
}

Associations Landmark::GetAssociations() const
{
    return _assoContainer;
}

void Landmark::AddAssociation(ptrAssociation asso)
{
    if (std::find(_assoContainer.begin(), _assoContainer.end(), asso)!=_assoContainer.end())
        return;
    else
        _assoContainer.push_back(asso);
}

const ptrRegion Landmark::IsInRegion() const
{
    return _region;
}

void Landmark::SetRegion(ptrRegion region)
{
    _region=region;
}



