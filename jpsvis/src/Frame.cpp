/**
* @file Frame.cpp
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
* this class contains the collection of all
* pedestrians coordinates (trajectoryPoint) belonging to the same frame(i.e at the same time)
*
* @brief Hold all information that will be displayed on the screen (one frame)
*
*  Created on: 10.07.2009
*
*/

#include <vector>
#include <iostream>

#include "TrajectoryPoint.h"
#include "Frame.h"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>


#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

Frame::Frame() {
	elementCursor=0;
	_polydata = vtkPolyData::New();
}

Frame::~Frame() {
	while (!framePoints.empty()){
		delete framePoints.back();
		framePoints.pop_back();
	}
	framePoints.clear();

	_polydata->Delete();
}

int Frame::getSize(){
	return this->framePoints.size();
}

void Frame::addElement(TrajectoryPoint* point){
	framePoints.push_back(point);
}

void Frame::clear(){
	while (!framePoints.empty()){
		delete framePoints.back();
		framePoints.pop_back();
	}
	framePoints.clear();

	elementCursor=0;
}

TrajectoryPoint* Frame::getNextElement(){
/*
	TrajectoryPoint* res;
	if(framePoints.empty()) {
		return NULL;
	}
	else{
		 res= framePoints.front();
		 // dont delete the elements.....
		 framePoints.erase(framePoints.begin());
		 return res;
	}
*/



	//The navigation is only allowed in one direction
	// so elementCursor cannot be negative
	if (elementCursor<0) {
		//Debug::Messages("CURSOR VALUE(NULL): %d" ,elementCursor);
		return NULL;
	}

	if(elementCursor>=framePoints.size()) {
		return NULL;

	}else{
		return framePoints.at(elementCursor++);
	}
//next test
}

vtkPolyData* Frame::GetPolyData() {

	VTK_CREATE (vtkPoints, points);
	VTK_CREATE (vtkFloatArray, colors);
	colors->SetName("color");
	//colors->SetNumberOfComponents(3);
	colors->SetNumberOfComponents(1);
	for (unsigned int i=0;i<framePoints.size();i++){
		double pos[3];
		double data[7];
		framePoints[i]->getPos(pos);
		framePoints[i]->getEllipse(data);
		points->InsertNextPoint(pos);
		if(data[6]==-1){
		colors->InsertNextValue(NAN);
		}
		else{
		colors->InsertNextValue(data[6]/255.0);
		}
	}
	//scalars->Print(cout);
	VTK_CREATE (vtkFloatArray, data);
	data->SetNumberOfComponents(2);
	data->SetNumberOfTuples(framePoints.size());
	data->CopyComponent(0, colors, 0);
	data->CopyComponent(1, colors, 0); // radius can come here later
	data->SetName("data");

	_polydata->SetPoints(points);
	_polydata->GetPointData()->AddArray(data);
	_polydata->GetPointData()->SetActiveScalars("data");
	return _polydata;
}

unsigned int Frame::getElementCursor(){

	return elementCursor;
}

void Frame::resetCursor(){
	elementCursor=0;
}
