/**
 * @file Pedestrian.cpp
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
 * @brief
 *
 *  Created on: 05.05.2009
 *
 */

// paraview tutorial for glyph
// http://www.mail-archive.com/paraview@paraview.org/msg02142.html
#include "Pedestrian.h"

#include "../forms/Settings.h"
#include "SystemSettings.h"
#include "TrajectoryPoint.h"
#include "general/Macros.h"
#include "geometry/JPoint.h"
#include "geometry/LinePlotter.h"
#include "geometry/PointPlotter.h"

#include <cmath>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDiskSource.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>


vtkCamera * Pedestrian::virtualCam;

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace std;
/**
 * class Constructor
 * @param ID the pedestrian ID
 * @param posX the initial x coordinate
 * @param posY the initial y coordinate
 * @param posZ the initial z coordinate
 */
Pedestrian::Pedestrian(int ID, double posX, double posY, double posZ)
{
    this->ID      = ID;
    this->posZ    = posZ;
    this->posX    = posX;
    this->posY    = posY;
    this->pedSize = 160; // 160 cm

    this->spaceNeeded = NULL;
    // this->assembly=NULL;
    this->ellipseActor          = NULL;
    this->trailActor            = NULL;
    this->pedestrianAssembly    = NULL;
    this->assembly3D            = NULL;
    this->assembly2D            = NULL;
    this->groupVisibilityStatus = true;
    this->trailPlotterLine      = NULL;
    this->trailPlotterPoint     = NULL;
    this->caption               = NULL;
    this->bodyActor             = NULL;
    this->autoCaptionColorMode  = true;
    trailPoint.reserve(100);
}

/**
 * Parameters less constructor
 */

Pedestrian::Pedestrian(int ID)
{
    Pedestrian(ID, 0, 0, 0);
}

/**
 * class Destructor
 *
 */
Pedestrian::~Pedestrian()
{
    if(assembly2D)
        assembly2D->Delete();

    if(assembly3D)
        assembly3D->Delete();

    if(spaceNeeded)
        spaceNeeded->Delete();

    if(ellipseActor)
        ellipseActor->Delete();

    if(trailActor)
        trailActor->Delete();


    //	int i=0;
    // free all memory

    // virtualCam->Delete();
    if(bodyActor)
        bodyActor->Delete();
    if(caption)
        caption->Delete();

    while(!trailPoint.isEmpty()) {
        delete trailPoint.pop();
    }
    if(trailPlotterPoint)
        delete trailPlotterPoint;
    if(trailPlotterLine)
        delete trailPlotterLine;
}

/**
 *
 * @return the pedestrian ID
 */
int Pedestrian::getID()
{
    return ID;
}

/**
 *
 * @return the x-coordinate of the pedestrians
 */
double Pedestrian::getX()
{
    return posX;
}
/**
 *
 * @return the y-coordinate of the pedestrian
 */
double Pedestrian::getY()
{
    return posY;
}
/**
 *
 * @return the z-coordinate of the pedestrian
 */
double Pedestrian::getZ()
{
    return posZ;
}

/***
 * create the trail and  its actor
 */
void Pedestrian::createTrailActor()
{
    trailPlotterPoint = new PointPlotter();
    trailPlotterLine  = new LinePlotter();
    trailActor        = vtkAssembly::New();
    // VTK_CREATE(vtkAssembly,trailActor);
    trailActor->AddPart(trailPlotterPoint->getActor());
    trailActor->AddPart(trailPlotterLine->getActor());
}

/**
 * creates and returns a plot actor  for the actual
 * pedestrian
 *
 */
void Pedestrian::createActor()
{
    // createSnowMan();
    // return;

    if(SystemSettings::get2D()) {
        CreateActor2D();
        pedestrianAssembly = assembly2D;
    } else {
        CreateActor3D();
        pedestrianAssembly = assembly3D;
    }
}

// create the 2D assembly
void Pedestrian::CreateActor2D()
{
    // double headRadius = 10.0;
    // double ambient=0.15;
    // double diffuse=1;
    // double specular=1;

    assembly2D = vtkAssembly::New();

    {
        // private sphere
        spaceNeeded = vtkDiskSource::New();
        // spaceNeeded->SetRadialResolution(30);
        spaceNeeded->SetCircumferentialResolution(5);
        spaceNeeded->SetInnerRadius(0);
        // spaceNeeded->SetOuterRadius(bodyRadius);
        spaceNeeded->SetOuterRadius(2);
        vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(spaceNeeded->GetOutputPort());
        // spaceNeeded->Delete();
        ellipseActor = vtkActor::New();
        ellipseActor->SetMapper(mapper);
        mapper->Delete();
        // they all start with red color.
        ellipseActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
        // set the ellipse a little bit higher than the ground to
        // eliminate interaction with the floor color.
        ellipseActor->SetPosition(0, 0, 7);
        //		actor->GetProperty()->SetSpecular(specular);
        //		actor->GetProperty()->SetDiffuse(diffuse);
        //		actor->GetProperty()->SetAmbient(ambient);

        // this actor belongs to both 2D and 3D
        // assembly3D->AddPart(ellipseActor);
        assembly2D->AddPart(ellipseActor);
        // ellipseActor->Delete();

        // lookup table
        vtkLookupTable * lut = vtkLookupTable::New();
        lut->SetHueRange(0.0, 0.470);
        // lut->SetSaturationRange(0,0);
        lut->SetValueRange(1.0, 1.0);
        lut->SetNumberOfTableValues(256);
        lut->Build();
        mapper->SetLookupTable(lut);
        lut->Delete();
    }
    {
        char txt[10];
        sprintf(txt, "%d", this->ID + 1);
        caption = vtkTextActor3D ::New();
        caption->SetVisibility(false);
        caption->SetInput(txt);
        // set the properties of the caption
        // FARBE
        vtkTextProperty * tprop = caption->GetTextProperty();
        tprop->SetFontFamilyToArial();
        tprop->BoldOn();
        // tprop->ShadowOn();
        tprop->SetLineSpacing(1.0);
        tprop->SetFontSize(SystemSettings::getPedestrianCaptionSize());

        tprop->SetColor(0.0, 0.0, 0.0);
        // tprop->SetShadowOffset(2,2);

        caption->SetPosition(0, 0, 20); // 20 cm on the ground
        assembly2D->AddPart(caption);
    }
    assembly2D->SetPosition(posX, posY, posZ);
}

void Pedestrian::CreateActor3D()
{
    double bodyRadius    = 20.0;
    double headRadius    = 8.0;
    double chestThikness = 20.0 / 1.0;
    double ambient       = 0.15;
    double diffuse       = 1;
    double specular      = 1;

    assembly3D = vtkAssembly::New();
    {
        // body
        vtkSphereSource * body = vtkSphereSource::New();
        body->SetEndPhi(90.0);
        body->SetStartPhi(0);
        body->SetThetaResolution(10);
        body->SetRadius(bodyRadius);

        // create mapper
        vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(body->GetOutputPort());
        body->Delete();
        // create actor
        double er[] = {chestThikness / bodyRadius, 1, pedSize / bodyRadius};

        bodyActor = vtkActor::New();
        bodyActor->SetMapper(mapper);
        mapper->Delete();
        bodyActor->SetScale(er);
        // bodyActor->GetProperty()->SetColor(pedsColors);
        bodyActor->GetProperty()->SetColor(
            pedsColors[0] / 255.0, pedsColors[1] / 255.0, pedsColors[2] / 255.0);

        // actor->GetProperty()->SetLighting(true);
        bodyActor->GetProperty()->SetSpecular(specular);
        bodyActor->GetProperty()->SetDiffuse(diffuse);
        bodyActor->GetProperty()->SetAmbient(ambient);

        assembly3D->AddPart(bodyActor);
    }
    {
        char txt[10];
        sprintf(txt, "%d", this->ID + 1);
        caption = vtkTextActor3D ::New();
        caption->SetVisibility(false);
        caption->SetInput(txt);
        // set the properties of the caption
        // FARBE
        vtkTextProperty * tprop = caption->GetTextProperty();
        tprop->SetFontFamilyToArial();
        tprop->BoldOn();
        tprop->SetLineSpacing(1.0);
        tprop->SetFontSize(SystemSettings::getPedestrianCaptionSize());

        tprop->SetColor(0.0, 0.0, 0.0);
        caption->SetPosition(0, 0, pedSize + 20);
        assembly3D->AddPart(caption);
    }
    {
        // face
        vtkSphereSource * head = vtkSphereSource::New();
        head->SetThetaResolution(5);
        head->SetStartTheta(270);
        head->SetEndTheta(90);
        head->SetRadius(headRadius);
        // head->SetCenter(0,0,bodyRadius+2);
        head->SetCenter(0, 0, pedSize + 2);
        // create mapper
        vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(head->GetOutputPort());
        head->Delete();
        // mapper->SetColorModeToMapScalars();
        // mapper->SetScalarModeToUsePointData();

        // create actor
        vtkActor * actor = vtkActor::New();
        actor->SetMapper(mapper);
        mapper->Delete();
        actor->GetProperty()->SetColor(.90, .90, 1.0);
        actor->GetProperty()->SetSpecular(specular);
        actor->GetProperty()->SetDiffuse(diffuse);
        actor->GetProperty()->SetAmbient(ambient - .10);

        assembly3D->AddPart(actor);
        actor->Delete();
        head = vtkSphereSource::New();
        head->SetThetaResolution(5);
        head->SetStartTheta(90);
        head->SetEndTheta(270);
        head->SetRadius(headRadius);
        head->SetCenter(0, 0, pedSize + 2);
        // create mapper
        mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(head->GetOutputPort());
        head->Delete();
        // mapper->SetColorModeToMapScalars();
        // mapper->SetScalarModeToUsePointData();

        // create actor
        actor = vtkActor::New();
        actor->SetMapper(mapper);
        mapper->Delete();
        actor->GetProperty()->SetColor(0.35, 0.35, 0.35);
        actor->GetProperty()->SetSpecular(specular);
        actor->GetProperty()->SetDiffuse(diffuse);
        actor->GetProperty()->SetAmbient(ambient - .10);

        assembly3D->AddPart(actor);
        actor->Delete();
    }
    {
        // Nose
        vtkConeSource * nose = vtkConeSource::New();
        nose->SetCenter((headRadius + nose->GetHeight() / 2), 0, pedSize + 2);
        nose->SetHeight(2);
        nose->SetRadius(.5);
        vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(nose->GetOutputPort());
        nose->Delete();
        vtkActor * actor = vtkActor::New();
        actor->SetMapper(mapper);
        mapper->Delete();
        actor->GetProperty()->SetColor(1.0, 1.0, 0.0);
        actor->GetProperty()->SetSpecular(specular);
        actor->GetProperty()->SetDiffuse(diffuse);
        actor->GetProperty()->SetAmbient(ambient);

        assembly3D->AddPart(actor);
        actor->Delete();
    }
    assembly3D->RotateZ(0.0);
    assembly3D->SetPosition(posX, posY, posZ);
}

void Pedestrian::setColor(int color[3])
{
    pedsColors[0] = color[0];
    pedsColors[1] = color[1];
    pedsColors[2] = color[2];

    if(bodyActor != NULL)
        //		bodyActor->GetProperty()->SetColor(pedsColors);
        bodyActor->GetProperty()->SetColor(
            pedsColors[0] / 255.0, pedsColors[1] / 255.0, pedsColors[2] / 255.0);
}

void Pedestrian::setColor(int color)
{
    // ich klaue mal die Farbe von der Ellipse
    if(bodyActor != NULL) {
        double * col =
            ellipseActor->GetMapper()->GetLookupTable()->GetColor((double) color / 255.0);
        bodyActor->GetProperty()->SetColor(col);
    } else {
        // lookup table
        vtkLookupTable * lut = vtkLookupTable::New();
        lut->SetHueRange(0.0, 0.470);
        // lut->SetSaturationRange(0,0);
        lut->SetValueRange(1.0, 1.0);
        lut->SetNumberOfTableValues(256);
        lut->Build();

        lut->GetColor((double) color / 255.0, pedsColors);
        pedsColors[0] *= 255;
        pedsColors[1] *= 255;
        pedsColors[2] *= 255;
    }
}

/// returns the actor to the pedestrians.
/// creates one if not existing
vtkAssembly * Pedestrian::getActor()
{
    if(pedestrianAssembly == NULL)
        createActor();
    // if(pedestrianAssembly==NULL) createSnowMan();
    return pedestrianAssembly;
}
/// returns the actor to the pedestrians trail.
/// creates one if inexisting
vtkAssembly * Pedestrian::getTrailActor()
{
    if(trailActor == NULL)
        createTrailActor();
    return trailActor;
}

/**
 *
 * move the pedestrian to its new position
 */

// void Pedestrian::moveTo(TrajectoryPoint *point) {
/*
    double xNew ;//= point->getX();
    double yNew ;//= point->getY();
    double zNew ;//= point->getZ();

    double ellipse[7];
    double agentColorAngle[2];

    //point->getEllipse(ellipse);
    //point->getAgentInfo(agentColorAngle);

    // this are radius
    double ellipseColor=1;	//red

    // scaling the disk to an ellipse
    // radiusA and radiusB are given
    if( !( isnan(ellipse[3])|| isnan(ellipse[4])) ){

        //TODO: this wont be necessary when they stop explode
        // dont allow ellipse greater than 1 meter
        ellipse[3]= (ellipse[3]>100)?100:ellipse[3];
        ellipse[4]= (ellipse[4]>100)?100:ellipse[4];

        double radius = spaceNeeded->GetOuterRadius();
        double scale[3] = { ellipse[3] / radius, ellipse[4] / radius, 1 };
        ellipseActor->SetScale(scale);

        //color is not given
        if( isnan(ellipse[6])){
            double maxEllipseDiameter = 40;
            double minEllipseDiameter = 1;

            ellipseColor= (ellipse[4]+ellipse[3])/2.0;
            ellipseColor = (ellipseColor - minEllipseDiameter) / (maxEllipseDiameter -
   minEllipseDiameter); }else{ ellipseColor=ellipse[6]/255;
        }
    }

    //todo: could make this variable global to use with trails
    double *elColorRGB = ellipseActor->GetMapper()->GetLookupTable()->GetColor(ellipseColor);
    ellipseActor->GetProperty()->SetColor(elColorRGB);


    //default agent orientation
    double pedestrianOrientation = vtkMath::DegreesFromRadians(atan2((yNew - posY), (xNew
            - posX)));//-lastRotationAngle;
    if (pedestrianOrientation < 0)
        pedestrianOrientation += 360.0;

    if(isnan(agentColorAngle[1])==false){ //agent orientation
        pedestrianOrientation=agentColorAngle[1];
    }

    // this is useful when the system is paused.
    // cuz moveTo is still called, just with the same coordinates
    if ((posX != xNew) || (posY != yNew) ||  (posZ!=zNew)) {
        double ori[3];
        pedestrianAssembly->GetOrientation(ori);
        pedestrianAssembly->RotateZ(pedestrianOrientation-ori[2]);

        //adjust the caption
        if(autoCaptionColorMode){
            vtkTextProperty* tprop = caption->GetTextProperty();
            tprop->SetColor(1-elColorRGB[0],1-elColorRGB[1],1-elColorRGB[2]);
        }
    }
    //set the new position
    pedestrianAssembly->SetPosition(xNew, yNew, zNew);

    // ellipse are only available in 2D mode and the centre coordinates may be different
    // from the head\s one
    if(SystemSettings::get2D()){
        //the ellipses are not "on" the floor, but slightly over , thats why +1 is added to z;
        ellipseActor->SetPosition(ellipse[0]-xNew,ellipse[1]-yNew,ellipse[2]-zNew+2);
    }



    //adjust the camera
    if(ID==SystemSettings::getVirtualAgent()){

        static bool cam=true;
        virtualCam->SetPosition(posX+15,posY,posZ+pedSize+8);

        //new focal point
        float eyeRange=150;//15m

        //float angle=vtkMath::RadiansFromDegrees(pedestrianOrienation);
        //float  x=posX+eyeRange*cos(angle);
        //	float  y=posY+eyeRange*sin(angle);
        virtualCam->SetFocalPoint(xNew+15,yNew,zNew+pedSize+8);
        //virtualCam->SetFocalPoint(x,y,zNew+pedSize+8);
        //virtualCam->Azimuth(pedestrianOrienation);
        //virtualCam->Azimuth(pedestrianOrienation);
        //virtualCam->Yaw(pedestrianOrienation);
        virtualCam->SetDistance(eyeRange);
        //virtualCam->SetDir
        if(cam)virtualCam->SetRoll(90);
        cam=false;
        virtualCam->Modified();

    }
    // save the actual position
    // needed for computing the rotation angle if missing
    posX = xNew;
    posY = yNew;
    posZ = zNew;
    pedestrianAssembly->SetVisibility(groupVisibilityStatus);

    trailActor->SetVisibility(groupVisibilityStatus & extern_tracking_enable);
    triggerPlotTrail();
*/
//}

///**
// *
// * move the pedestrian to its new position
// */
//
// void Pedestrian::moveTo(TrajectoryPoint *point) {
//
//	double xNew = point->getX();
//	double yNew = point->getY();
//	double zNew = point->getZ();
//	double ellipse[7];
//	double agentColorAngle[2];
//
//	point->getEllipse(ellipse);
//	point->getAgentInfo(agentColorAngle);
//
//	// this are radius
//	const double maxEllipseDiameter = 40;
//	const double minEllipseDiameter = 1;
//	double ellipseColor=1;	//red
//
//
//	// scaling the disk to an ellipse
//	// radiusA and radiusB are given
//	if( !( isnan(ellipse[3])|| isnan(ellipse[4])) ){
//
//		//TODO: FIXME: this wont be necessary when they stop explode
//		ellipse[3]= (ellipse[3]>100)?100:ellipse[3];
//		ellipse[4]= (ellipse[4]>100)?100:ellipse[4];
//
//		double radius = spaceNeeded->GetOuterRadius();
//		double scale[3] = { ellipse[3] / radius, ellipse[4] / radius, 1 };
//		ellipseActor->SetScale(scale);
//
//		//color is not given
//		if( isnan(ellipse[6])){
//			ellipseColor= (ellipse[4]+ellipse[3])/2.0;
//			ellipseColor = (ellipseColor - minEllipseDiameter) / (maxEllipseDiameter -
// minEllipseDiameter); 		}else{ 			ellipseColor=ellipse[6]/255;
//		}
//	}
//
//	//todo: could make this variable global to use with trails
//	double *elColorRGB = ellipseActor->GetMapper()->GetLookupTable()->GetColor(ellipseColor);
//	ellipseActor->GetProperty()->SetColor(elColorRGB);
//
//
//	//default agent orientation
//	double defaultOrientation = vtkMath::DegreesFromRadians(atan2((yNew - posY), (xNew
//			- posX)));//-lastRotationAngle;
//	if (defaultOrientation < 0)
//		defaultOrientation += 360.0;
//
//	double pedestrianOrienation=defaultOrientation-lastPedestrianOrientation;
//	if(! isnan(agentColorAngle[1])) //agent orientation
//		pedestrianOrienation=agentColorAngle[1];
//
//	double ellipseOrientation =pedestrianOrienation;//default
//	if(!( isnan(ellipse[5]))) //ellipse orientation
//		ellipseOrientation= ellipse[5];
//
//	// this is useful when the system is paused.
//	// cuz moveTo is still called, just with the same coordinates
//	if ((posX != xNew) || (posY != yNew) ||  (posZ!=zNew)) {
//		pedestrianAssembly->RotateZ(pedestrianOrienation);
//		//the ellipse should rotate independently to the agent
//		lastPedestrianOrientation=defaultOrientation;
//		ellipseActor->RotateZ(ellipseOrientation-pedestrianOrienation-lastEllipseOrientation);
//		lastEllipseOrientation=ellipseOrientation;
//
//		//adjust the caption
//		caption->RotateZ(-pedestrianOrienation);
//		if(autoCaptionColorMode){
//			vtkTextProperty* tprop = caption->GetTextProperty();
//			tprop->SetColor(1-elColorRGB[0],1-elColorRGB[1],1-elColorRGB[2]);
//		}
//	}
//	//set the new position
//	pedestrianAssembly->SetPosition(xNew, yNew, zNew);
//
//	// ellipse are only available in 2D mode and the centre coordinates may be different
//	// from the head\s one
//	if(SystemSettings::get2D()){
//		//the ellipses are not "on" the floor, but slightly over , thats why +1 is added to z;
//		ellipseActor->SetPosition(ellipse[0]-xNew,ellipse[1]-yNew,ellipse[2]-zNew+2);
//	}
//
//
//
//	//adjust the camera
//	if(ID==SystemSettings::getVirtualAgent()){
//
//		static bool cam=true;
//		virtualCam->SetPosition(posX+15,posY,posZ+pedSize+8);
//
//		//new focal point
//		float eyeRange=150;//15m
//
//		//float angle=vtkMath::RadiansFromDegrees(pedestrianOrienation);
//		//float  x=posX+eyeRange*cos(angle);
//		//	float  y=posY+eyeRange*sin(angle);
//		virtualCam->SetFocalPoint(xNew+15,yNew,zNew+pedSize+8);
//		//virtualCam->SetFocalPoint(x,y,zNew+pedSize+8);
//		//virtualCam->Azimuth(pedestrianOrienation);
//		//virtualCam->Azimuth(pedestrianOrienation);
//		//virtualCam->Yaw(pedestrianOrienation);
//		virtualCam->SetDistance(eyeRange);
//		//virtualCam->SetDir
//		if(cam)virtualCam->SetRoll(90);
//		cam=false;
//		virtualCam->Modified();
//
//	}
//	// save the actual position
//	// needed for computing the rotation angle if missing
//	posX = xNew;
//	posY = yNew;
//	posZ = zNew;
//	pedestrianAssembly->SetVisibility(groupVisibilityStatus);
//
//	trailActor->SetVisibility(groupVisibilityStatus & extern_tracking_enable);
//	triggerPlotTrail();
//
//}

bool Pedestrian::isVisible()
{
    return pedestrianAssembly->GetVisibility();
}

void Pedestrian::setVisibility(bool status)
{
    if(pedestrianAssembly == NULL) {
        createActor();
        createTrailActor();
    }

    pedestrianAssembly->SetVisibility(status);
    trailActor->SetVisibility(status);
}

void Pedestrian::setVisualModeTo2D(bool mode)
{
    if(mode) { // 2D
        pedestrianAssembly = assembly2D;
        assembly3D->SetVisibility(false);
    } else { // 3D
        pedestrianAssembly = assembly3D;
        assembly2D->SetVisibility(false);
    }
    pedestrianAssembly->Modified();
}


/// set the group visibility status
void Pedestrian::setGroupVisibility(bool status)
{
    groupVisibilityStatus = status;
}

/// get the group visibility
bool Pedestrian::getGroupVisibility()
{
    return groupVisibilityStatus;
}

void Pedestrian::setSize(double size)
{
    this->pedSize = size;
}

void Pedestrian::enableCaption(bool status)
{
    if(caption != NULL)
        this->caption->SetVisibility(status);
}


void Pedestrian::plotTrail(double x, double y, double z)
{
    trailPoint.push(new JPoint(x, y, z));
}

void Pedestrian::setTrailGeometry(int type)
{
    switch(type) {
        case 0: // points
            // trailPlotterLine->getActor()->SetVisibility(0);
            trailPlotterPoint->getActor()->SetVisibility(1);
            break;

        case 1: // polygone
            // trailPlotterLine->getActor()->SetVisibility(1);
            trailPlotterPoint->getActor()->SetVisibility(0);

            break;
    }
}


void Pedestrian::triggerPlotTrail()
{
    if(trailPoint.isEmpty())
        return;
    if(trailPoint.size() < 2)
        return;

    // make color uniform and use only [0,255]
    // double color[3]={pedsColors[0],255*pedsColors[1],255*pedsColors[2]};

    trailPlotterLine->clear();
    trailPlotterLine->SetNumberOfPoints(trailPoint.size());

    //	Point * first = trailPoint.pop();
    //	first->setColorRGB(color[0],color[1],color[2]);
    //	trailPlotterPoint->PlotPoint(first);
    //	trailPlotterLine->addVertex(first);

    while(!trailPoint.isEmpty()) {
        JPoint * next = trailPoint.pop();
        next->setColorRGB(pedsColors[0], pedsColors[1], pedsColors[2]);

        // trailPlotterLine->PlotLine(first, next);
        trailPlotterPoint->PlotPoint(next);
        trailPlotterLine->addVertex(next);
        delete(next);
    }
    trailPoint.clear();
}

void Pedestrian::setCaptionSize(int size)
{
    vtkTextProperty * tprop = caption->GetTextProperty();
    // tprop->SetFontFamilyToArial();
    // tprop->BoldOn();
    // tprop->ShadowOn();
    // tprop->SetLineSpacing(1.0);
    // tprop->SetColor(1.0,0.0,0.0);
    tprop->SetFontSize(size);
}


void Pedestrian::setCamera(vtkCamera * cam)
{
    virtualCam = cam;
}

void Pedestrian::setCaptionsColor(QColor & col)
{
    double captionColors[3];
    captionColors[0]        = (double) col.red() / 255;
    captionColors[1]        = (double) col.green() / 255;
    captionColors[2]        = (double) col.blue() / 255;
    vtkTextProperty * tprop = caption->GetTextProperty();
    // tprop->SetFontFamilyToArial();
    // tprop->BoldOn();
    // tprop->ShadowOn();
    // tprop->SetLineSpacing(1.0);
    tprop->SetColor(captionColors);
    //	tprop->SetFontSize(size);
}

void Pedestrian::setCaptionsColorModeToAuto(bool status)
{
    autoCaptionColorMode = status;
}

void Pedestrian::setResolution(int pts)
{
    spaceNeeded->SetRadialResolution(pts);
    spaceNeeded->SetCircumferentialResolution(pts);
}
