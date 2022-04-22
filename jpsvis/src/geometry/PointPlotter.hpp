/**
 * @headerfile PointPlotter.h
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
 * @brief method for plotting Plot a lot of points,<br> and create a single actor to render
 *
 *  Created on: 07.05.2009
 *
 */
#pragma once

#include <glm/vec3.hpp>
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkUnsignedCharArray;
class vtkPolyData;
class vtkActor;
class vtkDataArray;
class vtkFloatArray;
class JPoint;

class PointPlotter
{
public:
    PointPlotter();

    ~PointPlotter() = default;

    void PlotPoint(const glm::dvec3& pos, double color);

    vtkSmartPointer<vtkActor> getActor();

    void SetVisibility(bool status);

private:
    static constexpr double pt_radius{2};
    vtkSmartPointer<vtkPoints> pts;
    vtkSmartPointer<vtkFloatArray> colors;
    vtkSmartPointer<vtkActor> pointActor;
    vtkIdType nextPointID{0};
};
