/**
 * @file LinePlotter2D.h
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
 *
 *  Created on: 22.05.2009
 *
 */


#include "LinePlotter2D.h"
#include "../SystemSettings.h"

#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkAssembly.h>
#include <vtkSmartPointer.h>


#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

bool LinePlotter2D::doorColorsToDefault=true;

LinePlotter2D::LinePlotter2D()
:m_scalarMin(0.0), m_scalarMax(1.0)
{

	assembly=vtkAssembly::New();

	door_mapper= vtkPolyDataMapper::New();
	door_actor= vtkActor::New();
	door_points = vtkPoints::New();
	door_lines = vtkCellArray::New();
	door_lineScalars = vtkFloatArray::New();
	door_curPointID=0;
	door_width=3.5;

	wall_mapper= vtkPolyDataMapper::New();
	wall_actor= vtkActor::New();
	wall_points = vtkPoints::New();
	wall_lines = vtkCellArray::New();
	wall_lineScalars = vtkFloatArray::New();
	wall_curPointID=0;
	wall_width=2;


	// create a color lookup table
	m_lookupTable = vtkLookupTable::New();
	m_lookupTable->SetTableRange(0,255);
	m_lookupTable->SetNumberOfTableValues(256);

	//	m_lookupTable->SetHueRange(0.0,0.566);
	//	m_lookupTable->SetSaturationRange(0,0);
	//	m_lookupTable->SetValueRange(0.0,1.0);

	//m_lookupTable->SetHueRange(0.0,0.0);
	//m_lookupTable->SetValueRange(0.0,1.0);
	//m_lookupTable->SetSaturationRange(0.0,0.0);
	m_lookupTable->Build();



}

LinePlotter2D::~LinePlotter2D(){

	m_lookupTable->Delete();
	assembly->Delete();
	door_points->Delete();
	door_lines->Delete();
	door_lineScalars->Delete();
	door_mapper->Delete();
	door_actor->Delete();
	wall_points->Delete();
	wall_lines->Delete();
	wall_lineScalars->Delete();
	wall_mapper->Delete();
	wall_actor->Delete();
}

void LinePlotter2D::SetScalarRange(double minval, double maxval)
{
	m_scalarMin = minval ;
	m_scalarMax = maxval ;
}

//void LinePlotter2D::SetLookupTable(vtkLookupTable* table)
//{
//	m_lookupTable = table ;
//}
//void LinePlotter2D::PlotLine(double m[3], double n[3], double scalar)
//{
//
//	m_lineScalars->SetNumberOfComponents(1);
//	m_points->InsertNextPoint(m);
//	m_lineScalars->InsertNextTuple1(scalar);
//	m_points->InsertNextPoint(n);
//	m_lineScalars->InsertNextTuple1(scalar);
//
//	m_lines->InsertNextCell(2);
//	m_lines->InsertCellPoint(m_curPointID);
//	m_lines->InsertCellPoint(m_curPointID+1);
//
//	m_curPointID+=2;
//}
//void LinePlotter2D::PlotLine(double x, double y, double z,
//		double x2, double y2, double z2, double scalar)
//{
//	double m[3],n[3] ;
//	m[0]=x; m[1]=y; m[2]=z;
//	n[0]=x2; n[1]=y2; n[2]=z2;
//	PlotLine(m,n,scalar);
//
//}


void LinePlotter2D::SetAllLineWidth(int width)
{
	//m_allLineWidth = width ;
}

void LinePlotter2D::PlotDoor(double m[3], double n[3], double scalar)
{
	door_points->InsertNextPoint(m);
	door_lineScalars->InsertNextTuple1(scalar);
	door_points->InsertNextPoint(n);
	door_lineScalars->InsertNextTuple1(scalar);

	door_lines->InsertNextCell(2);
	door_lines->InsertCellPoint(door_curPointID);
	door_lines->InsertCellPoint(door_curPointID+1);

	door_curPointID+=2;

	if(scalar!=1.0){
		doorColorsToDefault=false;
	}
}

void LinePlotter2D::changeWallsColor(double *col)
{
	//first switch off the automatic mapping
	wall_mapper->SetScalarVisibility(0);
	//then set the new color
	wall_actor->GetProperty()->SetColor(col);
}

void LinePlotter2D::changeDoorsColor(double *col)
{
	//first switch off the automatic mapping
	door_mapper->SetScalarVisibility(0);
	//then set the new color
	door_actor->GetProperty()->SetColor(col);
}

void LinePlotter2D::PlotWall(double m[3], double n[3], double scalar)
{
	wall_points->InsertNextPoint(m);
	wall_lineScalars->InsertNextTuple1(scalar);
	wall_points->InsertNextPoint(n);
	wall_lineScalars->InsertNextTuple1(scalar);

	wall_lines->InsertNextCell(2);
	wall_lines->InsertCellPoint(wall_curPointID);
	wall_lines->InsertCellPoint(wall_curPointID+1);

	wall_curPointID+=2;
}

//vtkPolyData* LinePlotter2D::CreatePolyData()
//{
//	// Create poly data
//	vtkPolyData* polyData = vtkPolyData::New();
//	polyData->SetPoints(m_points);
//	polyData->SetLines(m_lines);
//	polyData->GetPointData()->SetScalars(m_lineScalars);
//	return polyData;
//}

//vtkActor* LinePlotter2D::CreateActor()
//{
//	// Create poly data
//	vtkPolyData* polyData = CreatePolyData();
//	// create mapper
//	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
//	mapper->SetInput(polyData);
//	mapper->SetLookupTable(m_lookupTable);
//	mapper->SetColorModeToMapScalars();
//	//mapper->SetScalarRange(m_scalarMin, m_scalarMax);
//	mapper->SetScalarModeToUsePointData();
//
//	// create actor
//	vtkActor* actor = vtkActor::New();
//	actor->SetMapper(mapper);
//	actor->GetProperty()->SetLineWidth(m_allLineWidth);
//
//	return actor ;
//}

vtkAssembly* LinePlotter2D::createAssembly(){
	//doors
	{
		// Create poly data
		//vtkPolyData* polyData =vtkPolyData::New();
		VTK_CREATE(vtkPolyData,polyData);
		polyData->SetPoints(door_points);
		polyData->SetLines(door_lines);
		polyData->GetPointData()->SetScalars(door_lineScalars);

		// create mapper

#if VTK_MAJOR_VERSION <= 5
        door_mapper->SetInput(polyData);
#else
        door_mapper->SetInputData(polyData);
#endif
		door_mapper->SetLookupTable(m_lookupTable);
		door_mapper->SetColorModeToMapScalars();
		//mapper->SetScalarRange(m_scalarMin, m_scalarMax);
		door_mapper->SetScalarModeToUsePointData();
		// create actor
		door_actor->SetMapper(door_mapper);
		door_actor->GetProperty()->SetLineWidth(door_width);
		assembly->AddPart(door_actor);

		//if default, then hide all doors
		// fixme: not working
		if(doorColorsToDefault){
			double col[3]={1.0,1.0,1.0};
			SystemSettings::getBackgroundColor(col);
			door_actor->GetProperty()->SetColor(col);
			door_actor->Modified();
		}
	}

	//walls
	{
		// Create poly data
		VTK_CREATE(vtkPolyData,polyData);
		polyData->SetPoints(wall_points);
		polyData->SetLines(wall_lines);
		polyData->GetPointData()->SetScalars(wall_lineScalars);
        // create mapper
#if VTK_MAJOR_VERSION <= 5
        wall_mapper->SetInput(polyData);
#else
        wall_mapper->SetInputData(polyData);
#endif
		wall_mapper->SetLookupTable(m_lookupTable);
		wall_mapper->SetColorModeToMapScalars();
		//mapper->SetScalarRange(m_scalarMin, m_scalarMax);
		wall_mapper->SetScalarModeToUsePointData();
		// create actor
		wall_actor->SetMapper(wall_mapper);
		wall_actor->GetProperty()->SetLineWidth(wall_width);
		assembly->AddPart(wall_actor);
	}

	return assembly;
}

void LinePlotter2D::showDoors(bool status){
	door_actor->SetVisibility(status);
}
void LinePlotter2D::showWalls(bool status){
	wall_actor->SetVisibility(status);
}
