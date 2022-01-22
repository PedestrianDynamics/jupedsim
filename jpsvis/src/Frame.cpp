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

#include "Frame.hpp"

#include <glm/vec3.hpp>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

void Frame::InsertElement(FrameElement && element)
{
    _framePoints.emplace_back(std::move(element));
}

int Frame::Size() const
{
    return _framePoints.size();
}

const std::vector<FrameElement> & Frame::GetFrameElements() const
{
    return _framePoints;
}

vtkSmartPointer<vtkPolyData> Frame::GetPolyData2D()
{
    vtkNew<vtkPoints> points;
    vtkNew<vtkFloatArray> colors;
    vtkNew<vtkFloatArray> tensors;
    vtkNew<vtkIntArray> labels;

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    labels->SetName("labels");
    labels->SetNumberOfComponents(1);

    for(unsigned int i = 0; i < _framePoints.size(); i++) {
        glm::dvec3 pos = _framePoints[i].pos;
        glm::dvec3 rad = _framePoints[i].radius;
        glm::dvec3 rot = _framePoints[i].orientation;
        double color   = _framePoints[i].color;
        labels->InsertNextValue(_framePoints[i].id + 1);

        rad[0] /= 30;
        rad[1] /= 30;
        rad[2] /= 120;
        points->InsertNextPoint(pos.x, pos.y, pos.z);
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

    vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::New();
    // setting the colors
    polydata->SetPoints(points);
    polydata->GetPointData()->AddArray(colors);
    polydata->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    polydata->GetPointData()->SetTensors(tensors);
    polydata->GetPointData()->SetActiveTensors("tensors");

    // setting the labels
    polydata->GetPointData()->AddArray(labels);
    return polydata;
}

vtkSmartPointer<vtkPolyData> Frame::GetPolyData3D()
{
    vtkNew<vtkPoints> points;
    vtkNew<vtkFloatArray> colors;
    vtkNew<vtkFloatArray> tensors;

    colors->SetName("color");
    colors->SetNumberOfComponents(1);

    tensors->SetName("tensors");
    tensors->SetNumberOfComponents(9);

    for(unsigned int i = 0; i < _framePoints.size(); i++) {
        glm::dvec3 pos = _framePoints[i].pos;
        glm::dvec3 rad = _framePoints[i].radius;
        glm::dvec3 rot = _framePoints[i].orientation;
        double color   = _framePoints[i].color;

        // values for cylindar
        double height_i   = 170;
        double max_height = 160;
        rot[0]            = vtkMath::RadiansFromDegrees(90.0);
        rot[1]            = vtkMath::RadiansFromDegrees(00.0);
        int angle_offset  = 0;
        pos[2] += height_i / 2.0 - 30; // slightly above ground
        rad[0] /= 20;
        rad[0] = 1;
        rad[2] /= 20;
        rad[2] = 1;
        rad[1] = height_i / max_height;

        points->InsertNextPoint(pos.x, pos.y, pos.z);
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
    vtkSmartPointer<vtkPolyData> polydata = vtkPolyData::New();
    polydata->SetPoints(points);
    polydata->GetPointData()->AddArray(colors);
    polydata->GetPointData()->SetActiveScalars("color");

    // setting the scaling and rotation
    polydata->GetPointData()->SetTensors(tensors);
    polydata->GetPointData()->SetActiveTensors("tensors");
    return polydata;
}
