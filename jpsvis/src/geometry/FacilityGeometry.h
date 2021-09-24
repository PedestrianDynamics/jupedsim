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

// forwarded classes
class vtkPolyData;
class vtkActor;
class JPoint;
class vtkAssembly;
class vtkDataSet;
class vtkLookupTable;
class LinePlotter2D;
class vtkActor2DCollection;

class FacilityGeometry
{
public:
    /**
     * Building element types that are actually supported
     */
    enum ELEMENT_TYPE {
        DOOR,     //!< DOOR defined by two coordinates points are needed.
        STEP,     //!< STEP @todo not fully implemented
        WALL,     //!< WALL defined by two coordinates points are needed.
        SPHERE,   //!< SPHERE defined by centre[x,y,z] and radius.
        CONE,     //!< CONE defined by centre, radius and height
        CYLINDER, //!< CYLINDER defined by centre ,radius and height
        BOX       //!< BOX defined by centre, length, width and height
    };

    FacilityGeometry(
        const std::string & description,
        const std::string & roomCaption,
        const std::string & subroomCaption);
    virtual ~FacilityGeometry();

    vtkAssembly * getActor2D();

    vtkAssembly * getActor3D();

    vtkAssembly * getCaptionsActor();

    vtkActor2DCollection * getCaptions();

    void CreateActors();

    /// draw a wall
    void addWall(
        double x1,
        double y1,
        double z1,
        double x2,
        double y2,
        double z2,
        double thickness = 15,
        double height    = 250,
        double col       = 255);
    // void addWall(double center[3], double width, double orientation);
    void addWall(JPoint * p1, JPoint * p2, std::string caption = "");

    /// draw a stair
    void addStair(
        double x1,
        double y1,
        double z1,
        double x2,
        double y2,
        double z2,
        double thickness = 15,
        double height    = 250,
        double col       = 255);
    // void addWall(double center[3], double width, double orientation);
    void addStair(JPoint * p1, JPoint * p2, std::string caption = "");


    /// draw a door
    void addDoor(
        double x1,
        double y1,
        double z1,
        double x2,
        double y2,
        double z2,
        double thickness = 17,
        double height    = 250,
        double col       = 30);
    // void addDoor(double center[3], double width, double orientation);
    void addDoor(JPoint * p1, JPoint * p2, std::string caption = "");

    /// draw a step
    /// todo: implement thickness and color
    void addStep(
        double x1,
        double y1,
        double z1,
        double x2,
        double y2,
        double z2 = 0 /*, double thickness=30, double height=10,double col=50*/);
    // void addStep(double center[3], double width, double orientation);
    void addStep(JPoint * p1, JPoint * p2);

    /// draw a navigation line
    void addNavLine(
        double x1,
        double y1,
        double z1,
        double x2,
        double y2,
        double z2,
        double thickness = 2,
        double height    = 250,
        double color     = 95);
    void addNavLine(JPoint * p1, JPoint * p2, std::string caption = "");

    /// draw a floor, divided in cells,
    void addFloor(double x1, double y1, double x2, double y2, double z = 0);
    void addRectangle(
        double x1,
        double y1,
        double x2,
        double y2,
        double z         = 0,
        double c1        = 120,
        double c2        = 150,
        std::string text = "");
    void addFloor(vtkPolyData * polygonPolyData);

    /// draw obstacles
    void addObstacles(vtkPolyData * polygonPolyData);

    /// add the gradient field
    void addGradientField(vtkActor * gradientField);

    const std::string & GetDescription() const;
    const std::string & GetRoomCaption() const;
    const std::string & GetSubRoomCaption() const;
    void SetRoomCaption(std::string s);
    void SetSubRoomCaption(std::string s);
    /// draw other kinds of objects
    void addObjectSphere(double center[3], double radius, double couleur = 1);
    void addObjectCone(double center[3], double height, double radius, double couleur = 2);
    void addObjectCylinder(
        double center[3],
        double radius,
        double height,
        double orientation[3],
        double couleur = 3);
    void
    addObjectBox(double center[3], double height, double width, double length, double couleur = 4);
    void addObjectLabel(double center[3], double orientation[3], std::string caption, double color);

    void changeWallsColor(double * color);
    void changeExitsColor(double * color);
    void changeNavLinesColor(double * color);
    void changeFloorColor(double * color);
    void changeObstaclesColor(double * color);

    void set2D(bool status);
    void set3D(bool status);

    void showDoors(bool status);
    void showStairs(bool status);
    void showWalls(bool status);
    void showNavLines(bool status);
    void showFloor(bool status);
    void showObstacles(bool status);
    void showGeometryLabels(int status);
    void showGradientField(bool status);

    void setVisibility(bool status);
    bool getVisibility() const;


private:
    // TODO Check if this function is really necessary
    // vtkActor* MapToActor(vtkDataSet *ds); //for drawing floor
    vtkLookupTable * lookupTable;
    void drawWall(JPoint * p1, JPoint * p2);
    void drawDoor(JPoint * p1, JPoint * p2);
    void addNewElement(double center[3], double orientation, double width, ELEMENT_TYPE type);
    void addNewElementText(double center[3], double orientation[3], std::string text, double color);

    // geometry parameters
    double doorThickness;
    double wallThickness;
    double wallHeight;
    double doorHeight;
    double stepHeight;
    double wallColor;
    double stepColor;
    double doorColor;
    double navlineColor;

    // geometry assembly
    vtkAssembly * assembly;

    // 2-d parts
    LinePlotter2D * linesPlotter2D;
    vtkAssembly * assembly2D;

    // 3-d parts
    // vtkAssembly* assemblyObjects;
    vtkAssembly * assemblyWalls3D;
    vtkAssembly * assemblyDoors3D;
    vtkAssembly * assembly3D;

    vtkActor * floorActor;
    vtkActor * obstaclesActor;
    vtkActor * gradientFieldActor;

    // other parts
    vtkAssembly * assemblyCaptions;
    vtkActor2DCollection * captions;

    std::string _description;
    std::string _roomCaption;
    std::string _subroomCaption;
    bool _visibility = true;
};

#endif /* FACILITYGEOMETRY_H_ */
