/**
 * @file FacilityGeometry.cpp
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
 * @brief method for plotting the different geometry/Building elements
 *
 *  Created on: 07.05.2009
 *
 */

#include "FacilityGeometry.h"
#include "Point.h"
#include "../SystemSettings.h"
#include "LinePlotter2D.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkAssembly.h>
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>
#include <vtkDiskSource.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkProp3DCollection.h>
#include <vtkSmartPointer.h>

#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkCaptionWidget.h>
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>
#include <vtkActor2DCollection.h>


#define PI 3.1415926535

using namespace std;

#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


FacilityGeometry::FacilityGeometry() {

	assembly = vtkAssembly::New();
	assembly2D = vtkAssembly::New();
	assemblyCaptions= vtkAssembly::New();

	captions=vtkActor2DCollection::New();

	linesPlotter2D = new LinePlotter2D();

	// initializing the lookup table for the colors
	// rainbow colors ranging from red to blue
	lookupTable =  vtkLookupTable::New();
	lookupTable->SetTableRange(0,255);
	//lookupTable->SetHueRange(0.0,0.566);
	//lut->SetSaturationRange(0,0);
	//lut->SetValueRange(0.0,1.0);
	lookupTable->SetNumberOfTableValues(256);
	lookupTable->Build();

	// geometry parameters all in cm
	doorThickness = 3;
	wallThickness = 3;
	wallHeight=250;
	doorHeight=250;
	stepHeight = 40;
	wallColor = 255;
	stepColor = 130;
	doorColor = 50;

}

FacilityGeometry::~FacilityGeometry() {
	if(assembly)
		assembly->Delete();

	lookupTable->Delete();

	delete linesPlotter2D;
}

//todo:
// each time this is called, the assemblies parts are added
// very bad

vtkAssembly* FacilityGeometry::getActor(){

	assembly2D->AddPart(linesPlotter2D->createAssembly());
	assembly2D->AddPart(assemblyCaptions);

	return assembly2D;
}


//void FacilityGeometry::drawWall(Point *p1, Point *p2){
//	double scale =1;
//
//	double *center = p1->centreCoordinatesWith(*p2);
//	double angle =p1->angleMadeWith(*p2);
//
//	vtkCylinderSource* src = vtkCylinderSource::New();
//	src->SetResolution(10);
//	src->SetCenter(center[0],center[1],center[2]);
//	src->SetRadius(scale*3);
//	src->SetHeight(p1->distanceTo(*p2));
//	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
//	mapper->SetInputConnection(src->GetOutputPort());
//	vtkActor* actor = vtkActor::New();
//	actor->SetMapper(mapper);
//	actor->GetProperty()->SetLighting(true);
//	actor->SetOrigin(center[0],center[1],center[2]);
//	//actor->SetOrientation(0,0,90);
//	//actor->RotateZ(90);
//	actor->RotateZ(angle);
//	actor->GetProperty()->SetColor(0.0,0.0,1.0);
//	actor->GetProperty()->SetAmbient(0.2);
//	actor->GetProperty()->SetDiffuse(0.8);
//
//	assembly->AddPart(actor);
//
//}

//void FacilityGeometry::drawDoor(Point *p1, Point *p2){
//	double *center = p1->centreCoordinatesWith(*p2);
//	double angle =p1->angleMadeWith(*p2);
//
//	vtkCylinderSource* src = vtkCylinderSource::New();
//	src->SetResolution(40);
//	src->SetCenter(center[0],center[1],center[2]);
//	src->SetRadius(4);
//	src->SetHeight(p1->distanceTo(*p2));
//	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
//	mapper->SetInputConnection(src->GetOutputPort());
//	vtkActor* actor = vtkActor::New();
//	actor->SetMapper(mapper);
//	actor->GetProperty()->SetLighting(true);
//	actor->SetOrigin(center[0],center[1],center[2]);
//	//actor->SetOrientation(0,0,90);
//	//actor->RotateZ(90);
//	actor->RotateZ(angle);
//	//actor->GetProperty()->SetColor(0.0,0.1,0.0);
//	actor->GetProperty()->SetColor(1.0,0.41,0.72);
//	//actor->GetProperty()->SetColor(0.0,0.0,0.0);
//	assembly->AddPart(actor);
//
//}

/***
 * This is the main build method and should be called by all functions
 * drawing a wall or a door. Important
 */
//void FacilityGeometry::addNewElement(double center[3], double length, double orientation, ELEMENT_TYPE type){
//
//	vtkCubeSource* src = vtkCubeSource::New();
//	src->SetCenter(center[0],center[1],center[2]);
//	src->SetYLength(length);
//
//	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
//	mapper->SetInputConnection(src->GetOutputPort());
//	src->Delete();
//
//	vtkActor* actor = vtkActor::New();
//	actor->SetMapper(mapper);
//	mapper->Delete();
//	actor->GetProperty()->SetLighting(true);
//	actor->SetOrigin(center[0],center[1],center[2]);
//	actor->RotateZ(orientation);
//	actor->GetProperty()->SetAmbient(0.2);
//	actor->GetProperty()->SetDiffuse(0.8);
//
//	// 	double data[3];
//	// 	actor->GetPosition(data);
//	// 	actor->SetPosition(data[0],data[1],src->GetZLength()/2);
//
//	switch (type)
//	{
//	case DOOR:
//	{
//		double colorRGB[3];
//		lookupTable->GetColor(doorColor, colorRGB);
//		actor->GetProperty()->SetColor(colorRGB);
//		src->SetXLength(doorThickness);
//		src->SetZLength(doorHeight);
//		//src->SetRadius(doorWidth);
//		actor->GetProperty()->SetOpacity(1.00);
//		assemblyDoors3D->AddPart(actor);
//	}
//	break;
//	case WALL:
//	{
//		double colorRGB[3];
//		lookupTable->GetColor(wallColor, colorRGB);
//		actor->GetProperty()->SetColor(colorRGB);
//		//actor->GetProperty()->SetSpecular(1);
//		//actor->GetProperty()->SetDiffuse(1);
//		//actor->GetProperty()->SetAmbient(1);
//
//		src->SetXLength(wallThickness);
//		src->SetZLength(wallHeight);
//		//src->SetRadius(wallWidth);
//		assemblyWalls3D->AddPart(actor);
//	}
//	break;
//	case STEP:
//	{
//		double colorRGB[3];
//		lookupTable->GetColor(stepColor, colorRGB);
//		actor->GetProperty()->SetColor(colorRGB);
//		src->SetXLength(wallThickness); //FIXME, this is wrong
//		src->SetZLength(stepHeight);
//		assemblyDoors3D->AddPart(actor);
//	}
//	break;
//
//	//default behaviour not defined
//	default:
//		break;
//	}
//
//	// now adjusting the z coordinates
//	double data[3];
//	actor->GetPosition(data);
//	actor->SetPosition(data[0],data[1],src->GetZLength()/2);
//
//	actor->Delete();
//}

void FacilityGeometry::addWall(double x1, double y1, double z1, double x2, double y2, double z2,double thickness,double height,double color){

	// all walls will have this parameters until changed
	wallColor=color;

	//	if(SystemSettings::get2D()){
	double m[]={x1,y1,z1};
	double n[]={x2,y2,z2};
	linesPlotter2D->PlotWall(m,n,wallColor/255.0);
}

void FacilityGeometry::addDoor(double x1, double y1, double z1, double x2, double y2, double z2,double thickness ,double height, double color){

	// all doors will take this color upon changed
	doorColor=color;
	//constructing the 2D assembly
	//	if(SystemSettings::get2D()){
	double m[]={x1,y1,z1};
	double n[]={x2,y2,z2};

	//double m[]={x1,y1,z+1};
	//double n[]={x2,y2,z+1};
	//printf("[%f %f %f]---[%f %f %f]\n",x1,y1,z,x2,y2,z);

	linesPlotter2D->PlotDoor(m,n,doorColor/255.0);
}

void FacilityGeometry::addStep(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double m[]={x1,y1,z1};
	double n[]={x2,y2,z2};
	linesPlotter2D->PlotDoor(m,n,doorColor/255.0);
}

void FacilityGeometry::addStep(Point* p1, Point* p2){

	double m[3];
	double n[3];
	double CHT[3];

	p1->getXYZ(m);
	p2->getXYZ(n);
	p1->getColorHeightThicknes(CHT);

	stepHeight=CHT[1];
	stepColor = CHT[0];

	linesPlotter2D->PlotDoor(m,n,doorColor/255.0);

}

void FacilityGeometry::addWall(Point* p1, Point* p2, string caption){
	double m[3];
	double n[3];
	double CHT[3];
	p1->getXYZ(m);
	p2->getXYZ(n);
	p1->getColorHeightThicknes(CHT);

	wallThickness = CHT[2];
	wallHeight=CHT[1];
	wallColor = CHT[0];
	linesPlotter2D->PlotWall(m,n,wallColor/255.0);


	if (caption.compare("") != 0){

		double center[3];
		center[0]=0.5*(m[0]+n[0]);
		center[1]=0.5*(m[1]+n[1]);
		center[2]=0.5*(m[2]+n[2]);
		double orientation[3]={0,0,0};
		addNewElementText(center,orientation,caption.c_str(),50);
	}

}

void FacilityGeometry::addDoor(Point* p1, Point* p2, string caption){

	double m[3];
	double n[3];
	double CHT[3];

	p1->getXYZ(m);
	p2->getXYZ(n);
	//to get the exits over the walls
	//m[0]++;	m[1]++;	m[2]++;
	//n[0]++;	n[1]++;	n[2]++;
	p1->getColorHeightThicknes(CHT);

	doorThickness = CHT[2];
	doorHeight=CHT[1];
	doorColor = CHT[0];

	linesPlotter2D->PlotDoor(m,n,doorColor/255.0);



	if (caption.compare("") != 0){

		double center[3];
		center[0]=0.5*(m[0]+n[0]);
		center[1]=0.5*(m[1]+n[1]);
		center[2]=0.5*(m[2]+n[2]);
		double orientation[3]={0,0,0};
		addNewElementText(center,orientation,caption.c_str(),0);
	}

}


void FacilityGeometry::addFloor(double x1, double y1, double x2, double y2, double z){
	//if(z!=1)return;
	const double cellSize=40; //cm
	//	const int dimX=(x2-x1)/cellSize+1;
	//	const int dimY=(y2-y1)/cellSize+1;
	const int dimX= (int)ceil((x2-x1)/cellSize) +1;
	const int dimY= (int)ceil((y2-y1)/cellSize) +1;


	const int dimZ=1;
	//vtkDoubleArray *scalars = vtkDoubleArray::New();
	vtkDataArray* pData = vtkUnsignedCharArray::New();
	pData->SetNumberOfComponents(3);

	double color[2][3]={{100, 100, 100},{150,150,150}};
	bool idx=0;
	bool firstIdx=1;

	//	for(int i=0;i<dimX-1;i++){
	//		firstIdx=!firstIdx;
	//		idx=firstIdx;
	//		for(int j=0;j<dimY-1;j++){
	//			pData->InsertNextTuple3(color[idx][0],color[idx][1],color[idx][2]);
	//			idx=!idx;
	//		}
	//	}
	//	bool lastColorUsed=0;
	//	for(int i=0;i<dimX-1;i++){
	//
	//			if(idx==lastColorUsed){
	//				lastColorUsed= !lastColorUsed;
	//				idx=lastColorUsed;
	//			}else{
	//				lastColorUsed=idx;
	//			}
	//
	//			for(int j=0;j<dimY-1;j++){
	//				pData->InsertNextTuple3(color[idx][0],color[idx][1],color[idx][2]);
	//				idx=!idx;
	//			}
	//		}

	bool lastColorUsed=0;
	for(int i=0;i<dimY-1;i++){

		if(idx==lastColorUsed){
			lastColorUsed= !lastColorUsed;
			idx=lastColorUsed;
		}else{
			lastColorUsed=idx;
		}

		for(int j=0;j<dimX-1;j++){
			pData->InsertNextTuple3(color[idx][0],color[idx][1],color[idx][2]);
			idx=!idx;
		}
	}

	// data as cellData of imageData
	VTK_CREATE(vtkImageData, image);
	image->SetDimensions(dimX, dimY, dimZ);
	image->SetSpacing(cellSize, cellSize, cellSize);
	image->GetCellData()->SetScalars(pData);


	VTK_CREATE(vtkActor, imageActor);
	VTK_CREATE(vtkDataSetMapper, map);
    map->SetInputData(image);
	//map->SetLookupTable(lookupTable);
	imageActor->SetMapper(map);
	imageActor->GetProperty()->SetAmbient(0.2);
	//imageActor->GetProperty()->SetDiffuse(0.8);

	// move the actor in x-direction
	imageActor->SetPosition(x1, y1, z);
	assembly2D->AddPart(imageActor);

}


void FacilityGeometry::addObjectSphere(double center[3], double radius,
		double color) {

	double colorRGB[3];
	lookupTable->GetColor(color, colorRGB);

	//create a disk for the 2D world
	{
		VTK_CREATE(vtkDiskSource, disk);
		disk->SetCircumferentialResolution(10);
		disk->SetInnerRadius(0);
		disk->SetOuterRadius(radius);

		VTK_CREATE(vtkPolyDataMapper, mapper);
		mapper->SetInputConnection(disk->GetOutputPort());

		VTK_CREATE(vtkActor, actor);
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(colorRGB);

		actor->SetPosition(center[0], center[1], 0);
		assembly2D->AddPart(actor);
	}
}


void FacilityGeometry::addObjectCylinder(double center[3], double radius,
		double height, double orientation[3],double color) {

	double colorRGB[3];
	lookupTable->GetColor(color, colorRGB);

	//create a disk for the 2D world
	//TODO: this is of course a wrong projection
	{
		VTK_CREATE(vtkDiskSource, disk);
		disk->SetCircumferentialResolution(10);
		disk->SetInnerRadius(0);
		disk->SetOuterRadius(radius);

		VTK_CREATE(vtkPolyDataMapper, mapper);
		mapper->SetInputConnection(disk->GetOutputPort());

		VTK_CREATE(vtkActor, actor);
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(colorRGB);

		actor->SetPosition(center[0], center[1], 0);
		assembly2D->AddPart(actor);
	}
}

void FacilityGeometry::addObjectBox(double center[3], double height,
		double width, double length, double color) {

	double colorRGB[3];
	lookupTable->GetColor(color, colorRGB);

	{
		VTK_CREATE(vtkCubeSource,src);
		src->SetCenter(center[0], center[1], 0);
		src->SetZLength(1); //todo: fake projection
		src->SetYLength(length);
		src->SetXLength(width);

		VTK_CREATE(vtkPolyDataMapper,mapper);
		mapper->SetInputConnection(src->GetOutputPort());

		VTK_CREATE(vtkActor,actor);
		actor->GetProperty()->SetLighting(true);
		actor->SetOrigin(center[0], center[1], 0);
		actor->GetProperty()->SetColor(colorRGB);
		actor->GetProperty()->SetAmbient(0.2);
		actor->GetProperty()->SetDiffuse(0.8);
		actor->SetMapper(mapper);
		assembly2D->AddPart(actor);
	}
}

void FacilityGeometry::changeWallsColor(double* color){

	linesPlotter2D->changeWallsColor(color);
	assembly2D->Modified();
}

void FacilityGeometry::changeExitsColor(double* color){

	linesPlotter2D->changeDoorsColor(color);
	assembly2D->Modified();

}

void FacilityGeometry::set2D(bool status){
	assembly2D->SetVisibility(status);
}

void FacilityGeometry::set3D(bool status){
}

void FacilityGeometry::showDoors(bool status){

	linesPlotter2D->showDoors(status);
	assembly2D->Modified();
}

void FacilityGeometry::showStairs(bool status){

}

void FacilityGeometry::showWalls(bool status){
	linesPlotter2D->showWalls(status);
	assembly2D->Modified();
}

void FacilityGeometry::addObjectLabel(double center[3], double orientation[3], std::string caption, double color){
	addNewElementText(center, orientation, caption, color);
}

vtkActor2DCollection* FacilityGeometry::getCaptions(){
	return captions;
}
// orientation and color ignored
void FacilityGeometry::addNewElementText(double center[3], double orientation[3],
		string text, double color) {

	return ;

		{//caption
			VTK_CREATE(vtkTextActor3D,caption);
			caption = vtkTextActor3D ::New();

			//caption->SetVisibility(false);
			caption->SetInput(text.c_str());
			// set the properties of the caption
			//FARBE
			vtkTextProperty* tprop = caption->GetTextProperty();
			//tprop->SetFontFamilyToArial();
			//tprop->BoldOn();
			//tprop->ShadowOn();
			//tprop->SetLineSpacing(1.0);
			tprop->SetFontSize(SystemSettings::getPedestrianCaptionSize());

			double colorRGB[3];
			lookupTable->GetColor(color, colorRGB);
			tprop->SetColor(colorRGB);

			//cout<<"size:" <<tprop->GetFontSize()<<endl;
			//tprop->SetColor(1,0,0);

			caption->SetPosition(center);
			assemblyCaptions->AddPart(caption);

		}
}

void FacilityGeometry::showGeometryLabels(int status){

	vtkProp3DCollection* col=assemblyCaptions->GetParts();
	assemblyCaptions->GetActors(col);

	int count = col->GetNumberOfItems();
	for (int i=0;i<count;i++){
		((vtkActor*)col->GetItemAsObject(i))->SetVisibility(status);
	}
	assemblyCaptions->Modified();

}

