/**
 * @file InteractorStyle.cpp
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
 *  Created on: Aug 18, 2009
 */

#include "InteractorStyle.hpp"

#include "Settings.hpp"
#include "Visualisation.hpp"
#include "general/Macros.hpp"

#include <QStringList>
#include <iostream>
#include <vtkCamera.h>
#include <vtkCoordinate.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>

using namespace std;

// Macro to implement ::New() for subclasses of vtkObject()
vtkStandardNewMacro(InteractorStyle);

void InteractorStyle::SetVisualisation(Visualisation* visulisation)
{
    _visualisation = visulisation;
}

// forward the event only if not in 2d mode
void InteractorStyle::Rotate()
{
    vtkInteractorStyleTrackballCamera::Rotate();
}

void InteractorStyle::Spin()
{
    vtkInteractorStyleTrackballCamera::Spin();
}

void InteractorStyle::Pan()
{
    vtkInteractorStyleTrackballCamera::Pan();
}

void InteractorStyle::Dolly()
{
    vtkInteractorStyleTrackballCamera::Dolly();
}

void InteractorStyle::OnMouseMove()
{
    if(_visualisation) {
        vtkRenderWindowInteractor* rwi = this->Interactor;
        vtkRenderWindow* renderWindow = rwi->GetRenderWindow();

        int pos[2] = {0, 0};
        rwi->GetEventPosition(pos);
        int pos_x = pos[0];
        int pos_y = pos[1];

        VTK_CREATE(vtkCoordinate, coordinate);
        coordinate->SetCoordinateSystemToDisplay();
        coordinate->SetValue(pos_x, pos_y, 0);
        double* world =
            coordinate->GetComputedWorldValue(renderWindow->GetRenderers()->GetFirstRenderer());
        // Note: coordinates are converted to meters
        _visualisation->onMouseMove(world[0] /= 100, world[1] /= 100, world[2] /= 100);
    }
    vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void InteractorStyle::OnChar()
{
    vtkRenderWindowInteractor* rwi = this->Interactor;
    char ch = rwi->GetKeyCode();

    switch(ch) {
        case '+':
        case '-':
            rwi->Render(); // render - update the screen
            break;
        case 'a': {
            double para[3];
            vtkCamera* cam =
                rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
            cam->GetPosition(para);
        } break;

        // zoom in
        case 'n':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                1.05);
            break;
        // zoom out
        case 'N':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(
                0.95);
            break;

        // pan
        case 'b':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Roll(
                5.0);
            // Pan();
            break;
        case 'B':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Roll(
                -5.0);
            break;

        // rotate
        case 'v':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Pitch(5);
            break;
        case 'V':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Pitch(
                -5);
            break;

        // dolly
        case 'c':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Yaw(5);
            break;
        case 'C':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Yaw(-5);
            break;

        // Spin
        case 'x':
            rwi->GetRenderWindow()
                ->GetRenderers()
                ->GetFirstRenderer()
                ->GetActiveCamera()
                ->Elevation(5);
            rwi->GetRenderWindow()->Modified();
            rwi->Render();
            break;
        case 'X':
            rwi->GetRenderWindow()
                ->GetRenderers()
                ->GetFirstRenderer()
                ->GetActiveCamera()
                ->Elevation(-5);
            rwi->GetRenderWindow()->Modified();
            rwi->Render();
            break;
        // Spin
        case 'm':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Azimuth(
                5);
            rwi->GetRenderWindow()->Modified();
            break;
        case 'M':
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Azimuth(
                -5);
            rwi->GetRenderWindow()->Modified();
            break;

        case 'h': { // display camera settings
            double para[3];
            vtkCamera* cam =
                rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
            cam->GetPosition(para);
            std::cout << endl << endl;
            std::cout << "position       [ " << para[0] << " " << para[1] << " " << para[2] << " ]"
                      << std::endl;
            cam->GetFocalPoint(para);
            std::cout << "focal point    [ " << para[0] << " " << para[1] << " " << para[2] << " ]"
                      << std::endl;
            cam->GetClippingRange(para);
            std::cout << "clipping range [ " << para[0] << " " << para[1] << " " << para[2] << " ]"
                      << std::endl;
            cam->GetViewPlaneNormal(para);
            std::cout << "viewplan norm  [ " << para[0] << " " << para[1] << " " << para[2] << " ]"
                      << std::endl;
            break;
        }
        default: {
            std::string key = rwi->GetKeySym();
            int sensitivity = 2;
            double pos[3];
            rwi->GetRenderWindow()
                ->GetRenderers()
                ->GetFirstRenderer()
                ->GetActiveCamera()
                ->GetPosition(pos);
            if(key == "Up") {
                pos[1] = pos[1] - sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 1);
            } else if(key == "Down") {
                pos[1] = pos[1] + sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 1);
            } else if(key == "Left") {
                pos[0] = pos[0] + sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            } else if(key == "Right") {
                pos[0] = pos[0] - sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            } else if(key == "k") {
                pos[0] = pos[0] - sensitivity * 10;
                pos[1] = pos[1] - sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            } else if(key == "K") {
                pos[0] = pos[0] + sensitivity * 10;
                pos[1] = pos[1] + sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            } else if(key == "l") {
                pos[0] = pos[0] - sensitivity * 10;
                pos[1] = pos[1] + sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            } else if(key == "L") {
                pos[0] = pos[0] + sensitivity * 10;
                pos[1] = pos[1] - sensitivity * 10;
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetPosition(pos);
                rwi->GetRenderWindow()
                    ->GetRenderers()
                    ->GetFirstRenderer()
                    ->GetActiveCamera()
                    ->SetFocalPoint(pos[0], pos[1], 10);
            }
            break;
        }
    }

    // forward events
    vtkInteractorStyleTrackballCamera::OnChar();
}

// http://vtk.1045678.n5.nabble.com/Coordinate-conversions-World-Display-td2808312.html
void InteractorStyle::OnLeftButtonUp()
{
    vtkRenderWindowInteractor* rwi = this->Interactor;

    static double last_pos[3] = {0, 0, 0};
    int pos[2] = {0, 0};
    rwi->GetEventPosition(pos);
    int pos_x = pos[0];
    int pos_y = pos[1];

    VTK_CREATE(vtkCoordinate, coordinate);
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(pos_x, pos_y, 0);
    double* world = coordinate->GetComputedWorldValue(
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

    world[0] /= 100;
    world[1] /= 100;
    world[2] /= 100;
    double dist = vtkMath::Distance2BetweenPoints(last_pos, world);
    last_pos[0] = world[0];
    last_pos[1] = world[1];
    last_pos[2] = world[2];

    cout.precision(2);
    std::cout << "mouse position: " << endl;
    std::cout << "\t screen: " << pos_x << " " << pos_y << endl;
    std::cout << "\t world : " << world[0] << " " << world[1] << " " << world[2] << std::endl;
    std::cout << "\t distance to last mouse position: " << dist << endl;
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}
