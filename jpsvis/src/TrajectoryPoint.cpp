/**
* @file TrajectoryPoint.cpp
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2010>
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
* \brief
*
*
*  Created on: 08.07.2009
*
*/



#include <limits>
#include "TrajectoryPoint.h"



TrajectoryPoint::TrajectoryPoint(int index)
{
    this->index=index;
    this->x = std::numeric_limits<double>::quiet_NaN();
    this->y = std::numeric_limits<double>::quiet_NaN();
    this->z = std::numeric_limits<double>::quiet_NaN();
    this->xVel = std::numeric_limits<double>::quiet_NaN();
    this->yVel = std::numeric_limits<double>::quiet_NaN();
    this->zVel = std::numeric_limits<double>::quiet_NaN();

    this->agentOrientation = std::numeric_limits<double>::quiet_NaN();
    this->ellipseRadiusA = std::numeric_limits<double>::quiet_NaN();
    this->ellipseRadiusB = std::numeric_limits<double>::quiet_NaN();
    this->ellipseColor = std::numeric_limits<double>::quiet_NaN();
    this->agentHeight = std::numeric_limits<double>::quiet_NaN();

}

TrajectoryPoint::TrajectoryPoint(int index,double x, double y, double z)
{
    this->index = index;
    this->x = x;
    this->y = y;
    this->z = z;
    this->xVel = std::numeric_limits<double>::quiet_NaN();
    this->yVel = std::numeric_limits<double>::quiet_NaN();
    this->zVel = std::numeric_limits<double>::quiet_NaN();

    this->agentOrientation = std::numeric_limits<double>::quiet_NaN();
    this->ellipseRadiusA = std::numeric_limits<double>::quiet_NaN();
    this->ellipseRadiusB = std::numeric_limits<double>::quiet_NaN();
    this->ellipseColor = std::numeric_limits<double>::quiet_NaN();
    this->agentHeight = std::numeric_limits<double>::quiet_NaN();
}
TrajectoryPoint::~TrajectoryPoint()
{

}


//setter
void TrajectoryPoint::setIndex(int index)
{
    this->index = index;
}
void TrajectoryPoint::setPos(double pos[3])
{
    x = pos[0];
    y = pos[1];
    z = pos[2];
}
void TrajectoryPoint::setVel(double vel[3])
{
    xVel = vel[0];
    yVel = vel[1];
    zVel = vel[2];
}

void TrajectoryPoint::setEllipse(double ellipse[7])
{
    ellipseCenter[0] = ellipse[0];
    ellipseCenter[1] = ellipse[1];
    ellipseCenter[2] = ellipse[2];
    ellipseRadiusA = ellipse[3];
    ellipseRadiusB = ellipse[4];
    agentOrientation = ellipse[5];
    ellipseColor = ellipse[6];
}

int TrajectoryPoint::getIndex()
{
    return index;
}
double TrajectoryPoint::getX()
{
    return x;
}
double TrajectoryPoint::getY()
{
    return y;
}
double TrajectoryPoint::getZ()
{
    return z;
}
void TrajectoryPoint::getPos(double pos[3])
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
}
void TrajectoryPoint::getVel(double vel[3])
{
    vel[0] = xVel;
    vel[1] = yVel;
    vel[2] = zVel;
}
void TrajectoryPoint::getEllipse(double ellipse[7])
{
    ellipse[0] = ellipseCenter[0];
    ellipse[1] = ellipseCenter[1];
    ellipse[2] = ellipseCenter[2];

    ellipse[3] = ellipseRadiusA;
    ellipse[4] = ellipseRadiusB;
    ellipse[5] = agentOrientation;
    ellipse[6] = ellipseColor;
}
void TrajectoryPoint::setAgentInfo(double para[2])
{
    agentColor=para[0];
    agentOrientation=para[1];
}

void TrajectoryPoint::getAgentInfo(double para[2])
{
    para[0]=agentColor;
    para[1]=agentOrientation;
}
