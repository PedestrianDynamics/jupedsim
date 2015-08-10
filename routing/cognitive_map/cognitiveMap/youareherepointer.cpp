#include "youareherepointer.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

YouAreHerePointer::YouAreHerePointer()
{
    _angle=0.0;
    _pos=Point(0.0,0.0);
    _oldpos=Point(0.0,0.0);
}

YouAreHerePointer::~YouAreHerePointer()
{

}

const Point &YouAreHerePointer::GetPos() const
{
    return _pos;
}

const double &YouAreHerePointer::GetDirection() const
{
    return _angle;
}

void YouAreHerePointer::SetPos(const Point &point)
{
    _pos=point;
}

void YouAreHerePointer::SetDirection(const double &angle)
{
//    double mod = std::fmod(angle,M_PI/4.0);
//    if (mod>=M_PI/8.0)
//        _angle=angle+M_PI/4.0-mod;
//    else
//        _angle=angle-mod;
    //std::cout << _angle << std::endl;
    _angle=angle;
    //std::cout << std::to_string(angle*180/3.14) << std::endl;
}

void YouAreHerePointer::UpdateYAH(const Point &move)
{
    _oldpos=_pos;
    double x = std::cos(_angle)*move.Norm();
    //std::cout << std::to_string(std::cos(_angle)*move.Norm()) << std::endl;
    //std::cout << std::to_string(move.GetX()) << std::endl;
    double y =std::sqrt(1-std::pow(std::cos(_angle),2))*move.Norm();
    if (_angle<0)
        y=-y;

        //std::cout << std::to_string(std::sqrt(1-std::pow(std::cos(_angle),2))*move.Norm()) << std::endl;

    _pos=_pos+Point(x,y);//Point(std::cos(_angle)*speed*timeInterval,std::sqrt(1-std::pow(std::cos(_angle),2))*speed*timeInterval);

}

