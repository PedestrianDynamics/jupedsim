/**
* @headerfile FacilityGeometry.h
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
* @brief method for plotting the different geometry/Building elements
*
*  Created on: 07.05.2009
*
*/


#ifndef FACILITYGEOMETRY_H_
#define FACILITYGEOMETRY_H_

#include <string>

//forwarded classes
class vtkPolyData;
class vtkActor;
class Point;
class vtkAssembly;
class vtkDataSet;
class vtkLookupTable;
class LinePlotter2D;
class vtkActor2DCollection;

class FacilityGeometry{


public:

	/**
	 * Building element types that are actually supported
	 */
	enum ELEMENT_TYPE{
		DOOR,    //!< DOOR defined by two coordinates points are needed.
		STEP,    //!< STEP @todo not fully implemented
		WALL,    //!< WALL defined by two coordinates points are needed.
		SPHERE,  //!< SPHERE defined by centre[x,y,z] and radius.
		CONE,    //!< CONE defined by centre, radius and height
		CYLINDER,//!< CYLINDER defined by centre ,radius and height
		BOX      //!< BOX defined by centre, length, width and height
	};

	FacilityGeometry();
	virtual ~FacilityGeometry();

	vtkAssembly* getActor();

	vtkActor2DCollection* getCaptions();

	///draw a wall
	void addWall(double x1, double y1, double z1, double x2, double y2, double z2, double thickness=15, double height=250,double col=255);
	//void addWall(double center[3], double width, double orientation);
	void addWall(Point* p1, Point* p2, std::string caption="");

	///draw a door
	void addDoor(double x1, double y1, double z1 ,double x2, double y2, double z2, double thickness=17, double height=250,double col=30);
	//void addDoor(double center[3], double width, double orientation);
	void addDoor(Point* p1, Point* p2, std::string caption="");

	///draw a step
	///todo: implement thickness and color
	void addStep(double x1, double y1, double z1, double x2, double y2, double z2=0/*, double thickness=30, double height=10,double col=50*/);
	//void addStep(double center[3], double width, double orientation);
	void addStep(Point* p1, Point* p2);

	/// draw a floor, divided in cells,
	void addFloor(double x1, double y1, double x2, double y2, double z=0);

	/// draw other kinds of objects
	void addObjectSphere(double center[3], double radius, double couleur=1);
	void addObjectCone(double center[3], double height, double radius, double couleur=2);
	void addObjectCylinder(double center[3],double radius, double height,double orientation[3], double couleur =3);
	void addObjectBox(double center[3],double height, double width, double length, double couleur =4);
	void addObjectLabel(double center[3], double orientation[3], std::string caption, double color);

	void changeWallsColor(double* color);
	void changeExitsColor(double* color);

	void set2D(bool status);
	void set3D(bool status);

	void showDoors(bool status);
	void showStairs(bool status);
	void showWalls(bool status);
	void showGeometryLabels(int v);


private:
	// TODO Check if this function is really necessary
	//vtkActor* MapToActor(vtkDataSet *ds); //for drawing floor
	vtkLookupTable* lookupTable;
	void drawWall(Point* p1, Point* p2);
	void drawDoor(Point* p1, Point* p2);
	void addNewElement(double center[3], double orientation, double width, ELEMENT_TYPE type);
	void addNewElementText(double center[3], double orientation[3], std::string text, double color);

	// geometry parameters
	double doorThickness ;
	double wallThickness ;
	double wallHeight;
	double doorHeight;
	double stepHeight;
	double wallColor;
	double stepColor;
	double doorColor;

	// geometry assembly
	vtkAssembly* assembly;

	//2-d parts
	LinePlotter2D* linesPlotter2D;
	vtkAssembly* assembly2D;

//	// 3-d parts
//	vtkAssembly* assemblyObjects;
//	vtkAssembly* assemblyWalls3D;
//	vtkAssembly* assemblyDoors3D;
//	vtkAssembly* assembly3D;

	// other parts
	vtkAssembly* assemblyCaptions;

	vtkActor2DCollection* captions;



};

#endif /* FACILITYGEOMETRY_H_ */
