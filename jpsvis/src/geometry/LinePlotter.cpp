/**
 * @file LinePlotter.h
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

#include "LinePlotter.h"

#include "../SystemSettings.h"
#include "JPoint.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

int LinePlotter::m_allLineWidth = 2;


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


LinePlotter::LinePlotter()
{
    m_curPointID = 0;

    m_scalars = vtkUnsignedCharArray::New();
    m_scalars->SetNumberOfComponents(3);
    m_points = vtkPoints::New();
    m_lines  = vtkCellArray::New();

    // m_lineScalars = vtkFloatArray::New();
    // create a color lookup table
    // m_lookupTable = vtkLookupTable::New();

    // create the poly data
    //	vtkPolyData* polyData = vtkPolyData::New();
    VTK_CREATE(vtkPolyData, polyData);
    polyData->SetPoints(m_points);
    polyData->SetLines(m_lines);
    // polyData->SetVerts(m_lines);
    polyData->GetPointData()->SetScalars(m_scalars);

    // create mapper
    VTK_CREATE(vtkPolyDataMapper, mapper);
    // vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    mapper->SetInputData(polyData);

    // polyData->Delete();

    //	mapper->SetLookupTable(m_lookupTable);
    //	mapper->SetColorModeToMapScalars();
    //	mapper->SetScalarRange(m_scalarMin, m_scalarMax);
    //	mapper->SetScalarModeToUsePointData();

    // create actor
    m_lineActors = vtkActor::New();
    m_lineActors->SetMapper(mapper);
    // mapper->Delete();
    m_lineActors->GetProperty()->SetLineWidth(m_allLineWidth);
}

LinePlotter::~LinePlotter()
{
    m_points->Delete();
    m_lineActors->Delete();
    m_lines->Delete();
    m_scalars->Delete();
}

void LinePlotter::setLineWidth(int width)
{
    m_allLineWidth = width;
}

void LinePlotter::SetNumberOfPoints(int nPoints)
{
    // m_lines->InsertNextCell(nPoints);
    m_lines->InsertNextCell(nPoints);
}


void LinePlotter::PlotLine(JPoint * pt1, JPoint * pt2)
{
    double m[3], n[3];
    unsigned char col[3];

    m[0] = pt1->getX();
    m[1] = pt1->getY();
    m[2] = pt1->getZ();

    n[0] = pt2->getX();
    n[1] = pt2->getY();
    n[2] = pt2->getZ();

    col[0] = pt1->getR();
    col[1] = pt1->getG();
    col[2] = pt1->getB();
    PlotLine(m, n, col);
}


void LinePlotter::addVertex(JPoint * pt1)
{
    double m[3];
    unsigned char col[3];

    m[0] = pt1->getX();
    m[1] = pt1->getY();
    m[2] = pt1->getZ() + 10;

    col[0] = pt1->getR();
    col[1] = pt1->getG();
    col[2] = pt1->getB();

    //	//m_points->InsertNextPoint(m);
    //	m_points->InsertPoint(m_curPointID,m);
    //	m_scalars->InsertTuple3(m_curPointID,col[0], col[1], col[2]);
    //	m_lines->InsertCellPoint(m_curPointID++);
    //	m_scalars->Modified();
    //	m_points->Modified();
    //	m_lines->Modified();

    // m_points->InsertNextPoint(m);
    m_points->InsertNextPoint(m);
    m_scalars->InsertTuple3(m_curPointID, col[0], col[1], col[2]);
    m_lines->InsertCellPoint(m_curPointID++);
    m_scalars->Modified();
    m_points->Modified();
    m_lines->Modified();
}

void LinePlotter::addVertex(double vertex[3], double col[3])
{
    JPoint * pts = new JPoint();
    pts->setColorRGB(col[0], col[1], col[2]);
    pts->setXYZ(vertex);
    addVertex(pts);

    delete pts;
}

// caution:
// this work only for a line with 2 points. so for a line
// not a polyline
void LinePlotter::PlotLine(double m[3], double n[3], unsigned char col[3])
{
    m_points->InsertNextPoint(m);
    m_points->InsertNextPoint(n);

    m_scalars->InsertNextTuple3(col[0], col[1], col[2]);

    m_lines->InsertNextCell(2);
    m_lines->InsertCellPoint(m_curPointID);
    m_lines->InsertCellPoint(m_curPointID + 1);

    m_curPointID += 2;

    // force the update
    m_scalars->Modified();
    m_points->Modified();
    m_lines->Modified();
}
void LinePlotter::PlotLine(
    double x,
    double y,
    double z,
    double x2,
    double y2,
    double z2,
    unsigned char color[3])
{
    double m[3], n[3];
    m[0] = x;
    m[1] = y;
    m[2] = z;
    n[0] = x2;
    n[1] = y2;
    n[2] = z2;
    PlotLine(m, n, color);
}

vtkActor * LinePlotter::getActor()
{
    return m_lineActors;
}
void LinePlotter::clear()
{
    m_curPointID = 0;
    m_points->Squeeze();
    m_lines->Squeeze();

    m_points->Reset();
    m_lines->Reset();

    m_lineActors->GetProperty()->SetLineWidth(m_allLineWidth);
}
