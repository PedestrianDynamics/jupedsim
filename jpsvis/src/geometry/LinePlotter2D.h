/**
 * @file LinePlotter2D.cpp
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
 *
 *
 *  Created on: 22.05.2009
 *
 */



#ifndef LINE_PLOTTER2D_H
#define LINE_PLOTTER2D_H

class vtkLookupTable;
class vtkPoints;
class vtkCellArray;
class vtkFloatArray;
class vtkActor;
class vtkAssembly;
class vtkPolyData;
class vtkPolyDataMapper;


class LinePlotter2D {
public:

    LinePlotter2D();
    ~LinePlotter2D();

    void SetAllLineWidth(int width = 1);

    void PlotDoor(double m[3], double n[3], double scalar);
    void changeDoorsColor(double* col);

    void PlotWall(double m[3], double n[3], double scalar);
    void changeWallsColor(double *col);


    void PlotNavLine(double m[3], double n[3], double scalar);
    void changeNavLinesColor(double *col);

    void showDoors(bool status);
    void showWalls(bool status);
    void showNavLines(bool status);

    vtkAssembly* createAssembly();
    static bool doorColorsToDefault;

private:

    vtkLookupTable* m_lookupTable ;
    vtkAssembly* assembly;


    /// doors parameters
    int door_curPointID ;
    double door_width;
    vtkPoints* door_points;
    vtkCellArray* door_lines;
    vtkFloatArray* door_lineScalars ;
    vtkPolyDataMapper* door_mapper;
    vtkActor* door_actor;

    /// walls parameters
    int wall_curPointID ;
    double wall_width;
    vtkPoints* wall_points;
    vtkCellArray* wall_lines;
    vtkFloatArray* wall_lineScalars ;
    vtkPolyDataMapper* wall_mapper;
    vtkActor* wall_actor;

    /// navigation lines parameters
    int navline_curPointID ;
    double navline_width;
    vtkPoints* navline_points;
    vtkCellArray* navline_lines;
    vtkFloatArray* navline_lineScalars ;
    vtkPolyDataMapper* navline_mapper;
    vtkActor* navline_actor;

};

#endif // LINE_PLOTTER2D_H
