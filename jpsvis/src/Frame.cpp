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

#include "Frame.h"

#include "FrameElement.h"
#include "general/Macros.h"

#include <iostream>
#include <vector>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

Frame::Frame(int id)
{
    _elementCursor = 0;
    _id            = id;
    _polydata2D    = vtkPolyData::New();
    _polydata3D    = vtkPolyData::New();
    _framePoints.reserve(1500);
}
Frame::~Frame()
{
    while(!_framePoints.empty()) {
        delete _framePoints.back();
        _framePoints.pop_back();
    }
    _framePoints.clear();

    _polydata2D->Delete();
    _polydata3D->Delete();
}

int Frame::getSize()
{
    return _framePoints.size();
}

void Frame::addElement(FrameElement * point)
{
    _framePoints.push_back(point);
}

// void Frame::clear()
//{
//    while (!_framePoints.empty()) {
//        delete _framePoints.back();
//        _framePoints.pop_back();
//    }
//    _framePoints.clear();

//    _elementCursor=0;
//}

FrameElement * Frame::getNextElement()
{
    if(_elementCursor >= _framePoints.size()) {
        return NULL;

    } else {
        // return _framePoints.at(_elementCursor++);
        return _framePoints[_elementCursor++];
    }
    // next test
}


/*
vtkPolyData* Frame::GetPolyData(bool is_ellipse)
{
    VTK_CREATE (vtkPoints, points);
    VTK_CREATE (vtkFloatArray, colors);
    VTK_CREATE (vtkFloatArray, tensors);

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    for (unsigned int i=0;i<_framePoints.size();i++)
    {
        double pos[3]={0,0,0};
        double rad[3]={1.0,1.0,1.0};
        double rot[3];
        double color;
        _framePoints[i]->GetPos(pos); //pos[2]=90;
        _framePoints[i]->GetOrientation(rot);
        _framePoints[i]->GetColor(&color);
        _framePoints[i]->GetRadius(rad);


        //only scale the ellipse 2D
        if(is_ellipse)
        {
            rad[0]/=30;rad[1]/=30;rad[2]/=120;
        }
        else
        {
            double height_i=170;
            rot[0]=vtkMath::RadiansFromDegrees(90.0);
            rot[1]=vtkMath::RadiansFromDegrees(00.0);
            pos[2]=height_i/2.0; // slightly above ground
            rad[0]/=30;
            rad[2]/=30;
            //?height default to 30 in SaxParser and 160 in Renderingengine
            rad[1]=height_i/160.0;

        }

        points->InsertNextPoint(pos);

        rot[2]=vtkMath::RadiansFromDegrees(rot[2]);

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


        if(color==-1){
            colors->InsertNextValue(NAN);
        }
        else{
            colors->InsertNextValue(color/255.0);
        }
    }

    // setting the colors
    _polydata->SetPoints(points);
    _polydata->GetPointData()->AddArray(colors);
    _polydata->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    _polydata->GetPointData()->SetTensors(tensors);
    _polydata->GetPointData()->SetActiveTensors("tensors");


    return _polydata;
}
*/

void Frame::ComputePolyData()
{
    ComputePolyData2D();
    ComputePolyData3D();
}

void Frame::ComputePolyData2D()
{
    VTK_CREATE(vtkPoints, points);
    VTK_CREATE(vtkFloatArray, colors);
    VTK_CREATE(vtkFloatArray, tensors);
    VTK_CREATE(vtkIntArray, labels);

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    labels->SetName("labels");
    labels->SetNumberOfComponents(1);

    for(unsigned int i = 0; i < _framePoints.size(); i++) {
        double pos[3] = {0, 0, 0};
        double rad[3] = {1.0, 1.0, 1.0};
        double rot[3];
        double color;
        _framePoints[i]->GetPos(pos); // pos[2]=90;
        _framePoints[i]->GetOrientation(rot);
        _framePoints[i]->GetColor(&color);
        _framePoints[i]->GetRadius(rad);
        labels->InsertNextValue(_framePoints[i]->GetId() + 1);

        rad[0] /= 30;
        rad[1] /= 30;
        rad[2] /= 120;
        points->InsertNextPoint(pos);
        rot[2] = vtkMath::RadiansFromDegrees(rot[2]);

        // scaling matrix
        double sc[3][3] = {{rad[0], 0, 0}, {0, rad[1], 0}, {0, 0, rad[2]}};


        // rotation matrix around x-axis
        double roX[3][3] = {
            {1, 0, 0}, {0, cos(rot[0]), -sin(rot[0])}, {0, sin(rot[0]), cos(rot[0])}};

        // rotation matrix around y-axis
        double roY[3][3] = {
            {cos(rot[1]), 0, sin(rot[1])}, {0, 1, 0}, {-sin(rot[1]), 0, cos(rot[1])}};

        // rotation matrix around z-axis
        double roZ[3][3] = {
            {cos(rot[2]), sin(rot[2]), 0.0}, {-sin(rot[2]), cos(rot[2]), 0.0}, {0.0, 0.0, 1.0}};


        // final rotation matrix
        double ro[3][3];
        vtkMath::Multiply3x3(roX, roY, ro);
        vtkMath::Multiply3x3(ro, roZ, ro);

        // final transformation matrix
        double rs[3][3];
        vtkMath::Multiply3x3(sc, ro, rs);

        tensors->InsertNextTuple9(
            rs[0][0],
            rs[0][1],
            rs[0][2],
            rs[1][0],
            rs[1][1],
            rs[1][2],
            rs[2][0],
            rs[2][1],
            rs[2][2]);


        if(color == -1) {
            colors->InsertNextValue(NAN);
        } else {
            colors->InsertNextValue(color / 255.0);
        }
    }

    // setting the colors
    _polydata2D->SetPoints(points);
    _polydata2D->GetPointData()->AddArray(colors);
    _polydata2D->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    _polydata2D->GetPointData()->SetTensors(tensors);
    _polydata2D->GetPointData()->SetActiveTensors("tensors");

    // setting the labels
    _polydata2D->GetPointData()->AddArray(labels);
}

void Frame::ComputePolyData3D()
{
    VTK_CREATE(vtkPoints, points);
    VTK_CREATE(vtkFloatArray, colors);
    VTK_CREATE(vtkFloatArray, tensors);

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    for(unsigned int i = 0; i < _framePoints.size(); i++) {
        double pos[3] = {0, 0, 0};
        double rad[3] = {1.0, 1.0, 1.0};
        double rot[3];
        double color;
        _framePoints[i]->GetPos(pos); // pos[2]=90;
        _framePoints[i]->GetOrientation(rot);
        _framePoints[i]->GetColor(&color);
        _framePoints[i]->GetRadius(rad);


        // values for cylindar
        double height_i   = 170;
        double max_height = 160;
        rot[0]            = vtkMath::RadiansFromDegrees(90.0);
        rot[1]            = vtkMath::RadiansFromDegrees(00.0);
        int angle_offset  = 0;

        // values for charlie
        // rot[0]=vtkMath::RadiansFromDegrees(00.0);
        // rot[1]=vtkMath::RadiansFromDegrees(00.0);
        // int angle_offset=90;
        // double max_height=350;


        pos[2] += height_i / 2.0 - 30; // slightly above ground
        rad[0] /= 20;
        rad[0] = 1;
        rad[2] /= 20;
        rad[2] = 1;
        //?height default to 30 in SaxParser and 160 in Renderingengine
        // rad[1]=height_i/160.0;
        rad[1] = height_i / max_height;


        points->InsertNextPoint(pos);
        rot[2] = vtkMath::RadiansFromDegrees(rot[2] + angle_offset);

        // scaling matrix
        double sc[3][3] = {{rad[0], 0, 0}, {0, rad[1], 0}, {0, 0, rad[2]}};


        // rotation matrix around x-axis
        double roX[3][3] = {
            {1, 0, 0}, {0, cos(rot[0]), -sin(rot[0])}, {0, sin(rot[0]), cos(rot[0])}};

        // rotation matrix around y-axis
        double roY[3][3] = {
            {cos(rot[1]), 0, sin(rot[1])}, {0, 1, 0}, {-sin(rot[1]), 0, cos(rot[1])}};

        // rotation matrix around z-axis
        double roZ[3][3] = {
            {cos(rot[2]), sin(rot[2]), 0.0}, {-sin(rot[2]), cos(rot[2]), 0.0}, {0.0, 0.0, 1.0}};


        // final rotation matrix
        double ro[3][3];
        vtkMath::Multiply3x3(roX, roY, ro);
        vtkMath::Multiply3x3(ro, roZ, ro);

        // final transformation matrix
        double rs[3][3];
        vtkMath::Multiply3x3(sc, ro, rs);

        tensors->InsertNextTuple9(
            rs[0][0],
            rs[0][1],
            rs[0][2],
            rs[1][0],
            rs[1][1],
            rs[1][2],
            rs[2][0],
            rs[2][1],
            rs[2][2]);


        if(color == -1) {
            colors->InsertNextValue(NAN);
        } else {
            colors->InsertNextValue(color / 255.0);
        }
    }

    // setting the colors
    _polydata3D->SetPoints(points);
    _polydata3D->GetPointData()->AddArray(colors);
    _polydata3D->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    _polydata3D->GetPointData()->SetTensors(tensors);
    _polydata3D->GetPointData()->SetActiveTensors("tensors");
}


vtkPolyData * Frame::GetPolyData2D()
{
    return _polydata2D;
}

vtkPolyData * Frame::GetPolyData3D()
{
    return _polydata3D;
}

const std::vector<FrameElement *> & Frame::GetFrameElements() const
{
    return _framePoints;
}

unsigned int Frame::getElementCursor()
{
    return _elementCursor;
}

void Frame::resetCursor()
{
    _elementCursor = 0;
}
