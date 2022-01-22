/*
 * PointPlotter.h
 *
 *  Created on: 17.05.2009
 *      Author: Ulrich  Kemloh
 */

#include "PointPlotter.hpp"

#include "JPoint.hpp"
#include "Settings.hpp"
#include "general/Macros.hpp"

#include <vtkActor.h>
#include <vtkDataArray.h>
#include <vtkDiskSource.h>
#include <vtkFloatArray.h>
#include <vtkGlyph3D.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRegularPolygonSource.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>


PointPlotter::PointPlotter() :
    pts(vtkPoints::New()), colors(vtkFloatArray::New()), pointActor(vtkActor::New())
{
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(pts);
    polyData->GetPointData()->SetScalars(colors);

    vtkNew<vtkRegularPolygonSource> src;
    src->SetRadius(pt_radius);
    src->SetNumberOfSides(5);

    vtkNew<vtkGlyph3D> glyph;
    glyph->SetSourceConnection(src->GetOutputPort());
    glyph->SetInputData(polyData);
    glyph->SetColorModeToColorByScalar();
    glyph->SetScaleModeToDataScalingOff();


    vtkNew<vtkLookupTable> lut;
    lut->SetHueRange(0.0, 0.470);
    lut->SetValueRange(1.0, 1.0);
    lut->SetNanColor(0.2, 0.2, 0.2, 0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(glyph->GetOutputPort());
    mapper->SetLookupTable(lut);

    pointActor->SetMapper(mapper);
}

void PointPlotter::PlotPoint(const glm::dvec3 & pos, double color)
{
    nextPointID++;
    if(color == -1) {
        colors->InsertTuple1(nextPointID, NAN);
    } else {
        colors->InsertTuple1(nextPointID, color / 255.0);
    }

    pts->InsertPoint(nextPointID, pos.x, pos.y, pos.x);
    pts->Modified();
    colors->Modified();
}

void PointPlotter::SetVisibility(bool status)
{
    pointActor->SetVisibility(status);
}

vtkSmartPointer<vtkActor> PointPlotter::getActor()
{
    return pointActor;
}
