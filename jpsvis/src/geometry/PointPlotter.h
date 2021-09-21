/**
* @headerfile PointPlotter.h
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
* @brief method for plotting Plot a lot of points,<br> and create a single actor to render
*
*  Created on: 07.05.2009
*
*/

#ifndef POINT_PLOTTER_H
#define POINT_PLOTTER_H

#define MAX_POINTS 30;

#include "vtkSmartPointer.h"

class vtkPoints ;
class vtkUnsignedCharArray ;
class vtkPolyData ;
class vtkActor ;
class vtkDataArray ;
class vtkFloatArray ;
class JPoint;

//for borrowing the lookup table
extern vtkActor* extern_glyphs_pedestrians_actor_2D;

class PointPlotter {
public:


    PointPlotter();
    ~PointPlotter();

    void PlotPoint(double x, double y, double z,
                   unsigned char r='a', unsigned char g='b', unsigned char b='c');

    void PlotPoint(JPoint* pt);
    void PlotPoint(double Position[3], double colour);
    void SetPointRadius(double radius = 1.0)
    {
        pt_radius = radius ;
    }
    void SetPointResolution(int res = 15)
    {
        pt_res = res ;
    }


    ///return the actors
    vtkActor* getActor();

    void SetVisibility(bool status);

private:

    vtkActor* pointActor;
    vtkPoints* pts ;
    vtkDataArray* scalars ;
    vtkFloatArray* colors;
    vtkIdType nextPointID;
    int scalar_mode ;
    double pt_radius ;
    int pt_res ;
};

#endif // POINT_PLOTTER_H
