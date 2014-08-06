/**
* @headerfile FrameElement.h
* @author   Ulrich Kemloh <kemlohulrich@gmail.com>
* @version 0.5
* Copyright (C) <2009-2014>
*
* @section LICENSE
* This file is part of JuPedSim.
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
*/


#ifndef FRAME_ELEMENT_H_
#define FRAME_ELEMENT_H_

class FrameElement {

public:
    FrameElement(int _id);
    virtual ~FrameElement();

    /// set/get the point ID
    void SetId(int _id);
    /// set/get the point ID
    int GetId();

    /// set/get the position of the point/agent
    void GetPos(double pos[3]);
    /// set/get the position of the point/agent
    void SetPos(double pos[3]);

    ///Set/Get the agent scaling, semi axes of the ellipses
    void SetRadius(double radius[3]);
    void GetRadius(double radius[3]);

    /// Set/Get the phisical orientation of the agent
    void SetOrientation(double angle[3]);
    void GetOrientation(double angle[3]);

    /// Set/Get the color which codes the velocity
    void SetColor(double color);
    void GetColor(double* color);

private:
    int _id;
    double _pos[3];
    double _radius[3];
    double _orientation[3];
    double _color;

};

#endif /* FRAME_ELEMENT_H_ */
