/**
* @headerfile LinePlotter.h
* @author  Ulrich Kemloh <kemlohulrich@gmail.com>
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


#ifndef LINE_PLOTTER_H
#define LINE_PLOTTER_H

class vtkLookupTable ;
class vtkPoints ;
class vtkCellArray ;
class vtkFloatArray ;
class vtkActor ;
class vtkPolyData;
class vtkDataArray;
class JPoint;


class LinePlotter {
public:

    LinePlotter();
    ~LinePlotter();

    //void SetScalarRange(double minval=0.0, double maxval=1.0);
    //void SetLookupTable(vtkLookupTable* table = 0);

    void PlotLine(JPoint* pt1, JPoint* pt2);
    void PlotLine(double m[3], double n[3], unsigned char scalar[3]);
    void PlotLine(double x, double y, double z,
                  double x2, double y2, double z2, unsigned char scalar[3]);
    void addVertex(double vertex[3],double scalar[3]);
    void addVertex(JPoint* pt1);
    void SetNumberOfPoints(int nPoints);
    void clear();
    static void setLineWidth(int width=1);
    vtkActor* getActor();

private:
    //double m_scalarMin, m_scalarMax ;
    //vtkLookupTable* m_lookupTable ;
    int m_curPointID ;
    static int m_allLineWidth ;
    //unsigned char colour[3];

    vtkPoints* m_points;
    vtkActor* m_lineActors;
    vtkCellArray* m_lines;
    //vtkFloatArray* m_lineScalars ;
    vtkDataArray* m_scalars ;

};

#endif // LINE_PLOTTER_H
