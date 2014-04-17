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
#include <vtkMath.h>
#include <vtkIntArray.h>


#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

Frame::Frame() {
	elementCursor=0;
	_polydata = vtkPolyData::New();
    _polydataLabels = vtkPolyData::New();
}

Frame::~Frame() {
	while (!framePoints.empty()){
		delete framePoints.back();
		framePoints.pop_back();
	}
	framePoints.clear();

	_polydata->Delete();
    _polydataLabels->Delete();
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

//	VTK_CREATE (vtkPoints, points);
//	VTK_CREATE (vtkFloatArray, colors);
//	colors->SetName("color");
//	//colors->SetNumberOfComponents(3);
//	colors->SetNumberOfComponents(1);
//	for (unsigned int i=0;i<framePoints.size();i++){
//		double pos[3];
//		double data[7];
//		framePoints[i]->getPos(pos);
//		framePoints[i]->getEllipse(data);
//		points->InsertNextPoint(pos);
//		if(data[6]==-1){
//		colors->InsertNextValue(NAN);
//		}
//		else{
//		colors->InsertNextValue(data[6]/255.0);
//		}
//	}
//	//scalars->Print(cout);
//	VTK_CREATE (vtkFloatArray, data);
//	data->SetNumberOfComponents(2);
//	data->SetNumberOfTuples(framePoints.size());
//	data->CopyComponent(0, colors, 0);
//	data->CopyComponent(1, colors, 0); // radius can come here later
//	data->SetName("data");

//	_polydata->SetPoints(points);
//	_polydata->GetPointData()->AddArray(data);
//	_polydata->GetPointData()->SetActiveScalars("data");


    VTK_CREATE (vtkPoints, points);
    VTK_CREATE (vtkFloatArray, colors);
    VTK_CREATE (vtkFloatArray, tensors);
    VTK_CREATE (vtkIntArray, labels);

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    labels->SetName("labels");
    labels->SetNumberOfComponents(1);

    for (unsigned int i=0;i<framePoints.size();i++){
        double pos[3]={0,0,0};
        double rad[3];
        double rot[3];

        framePoints[i]->getPos(pos); //pos[2]=90;
        points->InsertNextPoint(pos);
        labels->InsertNextValue(framePoints[i]->getIndex()+1);

        double data[7];
        framePoints[i]->getEllipse(data);


        //framePoints[i]->GetRadius(rad);
        rad[0]=data[3]/30;
        rad[1]=data[4]/30;
        rad[2]=30.0/120.0;

        //rad[0]=1;
       //rad[1]=1;
       //rad[2]=1.0;

        rot[0]=vtkMath::RadiansFromDegrees(0.0);
        rot[1]=vtkMath::RadiansFromDegrees(0.0);
        rot[2]=vtkMath::RadiansFromDegrees(data[5]);

        //scaling matrix
        double sc[3][3] = {{rad[0],0,0},
                          {0,rad[1],0},
                          {0,0,rad[2]}};


        //rotation matrix around x-axis
        double roX[3][3] = {{1, 0,                    0},
                            {0, cos(rot[0]),-sin(rot[0])},
                            {0, sin(rot[0]), cos(rot[0])}};

        //rotation matrix around y-axis
        double roY[3][3] = {{cos(rot[1]), 0,sin(rot[1])},
                            {0,           1,          0},
                            {-sin(rot[1]),0,cos(rot[1])}};

        //rotation matrix around z-axis
        double roZ[3][3] = {{cos(rot[2]),sin(rot[2]),0.0},
                            {-sin(rot[2]),cos(rot[2]),0.0},
                            {0.0,0.0,1.0}};

        //final rotation matrix
        double ro[3][3];
        vtkMath::Multiply3x3(roX,roY,ro);
        vtkMath::Multiply3x3(ro,roZ,ro);


        //final transformation matrix
        double rs[3][3];
        vtkMath::Multiply3x3(sc,ro,rs);

        tensors->InsertNextTuple9(rs[0][0],rs[0][1],rs[0][2],
                                  rs[1][0],rs[1][1],rs[1][2],
                                  rs[2][0],rs[2][1],rs[2][2]);


        //color
        if(data[6]==-1){
            colors->InsertNextValue(NAN);
        }
        else{
            colors->InsertNextValue(data[6]/255.0);
        }

      }

    // setting the colors
    _polydata->SetPoints(points);
    _polydata->GetPointData()->AddArray(colors);
    _polydata->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    _polydata->GetPointData()->SetTensors(tensors);
    _polydata->GetPointData()->SetActiveTensors("tensors");


    // setting the labels
    _polydata->GetPointData()->AddArray(labels);
    //_polydata->GetPointData()->set

    //labels
    //_polydataLabels->SetPoints(points);

    return _polydata;
}

vtkPolyData *Frame::GetPolyDataLabels()
{
 return _polydataLabels;
}

unsigned int Frame::getElementCursor(){

	return elementCursor;
}

void Frame::resetCursor(){
	elementCursor=0;
}
