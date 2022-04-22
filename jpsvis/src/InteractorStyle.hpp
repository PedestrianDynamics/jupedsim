/**
 * @headerfile InteractorStyle.h
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
 * @brief  Catch some mouse and keyboard events and redirect them (vtk stuff)
 *
 *
 *  Created on: Aug 18, 2009
 */

#pragma once
#include <vtkInteractorStyleTrackballCamera.h>

class Visualisation;

/// This defines the interactions  between the user and the visualisation window.
class InteractorStyle : public vtkInteractorStyleTrackballCamera
{
private:
    Visualisation* _visualisation{nullptr};

public:
    virtual ~InteractorStyle() = default;

    /// static constructor
    static InteractorStyle* New();

    void SetVisualisation(Visualisation* visualisation);

    void OnChar() override;

    void Rotate() override;

    void Spin() override;

    void Pan() override;

    void Dolly() override;

    void OnLeftButtonUp() override;

    void OnMouseMove() override;

private:
    InteractorStyle() = default;
};
