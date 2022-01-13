/**
 * @headerfile Frame.h
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
 */
#pragma once

#include "FrameElement.h"

#include <vector>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

/// Represents a single frame of the simulation
/// TODO(kkratz): Split into FrameData / Frame2DModel / Frame3DModel
class Frame
{
    std::vector<FrameElement> _framePoints;

public:
    /// Constructor
    Frame() = default;

    /// Destructor
    ~Frame() = default;

    /// Insert an Element to the Frame.
    /// @param element to add
    void InsertElement(FrameElement && element);

    /// @return the number of element in this frame
    int Size() const;

    /// @return the 2D polydata set
    vtkSmartPointer<vtkPolyData> GetPolyData2D();

    /// @return the 3D polydata set
    vtkSmartPointer<vtkPolyData> GetPolyData3D();

    /// Access Elements in this Frame
    /// @preturn vector of FrameElements
    const std::vector<FrameElement> & GetFrameElements() const;
};
