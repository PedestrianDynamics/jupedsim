/**
 * @headerfile PointPlotter2D.cpp
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

#include "PointPlotter2D.h"

#include "vtkActor.h"
#include "vtkDataArray.h"
#include "vtkDiskSource.h"
#include "vtkGlyph3D.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkUnsignedCharArray.h"

PointPlotter2D::PointPlotter2D()
{
    pts     = NULL;
    scalars = NULL;

    SetPointRadius();
    SetPointResolution();
}

PointPlotter2D::~PointPlotter2D()
{
    if(pts)
        pts->Delete();
    if(scalars)
        scalars->Delete();
}

void PointPlotter2D::PlotPoint(
    double x,
    double y,
    double z,
    unsigned char r,
    unsigned char g,
    unsigned char b)
{
    if(pts == NULL)
        pts = vtkPoints::New();

    if(scalars == NULL) {
        scalars = vtkUnsignedCharArray::New();
        scalars->SetNumberOfComponents(3);
    }

    pts->InsertNextPoint(x, y, z);
    scalars->InsertNextTuple3(r, g, b);
}

vtkPolyData * PointPlotter2D::CreatePolyData()
{
    vtkDiskSource * src = vtkDiskSource::New();
    src->SetRadialResolution(1);
    src->SetCircumferentialResolution(pt_res);

    src->SetInnerRadius(0.0);
    src->SetOuterRadius(pt_radius);


    vtkPolyData * polyData = vtkPolyData::New();
    polyData->SetPoints(pts);
    polyData->GetPointData()->SetScalars(scalars);

    vtkGlyph3D * glyph = vtkGlyph3D::New();
    glyph->SetSourceConnection(src->GetOutputPort());
    glyph->SetInputData(polyData);

    glyph->SetColorModeToColorByScalar();
    glyph->SetScaleModeToDataScalingOff();


    vtkPolyData * output = glyph->GetOutput();
    return output;
}

vtkActor * PointPlotter2D::CreateActor()
{
    vtkPolyData * polyData = CreatePolyData();

    vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
    mapper->SetInputData(polyData);

    vtkActor * actor = vtkActor::New();
    actor->SetMapper(mapper);

    return actor;
}
