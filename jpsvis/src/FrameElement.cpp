/**
* @file FrameElement.cpp
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.1
* Copyright (C) <2009-2014>
*
* @section LICENSE
* This file is part of JuPedsim.
*
* JuPedSim is free software: you can redistribute it and/or modify
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
*/



#include <limits>
#include "FrameElement.h"


FrameElement::FrameElement(int id)
{
    _id=id;
    _pos[0] = std::numeric_limits<double>::quiet_NaN();
    _pos[1] = std::numeric_limits<double>::quiet_NaN();
    _pos[2] = std::numeric_limits<double>::quiet_NaN();

    _orientation[0] = std::numeric_limits<double>::quiet_NaN();
    _orientation[1] = std::numeric_limits<double>::quiet_NaN();
    _orientation[2] = std::numeric_limits<double>::quiet_NaN();

    _radius[0] = std::numeric_limits<double>::quiet_NaN();
    _radius[1] = std::numeric_limits<double>::quiet_NaN();
    _radius[2] = std::numeric_limits<double>::quiet_NaN();
}

FrameElement::~FrameElement() {

}

void FrameElement::SetId(int index) {
    _id = index;
}

void FrameElement::SetPos(double pos[3])
{
    _pos[0] = pos[0];
    _pos[1] = pos[1];
    _pos[2] = pos[2];
}

void FrameElement::SetRadius(double radius[])
{
    _radius[0] = radius[0];
    _radius[1] = radius[1];
    _radius[2] = radius[2];
}

void FrameElement::GetRadius(double radius[])
{
    radius[0] = _radius[0];
    radius[1] = _radius[1];
    radius[2] = _radius[2];
}

void FrameElement::GetPos(double pos[3])
{
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    pos[2] = _pos[2];
}

void FrameElement::SetOrientation(double angle[])
{
  _orientation[0]=angle[0];
  _orientation[1]=angle[1];
  _orientation[2]=angle[2];
}

void FrameElement::GetOrientation(double angle[])
{
    angle[0]=_orientation[0];
    angle[1]=_orientation[1];
    angle[2]=_orientation[2];
}

void FrameElement::SetColor(double color)
{
    _color=color;
}

void FrameElement::GetColor(double *color)
{
    *color=_color;
}

int FrameElement::GetId()
{
    return _id;
}

