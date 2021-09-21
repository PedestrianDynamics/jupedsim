/**
* @headerfile PointPlotter2D.h
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
* @brief method for plotting Plot a lot of points(in 2D),<br> and create a single actor to render
*
*  Created on: 07.05.2009
*
*/
#ifndef MY_VTK_POINT_PLOTTER_H
#define MY_VTK_POINT_PLOTTER_H


class vtkPoints ;
class vtkUnsignedCharArray ;
class vtkPolyData ;
class vtkActor ;
class vtkDataArray ;

class PointPlotter2D {
public:


    PointPlotter2D();
    ~PointPlotter2D();


    void PlotPoint(double x, double y, double z,
                   unsigned char r, unsigned char g, unsigned char b);

    void SetPointRadius(double radius = 1.0)
    {
        pt_radius = radius ;
    }
    void SetPointResolution(int res = 15)
    {
        pt_res = res ;
    }



    vtkPolyData* CreatePolyData();  // call it after all points are plotted
    vtkActor* CreateActor(); // call it after all points are plotted
private:


    vtkPoints* pts ;
    vtkDataArray* scalars ;
    int scalar_mode ;

    double pt_radius ;
    int pt_res ;
};

#endif // MY_VTK_POINT_PLOTTER_H
