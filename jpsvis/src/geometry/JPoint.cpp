/**
 * @headerfile Point.cpp
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Copyright (C) <2009-2010>
 *
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * @brief method for plotting Plot a lot of straight lines,<br> and create a single actor to render
 *
 *  Created on: 17.05.2009
 *
 */

#include "JPoint.h"

#include "../Log.h"
#include "../general/Macros.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <vtkMath.h>

JPoint::JPoint(double xyz[3], const char * color /*="abc"*/)
{
    x = xyz[0];
    y = xyz[1];
    z = xyz[2];
    r = color[0];
    g = color[1];
    b = color[2];
}


JPoint::JPoint(
    double x,
    double y,
    double z,
    unsigned char r /*=100*/,
    unsigned char g /*=100*/,
    unsigned char b /*=100*/)
{
    this->setColorRGB(r, g, b);
    this->x = x;
    this->y = y;
    this->z = z;
}
JPoint::JPoint()
{
    x = 0;
    y = 0;
    z = 0; // creates a point centered at the origin
}

JPoint::~JPoint() {}

void JPoint::setColorRGB(unsigned char r1, unsigned char g1, unsigned char b1)
{
    r = r1;
    g = g1;
    b = b1;
}

void JPoint::setXYZ(double * xyz)
{
    x = xyz[0];
    y = xyz[1];
    z = xyz[2];
}
void JPoint::getXYZ(double * xyz)
{
    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;
}

/*
 * return the coordinates
 */
double JPoint::getX() const
{
    return x;
}
double JPoint::getY() const
{
    return y;
}
double JPoint::getZ() const
{
    return z;
}

/**
 * return the angle of intersection between the 2 points and the x axis
 * @return the angle in degree
 * FIXME: why +90??
 */
double JPoint::angleMadeWith(JPoint & pt) const
{
    double dx = x - pt.x;
    double dy = y - pt.y;
    // double dist=distanceTo(pt);

    if((dx == 0) && (dy == 0)) {
        Log::Error("error found in geometry");
        Log::Error("wrong angle might be returned");
        return 0;
        // return vtkMath::DegreesFromRadians(asin(dx/dist));
    }

    //	return vtkMath::DegreesFromRadians(asin(dx/dist));
    return vtkMath::DegreesFromRadians(atan2(dy, dx)) + 90;
}

double JPoint::distanceTo(JPoint & pt) const
{
    double dx = x - pt.x;
    dx *= dx;
    double dy = y - pt.y;
    dy *= dy;
    double dz = z - pt.z;
    dz *= dz;
    return sqrt(dx + dy + dz);
}

JPoint JPoint::centreCoordinatesWith(JPoint & pt) const
{
    // centre[0]=(x+pt.getX())/2;
    // centre[1]=(y+pt.getY())/2;
    // centre[2]=(z+pt.getZ())/2;
    return (*this + pt) * 0.5;
}

double JPoint::distanceBetween(JPoint & pt1, JPoint & pt2)
{
    return pt1.distanceTo(pt2);
}

double JPoint::angleMadeBetween(JPoint & pt1, JPoint & pt2)
{
    return pt1.angleMadeWith(pt2);
}

JPoint JPoint::centreCoordinatesBetween(JPoint & pt1, JPoint & pt2)
{
    return pt1.centreCoordinatesWith(pt2);
}

/*
 * return the color
 */

unsigned char JPoint::getR()
{
    return r;
}
unsigned char JPoint::getG()
{
    return g;
}
unsigned char JPoint::getB()
{
    return b;
}


void JPoint::setColorHeightThicknes(double CHT[3])
{
    thickness = CHT[2];
    height    = CHT[1];
    color     = CHT[0];
}
void JPoint::getColorHeightThicknes(double * CHT)
{
    CHT[0] = color;
    CHT[1] = height;
    CHT[2] = thickness;
}

//  sum
const JPoint JPoint::operator+(const JPoint & p) const
{
    // return Point(_x + p.GetX(), _y + p.GetY());
    return JPoint(x + p.x, y + p.y, z + p.z);
}

// sub
const JPoint JPoint::operator-(const JPoint & p) const
{
    // return Point(_x - p.GetX(), _y - p.GetY());
    return JPoint(x - p.x, y - p.y, z - p.z);
}


// multiplication with scalar
JPoint operator*(const JPoint & p, double f)
{
    return JPoint(p.getX() * f, p.getY() * f, p.getZ() * f);
}

// divition with scalar
JPoint operator/(const JPoint & p, double f)
{
    //     if (f>J_EPS*J_EPS)
    //          return JPoint(p._x / f, p._y / f);
    //     else {
    //          std::cout << "Warning: Point::/operator. dividand "<<f<< " is to small. Set it to 1
    //          instead"<<std::endl; return JPoint(p._x, p._y);
    //     }
    return JPoint(p.getX() / f, p.getY() / f, p.getZ() / f);
}
