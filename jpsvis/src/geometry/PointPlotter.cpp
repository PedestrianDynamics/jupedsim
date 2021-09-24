/*
 * PointPlotter.h
 *
 *  Created on: 17.05.2009
 *      Author: Ulrich  Kemloh
 */

#include "PointPlotter.h"

#include "../SystemSettings.h"
#include "JPoint.h"

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


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

PointPlotter::PointPlotter()
{
    pts = vtkPoints::New();
    // pts->Allocate(30);
    // pts->SetNumberOfPoints(30);
    // pts->SetNumberOfPoints(MAX_POINTS);
    SetPointRadius(2);
    SetPointResolution();

    scalars = vtkUnsignedCharArray::New();
    scalars->SetNumberOfComponents(3);
    colors = vtkFloatArray::New();

    //	VTK_CREATE(vtkDiskSource,src);
    //	src->SetRadialResolution(5);
    //	src->SetCircumferentialResolution(pt_res);
    //	src->SetInnerRadius(0.00);
    //	src->SetOuterRadius(pt_radius);

    VTK_CREATE(vtkRegularPolygonSource, src);
    src->SetRadius(2.0);
    src->SetNumberOfSides(5);


    VTK_CREATE(vtkPolyData, polyData);
    polyData->SetPoints(pts);
    // polyData->GetPointData()->SetScalars(scalars);
    polyData->GetPointData()->SetScalars(colors);

    VTK_CREATE(vtkGlyph3D, glyph);
    glyph->SetSourceConnection(src->GetOutputPort());
    glyph->SetInputData(polyData);

    glyph->SetColorModeToColorByScalar();
    glyph->SetScaleModeToDataScalingOff();


    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(glyph->GetOutputPort());

    // borrow the lookup table from the peds glyphs
    if(extern_glyphs_pedestrians_actor_2D->GetMapper())
        mapper->SetLookupTable(extern_glyphs_pedestrians_actor_2D->GetMapper()->GetLookupTable());

    // vtkActor
    pointActor = vtkActor::New();
    pointActor->SetMapper(mapper);

    /// initizliae the ID
    nextPointID = 0;
}

PointPlotter::~PointPlotter()
{
    if(pts)
        pts->Delete();
    if(scalars)
        scalars->Delete();
    if(colors)
        colors->Delete();
    if(pointActor)
        pointActor->Delete();
}


/***
 * add a point to the plot
 */

void PointPlotter::PlotPoint(JPoint * point)
{
    double x = point->getX();
    double y = point->getY();
    double z = point->getZ();

    unsigned char r = point->getR();
    unsigned char b = point->getB();
    unsigned char g = point->getG();
    PlotPoint(x, y, z, r, g, b);
}

void PointPlotter::PlotPoint(double pos[3], double col)
{
    nextPointID++;
    if(col == -1) {
        colors->InsertTuple1(nextPointID, NAN);
    } else {
        colors->InsertTuple1(nextPointID, col / 255.0);
    }

    pts->InsertPoint(nextPointID, pos);
    pts->Modified();
    colors->Modified();
}

void PointPlotter::PlotPoint(
    double x,
    double y,
    double z,
    unsigned char r,
    unsigned char g,
    unsigned char b)
{
    nextPointID++;
    int PointsCount = 1, dummy = 0, dummy1;
    ;

    SystemSettings::getTrailsInfo(&PointsCount, &dummy, &dummy1);

    // nextPointID=nextPointID%PointsCount;
    pts->InsertPoint(nextPointID, x, y, z);
    // pts->SetPoint(nextPointID,x,y,z);
    // scalars->SetTuple3(nextPointID,r,g,b);
    scalars->InsertTuple3(nextPointID, r, g, b);
    // scalars->InsertNextTuple3(r,g,b);

    pts->Modified();
    scalars->Modified();
}

void PointPlotter::SetVisibility(bool status)
{
    pointActor->SetVisibility(status);
}

vtkActor * PointPlotter::getActor()
{
    return pointActor;
}
