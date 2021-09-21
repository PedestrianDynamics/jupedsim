/**
* @headerfile TrajectoryPoint.h
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


#ifndef TRAJECTORYPOINT_H_
#define TRAJECTORYPOINT_H_

class TrajectoryPoint {
public:
    TrajectoryPoint(int index);
    TrajectoryPoint(int index, double x, double y, double z);
    virtual ~TrajectoryPoint();

    /// set/get the point ID
    void setIndex(int index);
    /// set/get the point ID
    int getIndex();

    /// set/get the position of the point/agent
    void getPos(double pos[3]);
    /// set/get the position of the point/agent
    void setPos(double pos[3]);

    /// set/get the velocity of the point/agent
    void getVel(double vel[3]);
    /// set/get the velocity of the point/agent
    void setVel(double vel[3]);

    /**
    * set/get the coordinate of the modelled ellipse.
    * @param ellipse, the center coordinates and the 2 semi-axes
    */
    void getEllipse(double ellipse[7]);

    /**
    * set/get the coordinate of the modelled ellipse.
    * @param ellipse, the center coordinates , the 2 semi-axes, the color and the orientation
    */
    void setEllipse(double ellipse[7]);

    /**
     * set/get other agents infos: the color and the orientation.
     * IF ommitted the orientation will be calculated based on the last coordinates
     *
     * @param param, Color[0 255] and orientation[0..360]
     */
    void setAgentInfo(double param[2]);
    void getAgentInfo(double para[2]);

    double getX();
    double getY();
    double getZ();


private:
    int index;
    double x;
    double y;
    double z;

    double xVel;
    double yVel;
    double zVel;

    double agentColor;
    double agentHeight;
    double agentOrientation;

    double ellipseCenter[3];
    double ellipseRadiusA;
    double ellipseRadiusB;
    double ellipseColor;
};

#endif /* TRAJECTORYPOINT_H_ */
