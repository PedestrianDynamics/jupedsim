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
 *
 *  Created on: Aug 18, 2009
 */

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkCoordinate.h>


#include "SystemSettings.h"
#include "InteractorStyle.h"

#include <iostream>
#include <QStringList>

using namespace std;

#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

InteractorStyle::InteractorStyle()
{

}

InteractorStyle::~InteractorStyle()
{

}

///static constructor
InteractorStyle* InteractorStyle::New()
{
    return new InteractorStyle();
}

void InteractorStyle::SetActor()
{
}
;

void InteractorStyle::SetSource()
{
}
;

//forward the event only if not in 2d mode
void InteractorStyle::Rotate()
{
    //if(!SystemSettings::get2D())
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
    vtkRenderWindowInteractor *rwi = this->Interactor;
    vtkRenderWindow  *renderWindow = rwi->GetRenderWindow();

    int pos[2]={0,0};
    rwi->GetEventPosition(pos);
    int pos_x=pos[0];
    int pos_y=pos[1];

    VTK_CREATE(vtkCoordinate,coordinate);
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(pos_x,pos_y,0);
    double* world = coordinate->GetComputedWorldValue(renderWindow->GetRenderers()->GetFirstRenderer());
    //conversion in metre
    world[0]/=100; world[1]/=100; world[2]/=100;

    QString winName=renderWindow->GetWindowName();
    QStringList query = winName.split(" -->") ;

    if(query.size()>1)
    {
        char tmp[50];
        sprintf(tmp,"--> [ %.2f x %.2f x %.2f ]",world[0],world[1],world[2]);
        winName=query[0] +" -->"+QString(tmp);
        renderWindow->SetWindowName(winName.toStdString().c_str());
    }
    vtkInteractorStyleTrackballCamera::OnMouseMove();
}

void InteractorStyle::OnChar()
{

    vtkRenderWindowInteractor *rwi = this->Interactor;
    //this->Interactor->GetRenderWindow()->GetScreenSize();
    //rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Print(std::cout);

    char ch = rwi->GetKeyCode();


    switch (ch) {

    case '+':
    case '-':
        rwi->Render(); // render - update the screen
        break;

    //escape
    case 27:
        extern_fullscreen_enable = false;
        extern_force_system_update = true;
        break;

    case 'a': {
        double para[3];
        vtkCamera
        * cam =
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
        cam->GetPosition(para);
        //std::cout<<endl<<;
        //cam->Roll(90-cam->GetRoll());
        //cam->Yaw(0);
        //cam->Pitch(0);
        //cam->Roll(-90);
        //cam->Elevation(0);
        //cam->Azimuth(0);
        //std::cout <<"roll       [ "<< cam->GetRoll()<<" ]"<<std::endl;
        //std::cout <<"azimuth    [ "<< cam->GetRoll()<<" ]"<<std::endl;
        //std::cout <<"elevation  [ "<< cam->GetRoll()<<" ]"<<std::endl;
        //std::cout <<"roll       [ "<<para[0]<<" " <<para[1] <<" "<<para[2]<<" ]"<<std::endl;
    }
    break;

    //zoom in
    case 'n':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.05);
        break;
    //zoom out
    case 'N':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(0.95);
        break;

    //pan
    case 'b':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Roll(5.0);
        //Pan();
        break;
    case 'B':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Roll(-5.0);
        break;

    //rotate
    case 'v':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Pitch(5);
        break;
    case 'V':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Pitch(-5);
        break;

    //dolly
    case 'c':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Yaw(5);
        break;
    case 'C':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Yaw(-5);
        break;

    //Spin
    case 'x':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Elevation(5);
        rwi->GetRenderWindow()->Modified();
        rwi->Render();
        break;
    case 'X':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Elevation(-5);
        rwi->GetRenderWindow()->Modified();
        rwi->Render();
        break;

    //Spin
    case 'm':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Azimuth(5);
        rwi->GetRenderWindow()->Modified();
        break;
    case 'M':
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Azimuth(-5);
        rwi->GetRenderWindow()->Modified();
        break;


    case 'h': { // display camera settings
        double para[3];
        vtkCamera
        * cam =
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
        cam->GetPosition(para);
        std::cout << endl << endl;
        std::cout << "position       [ " << para[0] << " " << para[1] << " "
                  << para[2] << " ]" << std::endl;
        cam->GetFocalPoint(para);
        std::cout << "focal point    [ " << para[0] << " " << para[1] << " "
                  << para[2] << " ]" << std::endl;
        cam->GetClippingRange(para);
        std::cout << "clipping range [ " << para[0] << " " << para[1] << " "
                  << para[2] << " ]" << std::endl;
        cam->GetViewPlaneNormal(para);
        std::cout << "viewplan norm  [ " << para[0] << " " << para[1] << " "
                  << para[2] << " ]" << std::endl;

    }
    break;

    default: {
        std::string key = rwi->GetKeySym();
        int sensitivity=2;
        double pos[3];
        rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->GetPosition(pos);
//		printf("[%f,%f,%f]\n",pos[0],pos[1],pos[2]);


        if(key=="Up") {
            pos[1]=pos[1]-sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],1);

        } else if(key=="Down") {
            pos[1]=pos[1]+sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],1);

        } else if(key=="Left") {
            pos[0]=pos[0]+sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);

        } else if(key=="Right") {
            pos[0]=pos[0]-sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);

        } else if(key=="k") {
            pos[0]=pos[0]-sensitivity*10;
            pos[1]=pos[1]-sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);

        } else if(key=="K") {
            pos[0]=pos[0]+sensitivity*10;
            pos[1]=pos[1]+sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);

        } else if(key=="l") {
            pos[0]=pos[0]-sensitivity*10;
            pos[1]=pos[1]+sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);

        } else if(key=="L") {
            pos[0]=pos[0]+sensitivity*10;
            pos[1]=pos[1]-sensitivity*10;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(pos);;
            rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(pos[0],pos[1],10);
        }
    }
    break;

    }

    // forward events
    vtkInteractorStyleTrackballCamera::OnChar();
}

//http://vtk.1045678.n5.nabble.com/Coordinate-conversions-World-Display-td2808312.html
void InteractorStyle::OnLeftButtonUp()
{

    vtkRenderWindowInteractor *rwi = this->Interactor;

    int pos[2] = {0,0};
    rwi->GetEventPosition(pos);
    int pos_x=pos[0];
    int pos_y=pos[1];

    VTK_CREATE(vtkCoordinate,coordinate);
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(pos_x,pos_y,0);
    double* world = coordinate->GetComputedWorldValue(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    world[0]/=100;
    world[1]/=100;
    world[2]/=100;
    cout.precision(2);
    std::cout<<"mouse position: " <<endl;
    std::cout<<"\t screen: " <<pos_x<<" "<<pos_y<<endl;
    std::cout<<"\t world : " <<world[0] << " " << world[1] << " " << world[2] << std::endl;

    //vtkRenderer *ren =rwi->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    //vtkCamera *cam = ren->GetActiveCamera();
    //cam->SetFocalPoint(world[0],world[1],world[2]);
    //cam->Zoom(1.05);
    //cam->Modified();
    //double p[2]={pos[0],pos[1]};
    //rwi->FlyToImage(ren,p);
    // forward events for who ever needs it
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();

}
