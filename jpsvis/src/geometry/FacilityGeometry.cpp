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

#include "../SystemSettings.h"
#include "JPoint.h"
#include "LinePlotter2D.h"

#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAssembly.h>
#include <vtkCaptionActor2D.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionWidget.h>
#include <vtkCellData.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDataSetMapper.h>
#include <vtkDiskSource.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>
#include <vtkTriangleFilter.h>


#define PI 3.1415926535

using namespace std;

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


FacilityGeometry::FacilityGeometry(
    const string & description,
    const string & roomCaption,
    const string & subroomCaption)
{
    assembly         = vtkAssembly::New();
    assembly2D       = vtkAssembly::New();
    assemblyCaptions = vtkAssembly::New();

    assemblyWalls3D = vtkAssembly::New();
    assemblyDoors3D = vtkAssembly::New();
    assembly3D      = vtkAssembly::New();

    floorActor         = vtkActor::New();
    obstaclesActor     = vtkActor::New();
    gradientFieldActor = vtkActor::New();

    captions       = vtkActor2DCollection::New();
    linesPlotter2D = new LinePlotter2D();

    // initializing the lookup table for the colors
    // rainbow colors ranging from red to blue
    lookupTable = vtkLookupTable::New();
    lookupTable->SetTableRange(0, 255);
    // lookupTable->SetHueRange(0.0,0.566);
    // lut->SetSaturationRange(0,0);
    // lut->SetValueRange(0.0,1.0);
    lookupTable->SetNumberOfTableValues(256);
    lookupTable->Build();

    // geometry parameters all in cm
    doorThickness   = 3;
    wallThickness   = 3;
    wallHeight      = 250;
    doorHeight      = 250;
    stepHeight      = 40;
    wallColor       = 255;
    stepColor       = 130;
    doorColor       = 50;
    navlineColor    = 95;
    _description    = description;
    _roomCaption    = roomCaption;
    _subroomCaption = subroomCaption;
}

FacilityGeometry::~FacilityGeometry()
{
    lookupTable->Delete();
    captions->Delete();

    assembly->Delete();
    assembly2D->Delete();
    assemblyCaptions->Delete();

    assemblyWalls3D->Delete();
    assemblyDoors3D->Delete();
    assembly3D->Delete();
    floorActor->Delete();
    obstaclesActor->Delete();

    delete linesPlotter2D;
}

vtkAssembly * FacilityGeometry::getActor2D()
{
    return assembly2D;
}

vtkAssembly * FacilityGeometry::getCaptionsActor()
{
    return assemblyCaptions;
}

vtkAssembly * FacilityGeometry::getActor3D()
{
    return assembly3D;
}

void FacilityGeometry::CreateActors()
{
    assembly2D->AddPart(linesPlotter2D->createAssembly());
    assembly2D->AddPart(assemblyCaptions);

    assembly3D->AddPart(assemblyDoors3D);
    assembly3D->AddPart(assemblyWalls3D);
    assembly3D->AddPart(assemblyCaptions);
}

void FacilityGeometry::setVisibility(bool status)
{
    if(SystemSettings::get2D()) {
        assembly2D->SetVisibility(status);
    } else {
        assembly3D->SetVisibility(status);
    }
    _visibility = status;
}

bool FacilityGeometry::getVisibility() const
{
    return _visibility;
}
/***
 * This is the main build method and should be called by all functions
 * drawing a wall or a door. Important
 */
void FacilityGeometry::addNewElement(
    double center[3],
    double length,
    double orientation,
    ELEMENT_TYPE type)
{
    vtkCubeSource * src = vtkCubeSource::New();
    src->SetCenter(center[0], center[1], center[2]);
    src->SetYLength(length);

    vtkPolyDataMapper * mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(src->GetOutputPort());
    src->Delete();

    vtkActor * actor = vtkActor::New();
    actor->SetMapper(mapper);
    mapper->Delete();
    actor->GetProperty()->SetLighting(true);
    actor->SetOrigin(center[0], center[1], center[2]);
    actor->RotateZ(orientation);
    actor->GetProperty()->SetAmbient(0.2);
    actor->GetProperty()->SetDiffuse(0.8);

    //  double data[3];
    //  actor->GetPosition(data);
    //  actor->SetPosition(data[0],data[1],src->GetZLength()/2);

    switch(type) {
        case DOOR: {
            double colorRGB[3];
            lookupTable->GetColor(doorColor, colorRGB);
            actor->GetProperty()->SetColor(colorRGB);
            src->SetXLength(doorThickness);
            src->SetZLength(doorHeight);
            // src->SetRadius(doorWidth);
            actor->GetProperty()->SetOpacity(0.5);
            assemblyDoors3D->AddPart(actor);
        } break;
        case WALL: {
            double colorRGB[3];
            lookupTable->GetColor(wallColor, colorRGB);
            actor->GetProperty()->SetColor(colorRGB);
            // actor->GetProperty()->SetSpecular(1);
            // actor->GetProperty()->SetDiffuse(1);
            // actor->GetProperty()->SetAmbient(1);

            src->SetXLength(wallThickness);
            src->SetZLength(wallHeight);
            // src->SetRadius(wallWidth);
            assemblyWalls3D->AddPart(actor);
        } break;
        case STEP: {
            double colorRGB[3];
            lookupTable->GetColor(stepColor, colorRGB);
            actor->GetProperty()->SetColor(colorRGB);
            src->SetXLength(wallThickness); // FIXME, this is wrong
            src->SetZLength(stepHeight);
            assemblyDoors3D->AddPart(actor);
        } break;

            // default behaviour not defined
        default:
            break;
    }

    // now adjusting the z coordinates
    double data[3];
    actor->GetPosition(data);
    actor->SetPosition(data[0], data[1], src->GetZLength() / 2);
    actor->Delete();
}

void FacilityGeometry::addWall(
    double x1,
    double y1,
    double z1,
    double x2,
    double y2,
    double z2,
    double thickness,
    double height,
    double color)
{
    // all walls will have this parameters until changed
    wallColor = color;

    //	if(SystemSettings::get2D()){
    double m[] = {x1, y1, z1};
    double n[] = {x2, y2, z2};
    linesPlotter2D->PlotWall(m, n, wallColor / 255.0);


    JPoint * p1 = new JPoint(x1, y1, z1);
    JPoint * p2 = new JPoint(x2, y2, z2);
    JPoint p3   = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;

    addNewElement(centre, length, angle, WALL);

    delete p1;
    delete p2;
}

void FacilityGeometry::addStair(
    double x1,
    double y1,
    double z1,
    double x2,
    double y2,
    double z2,
    double thickness,
    double height,
    double color)
{
    // all walls will have this parameters until changed
    wallColor = color;

    //	if(SystemSettings::get2D()){
    double m[] = {x1, y1, z1};
    double n[] = {x2, y2, z2};
    linesPlotter2D->PlotWall(m, n, wallColor / 255.0);


    //    JPoint *p1 = new JPoint(x1,y1,z1);
    //    JPoint *p2 = new JPoint(x2,y2,z2);
    //    double *center = p1->centreCoordinatesWith(*p2);
    //    double angle =p1->angleMadeWith(*p2);
    //    double length =p1->distanceTo(*p2)+wallThickness;

    //    addNewElement(center, length, angle, WALL);

    //    delete p1;
    //    delete p2;
    //    delete center;
}

void FacilityGeometry::addDoor(
    double x1,
    double y1,
    double z1,
    double x2,
    double y2,
    double z2,
    double thickness,
    double height,
    double color)
{
    // all doors will take this color upon changed
    doorColor = color;
    // constructing the 2D assembly
    //	if(SystemSettings::get2D()){
    double m[] = {x1, y1, z1};
    double n[] = {x2, y2, z2};

    linesPlotter2D->PlotDoor(m, n, doorColor / 255.0);

    JPoint * p1 = new JPoint(x1, y1, z1);
    JPoint * p2 = new JPoint(x2, y2, z2);
    JPoint p3   = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;

    addNewElement(centre, length, angle, DOOR);

    delete p1;
    delete p2;
}

void FacilityGeometry::addNavLine(
    double x1,
    double y1,
    double z1,
    double x2,
    double y2,
    double z2,
    double thickness,
    double height,
    double color)
{
    // all doors will take this color upon changed
    navlineColor = color;
    // constructing the 2D assembly
    //	if(SystemSettings::get2D()){
    double m[] = {x1, y1, z1};
    double n[] = {x2, y2, z2};

    linesPlotter2D->PlotNavLine(m, n, navlineColor / 255.0);

    //    JPoint *p1 = new JPoint(x1,y1,z1);
    //    JPoint *p2 = new JPoint(x2,y2,z2);
    //    double *center = p1->centreCoordinatesWith(*p2);
    //    double angle =p1->angleMadeWith(*p2);
    //    double length =p1->distanceTo(*p2)+wallThickness;

    //    addNewElement(center, length, angle, DOOR);

    //    delete p1;
    //    delete p2;
    //    delete center;
}

void FacilityGeometry::addStep(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double m[] = {x1, y1, z1};
    double n[] = {x2, y2, z2};
    linesPlotter2D->PlotDoor(m, n, doorColor / 255.0);

    JPoint * p1 = new JPoint(x1, y1, z1);
    JPoint * p2 = new JPoint(x2, y2, z2);
    JPoint p3   = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;

    addNewElement(centre, length, angle, STEP);

    delete p1;
    delete p2;
}

void FacilityGeometry::addStep(JPoint * p1, JPoint * p2)
{
    double m[3];
    double n[3];
    double CHT[3];

    p1->getXYZ(m);
    p2->getXYZ(n);
    p1->getColorHeightThicknes(CHT);

    stepHeight = CHT[1];
    stepColor  = CHT[0];

    linesPlotter2D->PlotDoor(m, n, doorColor / 255.0);

    JPoint p3 = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;
    addNewElement(centre, length, angle, STEP);
}

void FacilityGeometry::addWall(JPoint * p1, JPoint * p2, string caption)
{
    double m[3];
    double n[3];
    double CHT[3];
    p1->getXYZ(m);
    p2->getXYZ(n);
    p1->getColorHeightThicknes(CHT);

    wallThickness = CHT[2];
    wallHeight    = CHT[1];
    wallColor     = CHT[0];
    linesPlotter2D->PlotWall(m, n, wallColor / 255.0);

    if(caption.compare("") != 0) {
        double center[3];
        center[0]             = 0.5 * (m[0] + n[0]);
        center[1]             = 0.5 * (m[1] + n[1]);
        center[2]             = 0.5 * (m[2] + n[2]);
        double orientation[3] = {0, 0, 0};
        addNewElementText(center, orientation, caption.c_str(), 50);
    }

    JPoint p3 = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;
    addNewElement(centre, length, angle, WALL);
}

void FacilityGeometry::addStair(JPoint * p1, JPoint * p2, string caption)
{
    double m[3];
    double n[3];
    double CHT[3];
    p1->getXYZ(m);
    p2->getXYZ(n);
    p1->getColorHeightThicknes(CHT);

    wallThickness = CHT[2];
    wallHeight    = CHT[1];
    wallColor     = CHT[0];
    linesPlotter2D->PlotWall(m, n, wallColor / 255.0);

    if(caption.compare("") != 0) {
        double center[3];
        center[0]             = 0.5 * (m[0] + n[0]);
        center[1]             = 0.5 * (m[1] + n[1]);
        center[2]             = 0.5 * (m[2] + n[2]);
        double orientation[3] = {0, 0, 0};
        addNewElementText(center, orientation, caption.c_str(), 50);
    }

    //    double *center = p1->centreCoordinatesWith(*p2);
    //    double angle =p1->angleMadeWith(*p2);
    //    double length =p1->distanceTo(*p2)+wallThickness;
    //    addNewElement( center,  length, angle,  WALL);
}

void FacilityGeometry::addDoor(JPoint * p1, JPoint * p2, string caption)
{
    double m[3];
    double n[3];
    double CHT[3];

    p1->getXYZ(m);
    p2->getXYZ(n);
    // to get the exits over the walls
    // m[0]++;	m[1]++;	m[2]++;
    // n[0]++;	n[1]++;	n[2]++;
    p1->getColorHeightThicknes(CHT);

    doorThickness = CHT[2];
    doorHeight    = CHT[1];
    doorColor     = CHT[0];

    linesPlotter2D->PlotDoor(m, n, doorColor / 255.0);

    if(caption.compare("") != 0) {
        double center[3];
        center[0]             = 0.5 * (m[0] + n[0]);
        center[1]             = 0.5 * (m[1] + n[1]);
        center[2]             = 0.5 * (m[2] + n[2]);
        double orientation[3] = {0, 0, 0};
        addNewElementText(center, orientation, caption.c_str(), 0);
    }

    JPoint p3 = p1->centreCoordinatesWith(*p2);
    double centre[3];
    p3.getXYZ(centre);
    double angle  = p1->angleMadeWith(*p2);
    double length = p1->distanceTo(*p2) + wallThickness;
    addNewElement(centre, length, angle, DOOR);
}

void FacilityGeometry::addNavLine(JPoint * p1, JPoint * p2, string caption)
{
    double m[3];
    double n[3];
    double CHT[3];

    p1->getXYZ(m);
    p2->getXYZ(n);
    // to get the exits over the walls
    // m[0]++;	m[1]++;	m[2]++;
    // n[0]++;	n[1]++;	n[2]++;
    p1->getColorHeightThicknes(CHT);

    doorThickness = CHT[2];
    doorHeight    = CHT[1];
    doorColor     = CHT[0];

    linesPlotter2D->PlotNavLine(m, n, doorColor / 255.0);

    if(caption.compare("") != 0) {
        double center[3];
        center[0]             = 0.5 * (m[0] + n[0]);
        center[1]             = 0.5 * (m[1] + n[1]);
        center[2]             = 0.5 * (m[2] + n[2]);
        double orientation[3] = {0, 0, 0};
        addNewElementText(center, orientation, caption.c_str(), 0);
    }
}

void FacilityGeometry::addFloor(vtkPolyData * polygonPolyData)
{
    // triagulate everything
    // Create a mapper and actor
    VTK_CREATE(vtkTriangleFilter, filter);
    VTK_CREATE(vtkPolyDataMapper, mapper);

    filter->SetInputData(polygonPolyData);
    mapper->SetInputConnection(filter->GetOutputPort());

    floorActor->SetMapper(mapper);
    floorActor->GetProperty()->SetColor(0, 0, 1);
    floorActor->GetProperty()->SetOpacity(0.4);

    assembly2D->AddPart(floorActor);
    assembly3D->AddPart(floorActor);
}

void FacilityGeometry::addGradientField(vtkActor * gradientField)
{
    gradientFieldActor = gradientField;
    assembly2D->AddPart(gradientFieldActor);
    assembly3D->AddPart(gradientFieldActor);
}

void FacilityGeometry::addObstacles(vtkPolyData * polygonPolyData)
{
    // triagulate everything
    // Create a mapper and actor
    VTK_CREATE(vtkTriangleFilter, filter);
    VTK_CREATE(vtkPolyDataMapper, mapper);

    filter->SetInputData(polygonPolyData);
    mapper->SetInputConnection(filter->GetOutputPort());


    obstaclesActor->SetMapper(mapper);
    obstaclesActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
    obstaclesActor->GetProperty()->SetOpacity(0.5);

    assembly2D->AddPart(obstaclesActor);
    assembly3D->AddPart(obstaclesActor);
}


void FacilityGeometry::addRectangle(
    double x1,
    double y1,
    double x2,
    double y2,
    double z,
    double color1,
    double color2,
    string text)
{
    // if(z!=1)return;
    const double cellSize = 10; // cm
    //	const int dimX=(x2-x1)/cellSize+1;
    //	const int dimY=(y2-y1)/cellSize+1;

    const int dimX = (int) ceil((x2 - x1) / cellSize) + 1;
    const int dimY = (int) ceil((y2 - y1) / cellSize) + 1;


    const int dimZ = 1;
    // vtkDoubleArray *scalars = vtkDoubleArray::New();
    vtkDataArray * pData = vtkUnsignedCharArray::New();
    pData->SetNumberOfComponents(3);
    double color[2][3] = {{color1, color1, color1}, {color2, color2, color2}};
    bool idx           = 0;
    bool lastColorUsed = 0;
    for(int i = 0; i < dimY - 1; i++) {
        if(idx == lastColorUsed) {
            lastColorUsed = !lastColorUsed;
            idx           = lastColorUsed;
        } else {
            lastColorUsed = idx;
        }

        for(int j = 0; j < dimX - 1; j++) {
            pData->InsertNextTuple3(color[idx][0], color[idx][1], color[idx][2]);
            idx = !idx;
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

    // map->SetLookupTable(lookupTable);
    imageActor->SetMapper(map);
    if(color2 == 90) // quick and dirty --> goal
        imageActor->GetProperty()->SetAmbient(1.5);
    else
        imageActor->GetProperty()->SetAmbient(0.6);
    // imageActor->GetProperty()->SetDiffuse(0.8);

    // move the actor in x-direction
    imageActor->SetPosition(x1, y1, z);
    assembly2D->AddPart(imageActor);
    double center[3] = {x1 / 2 + x2 / 2, y1 / 2 + y2 / 2, 0};
    addNewElementText(center, 0, text, 0);
}


void FacilityGeometry::addFloor(double x1, double y1, double x2, double y2, double z)
{
    // if(z!=1)return;
    const double cellSize = 40; // cm
    //	const int dimX=(x2-x1)/cellSize+1;
    //	const int dimY=(y2-y1)/cellSize+1;
    const int dimX = (int) ceil((x2 - x1) / cellSize) + 1;
    const int dimY = (int) ceil((y2 - y1) / cellSize) + 1;


    const int dimZ = 1;
    // vtkDoubleArray *scalars = vtkDoubleArray::New();
    vtkDataArray * pData = vtkUnsignedCharArray::New();
    pData->SetNumberOfComponents(3);

    double color[2][3] = {{100, 100, 100}, {150, 150, 150}};
    bool idx           = 0;
    bool lastColorUsed = 0;
    for(int i = 0; i < dimY - 1; i++) {
        if(idx == lastColorUsed) {
            lastColorUsed = !lastColorUsed;
            idx           = lastColorUsed;
        } else {
            lastColorUsed = idx;
        }

        for(int j = 0; j < dimX - 1; j++) {
            pData->InsertNextTuple3(color[idx][0], color[idx][1], color[idx][2]);
            idx = !idx;
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

    // map->SetLookupTable(lookupTable);
    imageActor->SetMapper(map);
    imageActor->GetProperty()->SetAmbient(0.2);
    // imageActor->GetProperty()->SetDiffuse(0.8);

    // move the actor in x-direction
    imageActor->SetPosition(x1, y1, z);
    assembly2D->AddPart(imageActor);
}


void FacilityGeometry::addObjectSphere(double center[3], double radius, double color)
{
    double colorRGB[3];
    lookupTable->GetColor(color, colorRGB);

    // create a disk for the 2D world
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

        actor->SetPosition(center[0], center[1], center[2]);
        assembly2D->AddPart(actor);
    }
}


void FacilityGeometry::addObjectCylinder(
    double center[3],
    double radius,
    double height,
    double orientation[3],
    double color)
{
    double colorRGB[3];
    lookupTable->GetColor(color, colorRGB);

    // create a disk for the 2D world
    // TODO: this is of course a wrong projection
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

        actor->SetPosition(center[0], center[1], center[2]);
        assembly2D->AddPart(actor);
    }
}

void FacilityGeometry::addObjectBox(
    double center[3],
    double height,
    double width,
    double length,
    double color)
{
    double colorRGB[3];
    lookupTable->GetColor(color, colorRGB);

    {
        VTK_CREATE(vtkCubeSource, src);
        src->SetCenter(center[0], center[1], center[2]);
        src->SetZLength(1); // todo: fake projection
        src->SetYLength(length);
        src->SetXLength(width);

        VTK_CREATE(vtkPolyDataMapper, mapper);
        mapper->SetInputConnection(src->GetOutputPort());

        VTK_CREATE(vtkActor, actor);
        actor->GetProperty()->SetLighting(true);
        actor->SetOrigin(center[0], center[1], 0);
        actor->GetProperty()->SetColor(colorRGB);
        actor->GetProperty()->SetAmbient(0.2);
        actor->GetProperty()->SetDiffuse(0.8);
        actor->SetMapper(mapper);
        assembly2D->AddPart(actor);
    }
}

void FacilityGeometry::changeWallsColor(double * color)
{
    // 2D part
    linesPlotter2D->changeWallsColor(color);
    assembly2D->Modified();

    // 3D parts
    vtkProp3DCollection * col = assemblyWalls3D->GetParts();
    assemblyWalls3D->GetActors(col);

    int count = col->GetNumberOfItems();
    for(int i = 0; i < count; i++) {
        ((vtkActor *) col->GetItemAsObject(i))->GetProperty()->SetColor(color);
    }
    assemblyWalls3D->Modified();
}

void FacilityGeometry::changeExitsColor(double * color)
{
    // 2D part
    linesPlotter2D->changeDoorsColor(color);
    assembly2D->Modified();

    // 3D part
    vtkProp3DCollection * col = assemblyDoors3D->GetParts();
    assemblyDoors3D->GetActors(col);

    int count = col->GetNumberOfItems();
    for(int i = 0; i < count; i++) {
        ((vtkActor *) col->GetItemAsObject(i))->GetProperty()->SetColor(color);
    }
    assemblyDoors3D->Modified();
}

void FacilityGeometry::changeNavLinesColor(double * color)
{
    // 2D part
    linesPlotter2D->changeNavLinesColor(color);
    assembly2D->Modified();
}

void FacilityGeometry::changeFloorColor(double * color)
{
    floorActor->GetProperty()->SetColor(color);
}

void FacilityGeometry::changeObstaclesColor(double * color)
{
    obstaclesActor->GetProperty()->SetColor(color);
}

void FacilityGeometry::set2D(bool status)
{
    assembly2D->SetVisibility(status);
}

void FacilityGeometry::set3D(bool status)
{
    if(assembly3D != NULL)
        assembly3D->SetVisibility(status);
}

void FacilityGeometry::showDoors(bool status)
{
    linesPlotter2D->showDoors(status);
    assembly2D->Modified();

    vtkProp3DCollection * col = assemblyDoors3D->GetParts();
    assemblyDoors3D->GetActors(col);

    int count = col->GetNumberOfItems();
    for(int i = 0; i < count; i++) {
        ((vtkActor *) col->GetItemAsObject(i))->SetVisibility(status);
    }
    assemblyDoors3D->Modified();
}

void FacilityGeometry::showStairs(bool status) {}

void FacilityGeometry::showWalls(bool status)
{
    linesPlotter2D->showWalls(status);
    assembly2D->Modified();

    vtkProp3DCollection * col = assemblyWalls3D->GetParts();
    assemblyWalls3D->GetActors(col);

    int count = col->GetNumberOfItems();
    for(int i = 0; i < count; i++) {
        ((vtkActor *) col->GetItemAsObject(i))->SetVisibility(status);
    }
    assemblyWalls3D->Modified();
}

void FacilityGeometry::showNavLines(bool status)
{
    linesPlotter2D->showNavLines(status);
}

void FacilityGeometry::showFloor(bool status)
{
    floorActor->SetVisibility(status);
}
void FacilityGeometry::showGradientField(bool status)
{
    gradientFieldActor->SetVisibility(status);
}

void FacilityGeometry::showObstacles(bool status)
{
    obstaclesActor->SetVisibility(status);
}
void FacilityGeometry::addObjectLabel(
    double center[3],
    double orientation[3],
    std::string caption,
    double color)
{
    addNewElementText(center, orientation, caption, color);
}

vtkActor2DCollection * FacilityGeometry::getCaptions()
{
    return captions;
}


// orientation and color ignored
void FacilityGeometry::addNewElementText(
    double center[3],
    double orientation[3],
    string text,
    double color)
{
    // return ;

    // caption
    VTK_CREATE(vtkTextActor3D, caption);
    // caption = vtkTextActor3D ::New();

    // caption->SetVisibility(false);
    caption->SetInput(text.c_str());
    // set the properties of the caption
    // FARBE
    vtkTextProperty * tprop = caption->GetTextProperty();
    // tprop->SetFontFamilyToArial();
    // tprop->BoldOn();
    // tprop->ShadowOn();
    // tprop->SetLineSpacing(1.0);
    tprop->SetFontSize(SystemSettings::getPedestrianCaptionSize() / 2);

    double colorRGB[3];
    lookupTable->GetColor(color, colorRGB);
    tprop->SetColor(colorRGB);

    caption->SetPosition(center);
    assemblyCaptions->AddPart(caption);
}

void FacilityGeometry::showGeometryLabels(int status)
{
    vtkProp3DCollection * col = assemblyCaptions->GetParts();
    assemblyCaptions->GetActors(col);

    for(int i = 0; i < col->GetNumberOfItems(); i++) {
        ((vtkActor *) col->GetItemAsObject(i))->SetVisibility(status);
    }
    assemblyCaptions->Modified();
}

const std::string & FacilityGeometry::GetDescription() const
{
    return _description;
}

const std::string & FacilityGeometry::GetSubRoomCaption() const
{
    return _subroomCaption;
}

const std::string & FacilityGeometry::GetRoomCaption() const
{
    return _roomCaption;
}

void FacilityGeometry::SetRoomCaption(std::string s)
{
    _roomCaption = s;
}

void FacilityGeometry::SetSubRoomCaption(std::string s)
{
    _subroomCaption = s;
}
