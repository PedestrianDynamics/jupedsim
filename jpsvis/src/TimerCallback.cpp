/**
 * @file TimerCallback.cpp
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
 * This class is called by the timer and update all elements on the screen
 *
 * \brief Callback class for the visualisation thread. Triggered by the timer
 *
 *
 *
 *  Created on: 11.05.2009
 *
 */


#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdio.h>

#include "general/Macros.h"

#include <QObject>
#include <QString>
#include <QTime>
#include <QDir>
#include <qwaitcondition.h>

#include <vtkCommand.h>
#include <vtkPolyData.h>
#include <vtkWindowToImageFilter.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPNGWriter.h>
#include <vtkPostScriptWriter.h>
#include <vtkActor2DCollection.h>
#include <vtkTextActor.h>
#include <vtkCamera.h>
#include <vtkTextProperty.h>
#include <vtkSphereSource.h>
#include <vtkMapper2D.h>
#include <vtkMapper.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkLabeledDataMapper.h>
#include <vtkMath.h>

#include <vtkPolyLine.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkLineSource.h>
#include <vtkVectorText.h>
#include <vtkFollower.h>
#include <vtkLine.h>
#include <vtkTubeFilter.h>

#include "geometry/FacilityGeometry.h"
#include "geometry/Point.h"

#include "Pedestrian.h"
#include "Frame.h"
#include "FrameElement.h"
#include "TrajectoryPoint.h"
#include "SyncData.h"
#include "SystemSettings.h"
#include "TrailPlotter.h"
#include "geometry/PointPlotter.h"
#include "TimerCallback.h"
#include <vtkTextActor3D.h>


#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace std;

static int once=1;


TimerCallback* TimerCallback::New()
{
    TimerCallback *cb = new TimerCallback;
    cb->RenderTimerId = 0;
    cb->windowToImageFilter=NULL;
    cb->runningTime=NULL;
    return cb;
}


void TimerCallback::Execute(vtkObject *caller, unsigned long eventId,
                            void *callData)
{
    if (vtkCommand::TimerEvent != eventId) {
        return;
    }

    const int tid = * static_cast<int *>(callData);
    if (tid != this->RenderTimerId) {
        return;
    }
    

    vtkRenderWindowInteractor * const iren = vtkRenderWindowInteractor::SafeDownCast(caller);
    vtkRenderWindow  * const renderWindow = iren->GetRenderWindow();
    vtkRenderer * const renderer =renderWindow->GetRenderers()->GetFirstRenderer();

    int  frameNumber=0;
    int minFrame=0;
    int nPeds=0;
    static bool isRecording =false;
    
    //first pedestrian group
    if(extern_first_dataset_loaded) {
        Frame * frame=NULL;

        // return the same frame if the system is paused
        // in fact you could just return, but in this case no update will be made
        // e.g showing captions/trails...

        if(extern_is_pause)
        {
            frame=extern_trajectories_firstSet.getFrame(extern_trajectories_firstSet.getFrameCursor());
        }
        else
        {
            frame = extern_trajectories_firstSet.getNextFrame();
        }


        frameNumber=extern_trajectories_firstSet.getFrameCursor();


        double now = frameNumber*iren->GetTimerDuration(tid)/1000;
        int countTrains  = 0;
        char label[100];

        for (auto tab: extern_trainTimeTables)
            {
                 VTK_CREATE(vtkTextActor3D, textActor);
                 auto trainType = tab.second->type;
                 sprintf(label, "%s_%d", trainType.c_str(), tab.second->id);
                 auto trainId = tab.second->id;
                 auto trackStart = tab.second->pstart;
                 auto trackEnd = tab.second->pend;
                 auto trainOffset = tab.second->train_offset;
                 auto reversed = tab.second->reversed;
                 auto train = extern_trainTypes[trainType];
                 auto train_length = train->_length;
                 auto doors = train->_doors;
                 std::vector<Point> doorPoints;
                 auto mapper = tab.second->mapper;
                 auto actor = tab.second->actor;
                 double elevation = tab.second->elevation;
                 auto txtActor = tab.second->textActor;
                 auto trackDirection = (reversed)?(trackStart - trackEnd):(trackEnd - trackStart);
                 trackDirection = trackDirection.Normalized();
                 auto trainStart = (reversed)?trackEnd + trackDirection*trainOffset:trackStart + trackDirection*trainOffset;
                 auto trainEnd = (reversed)?trackEnd + trackDirection*(trainOffset+train_length):trackStart + trackDirection*(trainOffset+train_length);
                 
                 for(auto door: doors)
                 {
                       Point trainDirection =  trainEnd - trainStart;;                                   
                       trainDirection = trainDirection.Normalized();
                       Point point1 = trainStart + trainDirection*(door._distance);
                       Point point2 = trainStart + trainDirection*(door._distance+door._width);
                      doorPoints.push_back(point1);
                      doorPoints.push_back(point2);
                 }//doors
                 if(once)
                 {
                       auto data = getTrainData(trainStart, trainEnd, doorPoints, elevation);
                      mapper->SetInputData(data);
                      actor->SetMapper(mapper);
                      actor->GetProperty()->SetLineWidth(10);
                      actor->GetProperty()->SetOpacity(0.1);//feels cool!
                      if(trainType == "RE")
                      {
                            actor->GetProperty()->SetColor(
                                  std::abs(0.0-renderer->GetBackground()[0]),
                                  std::abs(1.0-renderer->GetBackground()[1]),
                                  std::abs(1.0-renderer->GetBackground()[2])
                                  );
                      }                              
                      else
                      {
                            actor->GetProperty()->SetColor(
                                  std::abs(0.9-renderer->GetBackground()[0]),
                                  std::abs(0.9-renderer->GetBackground()[1]),
                                  std::abs(1.0-renderer->GetBackground()[2])
                                  );
                      }
                      // text
                      txtActor->GetTextProperty()->SetOpacity(0.7);
                      double pos_x = 50*(trainStart._x + trainEnd._x+0.5);
                      double pos_y = 50*(trainStart._y + trainEnd._y+0.5);

                      txtActor->SetPosition (pos_x, pos_y+2, 20);
                      txtActor->SetInput (label);
                      txtActor->GetTextProperty()->SetFontSize (30);
                      txtActor->GetTextProperty()->SetBold (true);
                      if(trainType == "RE")
                      {
                            txtActor->GetTextProperty()->SetColor (
                                  std::abs(0.0-renderer->GetBackground()[0]),
                                  std::abs(1.0-renderer->GetBackground()[1]),
                                  std::abs(1.0-renderer->GetBackground()[2])
                                  );                                        
                      }
                      else{
                            txtActor->GetTextProperty()->SetColor (
                                  std::abs(0.9-renderer->GetBackground()[0]),
                                  std::abs(0.9-renderer->GetBackground()[1]),
                                  std::abs(0.5-renderer->GetBackground()[2])
                                  );                                        
                            
                      }                        
                      txtActor->SetVisibility(false);
                 }
                 if((now >= tab.second->tin) && (now <= tab.second->tout))
                 {
                      actor->SetVisibility(true);
                      txtActor->SetVisibility(true);
                 }
                 else
                 {
                      actor->SetVisibility(false);
                      txtActor->SetVisibility(false);
                 }
                 if(once)
                 {
                      renderer->AddActor(actor);
                      renderer->AddActor(txtActor);
                      if(countTrains == extern_trainTimeTables.size())
                           once = 0;
                 }

                 countTrains++;
            }// time table




         if(frame==NULL)
        {

        } else {

            nPeds= frame->getSize();

            if(SystemSettings::get2D()==true) {
                vtkPolyData* pData=frame->GetPolyData2D();
                extern_glyphs_pedestrians->SetInputData(pData);
                extern_pedestrians_labels->GetMapper()->SetInputDataObject(pData);
                extern_glyphs_directions->SetInputData(pData);
                extern_glyphs_pedestrians->Update();
                extern_glyphs_directions->Update();
            } else {
                vtkPolyData* pData=frame->GetPolyData3D();
                extern_glyphs_pedestrians_3D->SetInputData(pData);
                extern_pedestrians_labels->GetMapper()->SetInputDataObject(pData);
                extern_glyphs_pedestrians_3D->Update();
            }
            auto FrameElements =  frame->GetFrameElements();
            if(FrameElements.size())
                 minFrame = frame->GetFrameElements()[0]->GetMinFrame();
            else
                 minFrame = 0;

            frameNumber += minFrame;
            if(SystemSettings::getShowTrajectories()) {
                const std::vector<FrameElement *> &elements=frame->GetFrameElements();

                for(unsigned int i=0; i<elements.size(); i++) {
                    FrameElement* el = elements[i];
                    double pos[3];
                    double color;
                    el->GetPos(pos);
                    el->GetColor(&color);
                    extern_trail_plotter->PlotPoint(pos,color);
                }
            }

            //virtual cam
            if(SystemSettings::getVirtualAgent()!=-1){
                updateVirtualCamera(frame,renderer);
            }
        }
    }

    int* winSize=renderWindow->GetSize();
    static int  lastWinX=winSize[0]+1; // +1 to trigger a first change
    static int lastWinY=winSize[1];
    // HHHHHH
    sprintf(runningTimeText,"Pedestrians: %d      Time: %ld Sec",nPeds,frameNumber*iren->GetTimerDuration(tid)/1000);
    runningTime->SetInput(runningTimeText);
    runningTime->Modified();

    if((lastWinX!=winSize[0]) || (lastWinY!=winSize[1]) /*|| (frameNumber<10)*/) {
        static std::string winBaseName(renderWindow->GetWindowName());
        std::string winName=winBaseName;
        std::string s;
        winName.append(" [ ");
        s=QString::number(winSize[0]).toStdString();
        winName.append(s);
        winName.append(" x ");
        s=QString::number(winSize[1]).toStdString();
        winName.append(s);
        winName.append(" ] -->");

        int posY=winSize[1]*(1.0-30.0/536.0);
        int posX=winSize[0]*(1.0-450.0/720.0);
        runningTime->SetPosition(posX,posY);
        renderWindow->SetWindowName(winName.c_str());

        lastWinX=winSize[0];
        lastWinY=winSize[1];
    }

    iren->Render();

    if(extern_force_system_update) {
        updateSettings(renderWindow);
    }
    if(extern_take_screenshot) {
        takeScreenshot(renderWindow);
    }
    if(SystemSettings::getRecordPNGsequence()) {
        takeScreenshotSequence(renderWindow);
    }

    if (frameNumber!=0) {
        int desiredfps=1000.0/iren->GetTimerDuration(tid);
        int effectivefps=1/(renderer->GetLastRenderTimeInSeconds());

        effectivefps = (effectivefps>desiredfps)?desiredfps:effectivefps;

        emit signalFrameNumber(frameNumber, minFrame);
        emit signalRunningTime(frameNumber*iren->GetTimerDuration(tid));
        emit signalRenderingTime(effectivefps);

    }

    if(extern_shutdown_visual_thread) {
         emit signalFrameNumber(0, 0);

        // this will force an update of the windows
        lastWinX=0;
        lastWinY=0;
        //exit if and only if the recording process is terminated
        if(isRecording) extern_recording_enable=false;
        else iren->ExitCallback();
    }
}

void TimerCallback::updateSettings(vtkRenderWindow* renderWindow)
{
    static bool fullscreen=false;

    extern_glyphs_pedestrians_actor_2D->SetVisibility(SystemSettings::getShowAgents()&& SystemSettings::get2D());
    extern_glyphs_pedestrians_actor_3D->SetVisibility(SystemSettings::getShowAgents()&& !SystemSettings::get2D());
    extern_glyphs_directions_actor->SetVisibility(SystemSettings::getShowDirections()&& SystemSettings::get2D());
    extern_trail_plotter->SetVisibility(SystemSettings::getShowTrajectories());

    //agents captions
    extern_pedestrians_labels->SetVisibility(SystemSettings::getShowAgentsCaptions());

    //geometry captions

    //enable / disable full screen
    if(fullscreen!=extern_fullscreen_enable) {
        renderWindow->SetFullScreen(extern_fullscreen_enable);
        //renderWindow->GetRenderers()->GetFirstRenderer()->ResetCamera();
        fullscreen=extern_fullscreen_enable;
    }

    // take
    extern_force_system_update=false;
}


void TimerCallback::getTrail(int datasetID, int frameNumber)
{

    int trailCount=0;
    int trailType=0;
    int trailForm=0;
    int tcMin=0;
    int tcMax=0;

    SystemSettings::getTrailsInfo(&trailCount,&trailType,&trailForm);

    switch(trailType) {
    case 0://backward
        tcMin=frameNumber-trailCount;
        tcMax=frameNumber;
        break;

    case 1://symetric
        tcMin=frameNumber-trailCount/2;
        tcMax=frameNumber+trailCount/2;

        break;

    case 2://forward
        tcMin=frameNumber;
        tcMax=frameNumber+trailCount;
        break;

    }


    for (int i=tcMin; i<tcMax; i++) {
        Frame* frame = extern_trajectories_firstSet.getFrame(i);
        if(frame==NULL) {
            //		cerr<<"Trajectory not available in getTrail(), first data set"<<endl;
        } else {
            FrameElement* point=NULL;
            while(NULL!=(point=frame->getNextElement())) {
                //extern_pedestrians_firstSet[point->getIndex()]->plotTrail(point->getX(),point->getY(),point->getZ());
                //extern_pedestrians_firstSet[point->getIndex()]->setTrailGeometry(trailForm);
            }
            frame->resetCursor();
        }
    }
}

void TimerCallback::updateVirtualCamera(Frame *frame, vtkRenderer *renderer)
{
    //look for the position of that agent
    const std::vector <FrameElement *> elements= frame->GetFrameElements();

    for(unsigned int i=0;i<elements.size();i++){
        FrameElement * el= elements[i];
        if(el->GetId()==SystemSettings::getVirtualAgent())
        {
            static bool cam=true;
            double pos[3]={0,0,0};
            double orien[3]={0,0,0};
            double pedSize=170;
            el->GetPos(pos);
            el->GetOrientation(orien);
            double angle=vtkMath::RadiansFromDegrees(orien[2]);

            //cout<<"new Pos: "<<pos[0]<<" " <<pos[1] <<" "<<pos[2]<<endl;

            vtkCamera* virtualCam=renderer->GetActiveCamera();

            virtualCam->Print(std::cout); exit(0);
            virtualCam->SetPosition(pos[0]+15,pos[1],pos[2]+pedSize);

            //new focal point
            double eyeRange=150;//15m

            double  x=pos[0]+eyeRange*cos(angle);
            double  y=pos[1]+eyeRange*sin(angle);
            double z = pos[2];
            //virtualCam->SetFocalPoint(pos[0]+15,pos[1],pos[2]+pedSize+8);
            virtualCam->SetFocalPoint(x,y,z+pedSize+8);
            //virtualCam->Azimuth(pedestrianOrienation);
            //virtualCam->Azimuth(pedestrianOrienation);
            //virtualCam->Yaw(pedestrianOrienation);

            virtualCam->SetDistance(eyeRange);
            if(cam)virtualCam->SetRoll(90);
            cam=false;

            virtualCam->Modified();

            return;
        }
    }
}


void TimerCallback::takeScreenshot(vtkRenderWindow *renderWindow)
{
    static int imageID=0;
    vtkWindowToImageFilter * winToImFilter  = vtkWindowToImageFilter::New();
    winToImFilter->SetInput( renderWindow );
    //winToImFilter->SetMagnification(4);
    //renderWindow->Delete();
    //vtkPostScriptWriter * image  = vtkPostScriptWriter::New();
    vtkPNGWriter * image  = vtkPNGWriter::New();
    image->SetInputConnection(winToImFilter->GetOutputPort());
    winToImFilter->Delete();

    QString screenshots;
    SystemSettings::getOutputDirectory(screenshots);
    //create directory if not exits
    if(!QDir(screenshots).exists()) {
        QDir dir;
        if(!dir.mkpath (screenshots )) {
            //Debug::Error("could not create directory: %s",screenshots.toStdString().c_str());
            //try with the current directory
            screenshots="";
        }
    }


    char filename[256]= {0};
    //	sprintf(filename,"travisto_video_%d.png",imageID++);
    std::string date= QString(QDateTime::currentDateTime().toString("yyMMdd_hh")).toStdString();

    sprintf(filename,"travisto_snap_%sh_%d.png",date.c_str(),imageID++);

    //append the prefix
    screenshots+=SystemSettings::getFilenamePrefix();
    screenshots+=QString(filename);
    image->SetFileName(screenshots.toStdString().c_str());
    winToImFilter->Modified();

    image->Write ();
    image->Delete();
    extern_take_screenshot=false;
}

/// take png screenshot sequence
void TimerCallback::takeScreenshotSequence(vtkRenderWindow* renderWindow)
{
    static int imageID=0;
    vtkWindowToImageFilter * winToImFilter  = vtkWindowToImageFilter::New();
    winToImFilter->SetInput( renderWindow );
    //renderWindow->Delete();
    vtkPNGWriter * image  = vtkPNGWriter::New();
    //vtkPostScriptWriter * image  = vtkPostScriptWriter::New();
    image->SetInputConnection( winToImFilter->GetOutputPort());
    winToImFilter->Delete();

    QString screenshots;
    SystemSettings::getOutputDirectory(screenshots);

    screenshots.append("./png_seq_"+QDateTime::currentDateTime().toString("yyMMddhh")+"_"+SystemSettings::getFilenamePrefix());
    screenshots.truncate(screenshots.size()-1);

    //create directory if not exits
    if(!QDir(screenshots).exists()) {
        QDir dir;
        if(!dir.mkpath (screenshots )) {
            cerr<<"could not create directory: "<< screenshots.toStdString();
            //try with the current directory
            screenshots="./png_seq_"+QDateTime::currentDateTime().toString("yyMMdd")+"_"+SystemSettings::getFilenamePrefix();
            screenshots.truncate(screenshots.size()-1);
        }
    }


    char filename[30]= {0};
    sprintf(filename,"/tmp_%07d.png",imageID++);
    screenshots.append(filename);
    image->SetFileName(screenshots.toStdString().c_str());
    winToImFilter->Modified();

    image->Write ();
    image->Delete();
}


void TimerCallback::SetRenderTimerId(int tid)
{
    this->RenderTimerId = tid;
}

void TimerCallback::setTextActor(vtkTextActor* ra)
{
    runningTime=ra;
}


// https://vtk.org/Wiki/VTK/Examples/Cxx/GeometricObjects/ColoredLines


vtkSmartPointer<vtkPolyData>  TimerCallback::getTrainData(
      Point trainStart, Point trainEnd, std::vector<Point> doorPoints, double elevation)

{
     float factor = 100.0;

     double pt[3] = { 1.0, 0.0, 0.0 }; // to convert from Point
     // Create the polydata where we will store all the geometric data
     vtkSmartPointer<vtkPolyData> linesPolyData =
          vtkSmartPointer<vtkPolyData>::New();

     // Create a vtkPoints container and store the points in it
     vtkSmartPointer<vtkPoints> pts =
          vtkSmartPointer<vtkPoints>::New();

     pt[0] = factor*trainStart._x; 
     pt[1] = factor*trainStart._y;
     pt[2] = factor*elevation;
     pts->InsertNextPoint(pt);

     for(auto p: doorPoints)
     {
          pt[0] = factor*p._x; 
          pt[1] = factor*p._y;
          pt[2] = factor*elevation;
          pts->InsertNextPoint(pt);
     }
     pt[0] = factor*trainEnd._x; 
     pt[1] = factor*trainEnd._y;
     pt[2] = factor*elevation;
     pts->InsertNextPoint(pt);


     // Add the points to the polydata container
     linesPolyData->SetPoints(pts);


     vtkSmartPointer<vtkCellArray> lines =
          vtkSmartPointer<vtkCellArray>::New();


     // Create the first line (between Origin and P0)
     for(int i = 0; i<= doorPoints.size(); i+=2 )
     {
          vtkSmartPointer<vtkLine> line =
               vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, i);
          line->GetPointIds()->SetId(1, i+1);

          lines->InsertNextCell(line);
          lines->InsertNextCell(line);
          line = nullptr;
     }

     // Add the lines to the polydata container
     linesPolyData->SetLines(lines);
     return linesPolyData;

}
