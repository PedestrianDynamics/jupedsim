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


#ifndef POINT_H_
#define POINT_H_

class Point {
private:
	double x,y,z;
	unsigned char r,g,b;
	double thickness;
	double height;
	double color;

public:
	Point(); //default constructor
	Point(double x, double y, double z, unsigned char r='a',  unsigned char g='b',unsigned char b='c');
	Point(double xyz[3],  char col[]="abc");
	virtual ~Point();

	double getX();
	double getY();
	double getZ();
	double distanceTo(Point& pt);
	double angleMadeWith(Point &pt);
	double* centreCoordinatesWith(Point &pt);

	static double distanceBetween(Point& pt1, Point& pt2);
	static double angleMadeBetween(Point& pt1, Point& pt2);
	static double *centreCoordinatesBetween(Point& pt1, Point& pt2);

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
};

#endif /* POINT_H_ */
