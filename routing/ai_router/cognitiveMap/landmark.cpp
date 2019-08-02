#include "landmark.h"

#include "general/Macros.h"
#include "associations.h"

#include <math.h>

AILandmark::AILandmark(const Point &pos)
{
    _realPos=pos;
    _visited=false;
    _posInMap=Point(0.0,0.0);
}

AILandmark::AILandmark(const Point& pos, double a, double b, int id)
{
    _realPos=pos;
    _a=a;
    _b=b;
    _visited=false;
    _id=id;
    _posInMap=Point(0.0,0.0);
}

AILandmark::~AILandmark()
{

}

void AILandmark::SetId(int id)
{
    _id=id;
}

void AILandmark::SetA(double a)
{
    _a=a;
}

void AILandmark::SetB(double b)
{
    _b=b;
}

void AILandmark::SetRealPos(const Point &point)
{
    _realPos=point;
}

void AILandmark::SetPosInMap(const Point &point)
{
    _posInMap=point;
}



void AILandmark::SetCaption(const std::string &string)
{
    _caption=string;
}

void AILandmark::SetType(const std::string& type)
{
    _type=type;
}

int AILandmark::GetId() const
{
    return _id;
}

const Point &AILandmark::GetRealPos() const
{
    return _realPos;
}

const Point &AILandmark::GetPosInMap() const
{
    return _posInMap;
}

double AILandmark::GetA() const
{
    return _a;
}

double AILandmark::GetB() const
{
    return _b;
}

const std::string& AILandmark::GetType() const
{
    return _type;
}


const std::string &AILandmark::GetCaption() const
{
    return _caption;
}


Point AILandmark::GetRandomPoint() const
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

Point AILandmark::PointOnShortestRoute(const Point& point) const
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

bool AILandmark::Visited() const
{
    return _visited;
}

void AILandmark::SetVisited(bool stat)
{
    _visited=stat;
}

bool AILandmark::Contains(const Point &point) const
{
    double x = _a-(std::fabs(this->GetPosInMap()._x-point._x));
    double y = _b-(std::fabs(this->GetPosInMap()._y-point._y));

    if ((std::fabs(this->GetPosInMap()._x-point._x)<=_a && std::pow((1-std::pow(x,2)/std::pow(_a,2)),0.5)*_b<=_b)
            || (std::fabs(this->GetPosInMap()._y-point._y)<=_b && std::pow((1-std::pow(y,2)/std::pow(_b,2)),0.5)*_a<=_a ))
        return true;

    return false;
}

const AIAssociations &AILandmark::GetAssociations() const
{
    return _assoContainer;
}

void AILandmark::AddAssociation(const AIAssociation &asso)
{
    if (std::find(_assoContainer.begin(), _assoContainer.end(), asso)!=_assoContainer.end())
        return;
    else
        _assoContainer.push_back(asso);
}

const AIRegion *AILandmark::IsInRegion() const
{
    return _region;
}

void AILandmark::SetRegion(const AIRegion *region)
{
    _region=region;
}



