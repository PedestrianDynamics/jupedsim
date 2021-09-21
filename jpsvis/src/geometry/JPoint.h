/**
* @headerfile Point.h
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


#ifndef JPOINT_H_
#define JPOINT_H_

class JPoint {
private:
    double x,y,z;
    unsigned char r,g,b;
    double thickness;
    double height;
    double color;

public:
    JPoint(); //default constructor
    JPoint(double x, double y, double z, unsigned char r='a',  unsigned char g='b',unsigned char b='c');
    JPoint(double xyz[3],  const char* col="abc");
    virtual ~JPoint();

    double getX() const;
    double getY()const;
    double getZ()const;
    double distanceTo(JPoint& pt)const;
    double angleMadeWith(JPoint &pt)const;
    JPoint centreCoordinatesWith(JPoint &pt) const;

    static double distanceBetween(JPoint& pt1, JPoint& pt2);
    static double angleMadeBetween(JPoint& pt1, JPoint& pt2);
    static JPoint centreCoordinatesBetween(JPoint& pt1, JPoint& pt2);

    void setColorRGB(unsigned char r,  unsigned char g, unsigned char b);
    void getColorRGB(unsigned char *rgb);

    void setXYZ(double* xyz);
    void getXYZ(double* xyz);

    unsigned char getR();
    unsigned char getG();
    unsigned char getB();

    //methods for convenience in the case
    //this is the end point of a door/wall for instance
    void setColorHeightThicknes(double CHT[3]);
    void getColorHeightThicknes(double *CHT);

    // operators
    /// addition
    const JPoint operator+(const JPoint& p) const;
    /// substraction
    const JPoint operator-(const JPoint& p) const;

};

/// multiplication
JPoint operator*(const JPoint& p, const double f);
/// division
JPoint operator/(const JPoint& p, const double f);

#endif /* POINT_H_ */
