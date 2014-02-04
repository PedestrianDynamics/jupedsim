/*
 * PointPlotter.h
 *
 *  Created on: 17.05.2009
 *      Author: Ulrich  Kemloh
 */

#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkDataArray.h>
#include <vtkGlyph3D.h>
#include <vtkDiskSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

#include "JPoint.h"
#include "PointPlotter.h"
#include "./src/SystemSettings.h"


#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

PointPlotter::PointPlotter()
{

	pts = vtkPoints::New();
	//pts->Allocate(30);
	//pts->SetNumberOfPoints(30);
	//pts->SetNumberOfPoints(MAX_POINTS);
	SetPointRadius(2);
	SetPointResolution();

	scalars = vtkUnsignedCharArray::New();
	//scalars->Allocate(30);
	scalars->SetNumberOfComponents(3);
	//scalars->setn

	//vtkDiskSource* src = vtkDiskSource::New();
	VTK_CREATE(vtkDiskSource,src);
	src->SetRadialResolution(5);
	src->SetCircumferentialResolution(pt_res);
	src->SetInnerRadius(0.00);
	src->SetOuterRadius(pt_radius);


	//vtkPolyData* polyData = vtkPolyData::New();
	VTK_CREATE(vtkPolyData,polyData);
	polyData->SetPoints(pts);
	polyData->GetPointData()->SetScalars(scalars);

//	vtkGlyph3D* glyph = vtkGlyph3D::New();
	VTK_CREATE(vtkGlyph3D,glyph);
	glyph->SetSourceConnection(src->GetOutputPort());
    glyph->SetInputData(polyData);
	//src->Delete();
	//polyData->Delete();
	glyph->SetColorModeToColorByScalar();
	glyph->SetScaleModeToDataScalingOff() ;


	//vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
	VTK_CREATE(vtkPolyDataMapper,mapper);
    mapper->SetInputConnection(glyph->GetOutputPort());
	//glyph->Delete();

	//vtkActor
	pointActor = vtkActor::New();
	pointActor->SetMapper(mapper);
	//mapper->Delete();

	/// initizliae the ID
	nextPointID=0;
}

PointPlotter::~PointPlotter()
{
	if (pts)
		pts->Delete();
	if (scalars)
		scalars->Delete();
	if (pointActor)
		pointActor->Delete();
}


/***
 * add a point to the plot
 */

void PointPlotter::PlotPoint(JPoint * point){
	double x=point->getX();
	double y=point->getY();
	double z=point->getZ();

	unsigned char r=point->getR();
	unsigned char b=point->getB();
	unsigned char g=point->getG();
	PlotPoint( x,  y,  z, r,   g,   b);
}

void PointPlotter::PlotPoint(double x, double y, double z,
		unsigned char r, unsigned char g, unsigned char b)
{

	nextPointID++;
	int PointsCount=1, dummy=0,dummy1;;

	SystemSettings::getTrailsInfo(&PointsCount,&dummy,&dummy1);

	nextPointID=nextPointID%PointsCount;
	pts->InsertPoint(nextPointID,x,y,z);
	//pts->SetPoint(nextPointID,x,y,z);
	//scalars->SetTuple3(nextPointID,r,g,b);
	scalars->InsertTuple3(nextPointID,r,g,b);
	//scalars->InsertNextTuple3(r,g,b);

	pts->Modified();
	scalars->Modified();

}


vtkActor *  PointPlotter::getActor(){ return pointActor;}
