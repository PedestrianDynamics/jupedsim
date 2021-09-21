/**
* @file SimpleVisualisationWindow.cpp
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
* @brief Testing class
*
*
*  Created on: 23.07.2009
*
*/


#include <vtkActor.h>
#include <vtkLight.h>
#include <vtkAssembly.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include "SimpleVisualisationWindow.h"
#include "./geometry/LinePlotter.h"
#include "./geometry/PointPlotter.h"

SimpleVisualisationWindow::SimpleVisualisationWindow()
{
    assembly =NULL;
    //actor=NULL;

}

SimpleVisualisationWindow::~SimpleVisualisationWindow()
{

}


void SimpleVisualisationWindow::setActor(vtkAssembly* actor)
{
    assembly=actor;
}


void SimpleVisualisationWindow::start()
{


    // Create the renderer with black bg
    vtkRenderer* renderer = vtkRenderer::New();
    renderer->SetBackground(0,0,0);

    //	//add the line actor
    //	LinePlotter * ln = new LinePlotter();
    //	ln->PlotPoint(324,34,34);
    //	ln->AddPoint(1,1,1);
    //	ln->AddPoint(11,11,171);
    //	ln->AddPoint(34,334,334);
    //	ln->AddPoint(81,821,81);
    //	ln->AddPoint(134,14,134);

    //	ln->PlotLine(1,1,1,324,34,34,1);
    //	ln->PlotLine(11,11,171,34,334,334,1);
    //	ln->PlotLine(81,821,81,14,134,134,1);
    //	renderer->AddActor(ln->getActor());

    //
    //	PointPlotter * pt = new PointPlotter();
    //	pt->PlotPoint(324,34,34);
    //	pt->PlotPoint(1,1,1);
    //	pt->PlotPoint(11,11,171);
    //	pt->PlotPoint(34,334,334);
    //	pt->PlotPoint(81,821,81);
    //	pt->PlotPoint(134,14,134);
    //	renderer->AddActor(pt->getActor());


    //add the different actors
    renderer->AddActor(assembly);
    //renderer->AddActor(actor);


    // Set up the lighting.
    vtkLight *light = vtkLight::New();
    light->SetFocalPoint(1.875,0.6125,0);
    light->SetPosition(3000,3000,10);
    light->SetIntensity(1);
    light->SetLightTypeToCameraLight();
    //	renderer->AddLight(light);
    //	light->Delete();

    // Create a render window
    vtkRenderWindow *renderWindow = vtkRenderWindow::New();
    renderWindow->AddRenderer( renderer );
    renderWindow->SetSize(600, 800);

    //CAUTION: this is necessary for windows to update the window name after the first render
    // but this will freeze your system on linux, strange....
#ifdef WIN32
    renderWindow->Render();
#endif
    renderWindow->SetWindowName("Visualizing a Geometry");


    // Create an interactor
    vtkRenderWindowInteractor *renderWinInteractor = vtkRenderWindowInteractor::New();
    // renderWinInteractor = vtkRenderWindowInteractor::New();
    renderWindow->SetInteractor( renderWinInteractor );
    //set full screen and catch esc to restore defaults one
    //renderWindow->SetFullScreen(1);

    // Create my interactor style
    vtkInteractorStyleTrackballCamera* style = vtkInteractorStyleTrackballCamera::New();
    renderWinInteractor->SetInteractorStyle( style );
    style->Delete();

    // Initialize and enter interactive mode
    renderWinInteractor->Initialize();
    renderWinInteractor->Start();

    //cleaning up
    renderWindow->Delete();
    renderWinInteractor->Delete();
    if(assembly)
        assembly->Delete();
    //	if(actor)
    //		actor->Delete();

}
