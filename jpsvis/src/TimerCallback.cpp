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


#ifdef WIN32
#include <vtkAVIWriter.h>
#include <windows.h>
#endif


#ifdef __linux__
#include <vtkFFMPEGWriter.h>
#endif

#include <QObject>
#include <QString>
#include <QTime>
#include <QDir>
#include <qwaitcondition.h>

#include <vtkCommand.h>
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


#include "geometry/FacilityGeometry.h"
#include "geometry/Point.h"

#include "Pedestrian.h"
#include "Frame.h"
#include "TrajectoryPoint.h"
#include "SyncData.h"
#include "SystemSettings.h"
#include "TimerCallback.h"

//#include <vtkPostScriptWriter.h>

using namespace std;
TimerCallback* TimerCallback::New()
{
	TimerCallback *cb = new TimerCallback;
	cb->RenderTimerId = 0;
	cb->windowToImageFilter=NULL;
	cb->runningTime=NULL;
	return cb;
}


void TimerCallback::Execute(vtkObject *caller, unsigned long eventId,
		void *callData){
	if (vtkCommand::TimerEvent == eventId)
	{
		int  frameNumber=0;
		int nPeds=0;
		static bool isRecording =false;
		int tid = * static_cast<int *>(callData);
		//double renderingTime=0;

		if (tid == this->RenderTimerId)
		{
			//dont update anything if the system is actually paused
			//if(extern_is_pause) return;

			vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast(caller);
			vtkRenderWindow  *renderWindow = iren->GetRenderWindow();
			vtkRenderer *renderer =renderWindow->GetRenderers()->GetFirstRenderer();

			if (iren && renderWindow && renderer)
			{
				// very important
				setAllPedestriansInvisible();

				//first pedestrian group
				if(extern_first_dataset_loaded) {
					Frame * frame=NULL;

					// return the same frame if the system is paused
					// in fact you could just return, but in this case no update will be made
					// e.g showing captions/trails...

					if(extern_is_pause)
						frame=extern_trajectories_firstSet.getFrame(extern_trajectories_firstSet.getFrameCursor());
					else
						frame = extern_trajectories_firstSet.getNextFrame();

					if(frame==NULL){

					}else{

						// just take the frame number given by this dataset
						frameNumber=extern_trajectories_firstSet.getFrameCursor();
						if(extern_tracking_enable)
						getTrail(1,frameNumber);

						TrajectoryPoint* point=NULL;
						while(NULL!=(point=frame->getNextElement())){
//							if(point->getIndex()==152) continue;
//							if(point->getIndex()==60) continue;
//							if(point->getIndex()==57) continue;
//							if(point->getIndex()==9) continue;
//							if(point->getIndex()==124) continue;
//							if(point->getIndex()==81) continue;
//							if(point->getIndex()==55) continue;
//
//							if(point->getIndex()==158) continue;
//							if(point->getIndex()==192) continue;
//							if(point->getIndex()==150) continue;
//							if(point->getIndex()==169) continue;
//							if(point->getIndex()==113) continue;
//
							//if(point->getIndex()==16) continue;
							//if(point->getIndex()==17) continue;

							//point index start at 1. this  may needed to be fixed
							extern_pedestrians_firstSet[point->getIndex()]->moveTo(point);
							nPeds++;
						}
						//CAUTION: reset the fucking counter
						frame->resetCursor();
					}
				}

				//second pedestrian group
				if(extern_second_dataset_loaded){

					Frame * frame=NULL;
					if(extern_is_pause)
						frame=extern_trajectories_secondSet.getFrame(extern_trajectories_secondSet.getFrameCursor());
					else
						frame = extern_trajectories_secondSet.getNextFrame();

					if(frame==NULL){

					}else{
						// just take the frame number given by this dataset
						frameNumber=extern_trajectories_secondSet.getFrameCursor();
						if(extern_tracking_enable)
						getTrail(2,frameNumber);

						TrajectoryPoint* point=NULL;
						while(NULL!=(point=frame->getNextElement())){
							extern_pedestrians_secondSet[point->getIndex()]->moveTo(point);
						}
						//CAUTION: reset the fucking counter
						frame->resetCursor();
					}
				}

				//third pedestrian group
				if(extern_third_dataset_loaded){

					Frame * frame=NULL;
					if(extern_is_pause)
						frame=extern_trajectories_thirdSet.getFrame(extern_trajectories_thirdSet.getFrameCursor());
					else
						frame = extern_trajectories_thirdSet.getNextFrame();

					if(frame==NULL){

					}else {
						// just take the frame number given by this dataset
						frameNumber=extern_trajectories_thirdSet.getFrameCursor();
						if(extern_tracking_enable)
						getTrail(3,frameNumber);

						TrajectoryPoint* point=NULL;
						while(NULL!=(point=frame->getNextElement())){
							//point index start at 1. this  may needed to be fixed
							extern_pedestrians_thirdSet[point->getIndex()]->moveTo(point);
							//set visible to true
						}
						//CAUTION: reset the fucking counter
						frame->resetCursor();
					}
				}

				int* winSize=renderWindow->GetSize();
				static int  lastWinX=winSize[0]+1; // +1 to trigger a first change
				static int lastWinY=winSize[1];

				sprintf(runningTimeText,"Pedestrians: %d      Time: %ld Sec",nPeds,frameNumber*iren->GetTimerDuration(tid)/1000);
				runningTime->SetInput(runningTimeText);
				runningTime->Modified();

				if((lastWinX!=winSize[0]) || (lastWinY!=winSize[1]) || (frameNumber<10))
				{
					static std::string winBaseName(renderWindow->GetWindowName());
					std::string winName=winBaseName;
					std::string s;
					winName.append(" [ ");
					s=QString::number(winSize[0]).toStdString();
					winName.append(s);
					winName.append("X");
					s=QString::number(winSize[1]).toStdString();
					winName.append(s);
					winName.append(" ] ");

					int posY=winSize[1]*(1.0-30.0/536.0);
					int posX=winSize[0]*(1.0-450.0/720.0);
					runningTime->SetPosition(posX,posY);
					renderWindow->SetWindowName(winName.c_str());

					lastWinX=winSize[0];
					lastWinY=winSize[1];
				}

				iren->Render();

				if(extern_force_system_update){
					updateSettings(renderWindow);
				}
				if(extern_take_screenshot){
					takeScreenshot(renderWindow);
				}
				if(SystemSettings::getRecordPNGsequence()){
					takeScreenshotSequence(renderWindow);
				}

				if (frameNumber!=0) {
					int desiredfps=1000.0/iren->GetTimerDuration(tid);
					int effectivefps=1/(renderer->GetLastRenderTimeInSeconds());

					effectivefps = (effectivefps>desiredfps)?desiredfps:effectivefps;

					emit signalFrameNumber(frameNumber);
					emit signalRunningTime(frameNumber*iren->GetTimerDuration(tid));
					emit signalRenderingTime(effectivefps);
				}


				if(extern_launch_recording){
					extern_launch_recording=false; //reset

					windowToImageFilter=vtkWindowToImageFilter::New();
#ifdef WIN32
					pAVIWriter=vtkAVIWriter::New();
#endif

#ifdef __linux__
					pAVIWriter=vtkFFMPEGWriter::New();
#endif

					pAVIWriter->SetQuality(2);//FIXME 2 is better
					pAVIWriter->SetRate(1000.0/iren->GetTimerDuration(tid));

					//static int videoID=0;
					//char filename[20]={0};
					//sprintf(filename,"travisto_video_%d.avi",videoID++);
					//pAVIWriter->SetFileName(filename);


					QString videoName;
					SystemSettings::getOutputDirectory(videoName);
					//create directory if not exits
					if(!QDir(videoName).exists()){
						QDir dir;
						if(!dir.mkpath (videoName )){
							cerr<<"could not create directory: "<< videoName.toStdString();
							videoName=""; // current
						}
					}

					videoName += "/tvtvid_"+QDateTime::currentDateTime().toString("yyMMdd_hh_mm_").append(SystemSettings::getFilenamePrefix()).append(".avi");

					pAVIWriter->SetFileName(videoName.toStdString().c_str());

					if(windowToImageFilter!=NULL)
						if(windowToImageFilter->GetInput()==NULL){ //should be the case by first call
							windowToImageFilter->SetInput(renderWindow);
							pAVIWriter->SetInput(windowToImageFilter->GetOutput());
							pAVIWriter->Start();
						}
					extern_recording_enable=true;
					isRecording=true;
				}

				if(isRecording){
					windowToImageFilter->Modified();
					// only write when not paused
					if(!extern_is_pause) pAVIWriter->Write();

					if(extern_recording_enable==false){ //stop the recording
						pAVIWriter->End();
						windowToImageFilter->Delete();
						pAVIWriter->Delete();
						isRecording=false;
					}
				}

				if(extern_shutdown_visual_thread){
					emit signalFrameNumber(0);

					// this will force an update of the windows
					lastWinX=0;
					lastWinY=0;
					//exit if and only if the recording process is terminated
					if(isRecording) extern_recording_enable=false;
					else iren->ExitCallback();


				}
			}
		}
	}
}

void TimerCallback::updateSettings(vtkRenderWindow* renderWindow) {

	static bool fullscreen=false;

	// check the caption colour mode
	int captionSize, orientation;
	bool automaticRotation, autoCaptionMode;
	QColor captionColor;
	SystemSettings::getCaptionsParameters(captionSize,captionColor,orientation,automaticRotation);

	autoCaptionMode= !(captionColor.isValid());

	if(autoCaptionMode==false){ // tODO set the colour to auto mode

	}

	if(extern_first_dataset_loaded){
		int pedColor[3];
		SystemSettings::getPedestrianColor(0,pedColor);
		for(int i=0;i<extern_trajectories_firstSet.getNumberOfAgents();i++){
			extern_pedestrians_firstSet[i]->enableCaption(SystemSettings::getShowCaption());
			if(SystemSettings::getPedestrianColorProfileFromFile()==false){
				extern_pedestrians_firstSet[i]->setColor(pedColor);
			}
			extern_pedestrians_firstSet[i]->setGroupVisibility(extern_first_dataset_visible);
			extern_pedestrians_firstSet[i]->setCaptionSize(captionSize);
			extern_pedestrians_firstSet[i]->setCaptionsColorModeToAuto(autoCaptionMode);
			extern_pedestrians_firstSet[i]->setCaptionsColor(captionColor);
			extern_pedestrians_firstSet[i]->setResolution(SystemSettings::getEllipseResolution());
		}
	}

	if(extern_second_dataset_loaded){
		int pedColor[3];
		SystemSettings::getPedestrianColor(1,pedColor);
		for(int i=0;i<extern_trajectories_secondSet.getNumberOfAgents();i++){
			extern_pedestrians_secondSet[i]->enableCaption(SystemSettings::getShowCaption());
			if(SystemSettings::getPedestrianColorProfileFromFile()==false){
				extern_pedestrians_secondSet[i]->setColor(pedColor);
			}
			extern_pedestrians_secondSet[i]->setGroupVisibility(extern_second_dataset_visible);
			extern_pedestrians_secondSet[i]->setCaptionSize(captionSize);
			extern_pedestrians_secondSet[i]->setCaptionsColorModeToAuto(autoCaptionMode);
			extern_pedestrians_secondSet[i]->setCaptionsColor(captionColor);
			extern_pedestrians_secondSet[i]->setResolution(SystemSettings::getEllipseResolution());

		}
	}

	if(extern_third_dataset_loaded){
		int pedColor[3];
		SystemSettings::getPedestrianColor(2,pedColor);
		for(int i=0;i<extern_trajectories_thirdSet.getNumberOfAgents();i++){
			extern_pedestrians_thirdSet[i]->enableCaption(SystemSettings::getShowCaption());

			if(SystemSettings::getPedestrianColorProfileFromFile()==false){
				extern_pedestrians_thirdSet[i]->setColor(pedColor);
			}
			extern_pedestrians_thirdSet[i]->setGroupVisibility(extern_third_dataset_visible);
			extern_pedestrians_thirdSet[i]->setCaptionSize(captionSize);
			extern_pedestrians_thirdSet[i]->setCaptionsColorModeToAuto(autoCaptionMode);
			extern_pedestrians_thirdSet[i]->setCaptionsColor(captionColor);
			extern_pedestrians_thirdSet[i]->setResolution(SystemSettings::getEllipseResolution());
		}
	}

	//enable / disable full screen
	if(fullscreen!=extern_fullscreen_enable){
		renderWindow->SetFullScreen(extern_fullscreen_enable);
		//renderWindow->GetRenderers()->GetFirstRenderer()->ResetCamera();
		fullscreen=extern_fullscreen_enable;
	}

	// take
	extern_force_system_update=false;
}


void TimerCallback::setAllPedestriansInvisible()
{
	if(extern_first_dataset_loaded){
		for(int i=0;i<extern_trajectories_firstSet.getNumberOfAgents();i++)
			extern_pedestrians_firstSet[i]->setVisibility(false);
	}

	if(extern_second_dataset_loaded){
		for(int i=0;i<extern_trajectories_secondSet.getNumberOfAgents();i++)
			extern_pedestrians_secondSet[i]->setVisibility(false);
	}

	if(extern_third_dataset_loaded){
		for(int i=0;i<extern_trajectories_thirdSet.getNumberOfAgents();i++)
			extern_pedestrians_thirdSet[i]->setVisibility(false);
	}
}



void TimerCallback::getTrail(int datasetID, int frameNumber){

	int trailCount=0;
	int trailType=0;
	int trailForm=0;
	int tcMin=0;
	int tcMax=0;

	SystemSettings::getTrailsInfo(&trailCount,&trailType,&trailForm);

	switch(trailType){
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

	switch(datasetID){
	case 1:
	{
		for (int i=tcMin;i<tcMax;i++){
			Frame* frame = extern_trajectories_firstSet.getFrame(i);
			if(frame==NULL){
				//		cerr<<"Trajectory not available in getTrail(), first data set"<<endl;
			}else {
				TrajectoryPoint* point=NULL;
				while(NULL!=(point=frame->getNextElement())){
					extern_pedestrians_firstSet[point->getIndex()]->plotTrail(point->getX(),point->getY(),point->getZ());
					extern_pedestrians_firstSet[point->getIndex()]->setTrailGeometry(trailForm);
				}
				frame->resetCursor();
			}
		}
	}
	break;

	case 2:
	{
		for (int i=tcMin;i<tcMax;i++){
			Frame* frame = extern_trajectories_secondSet.getFrame(i);
			if(frame==NULL){
				//			cerr<<"Trajectory not available in getTrail(), second data set"<<endl;
			}else {

				TrajectoryPoint* point=NULL;
				while(NULL!=(point=frame->getNextElement())){
					extern_pedestrians_secondSet[point->getIndex()]->plotTrail(point->getX(),point->getY(),point->getZ());
					extern_pedestrians_secondSet[point->getIndex()]->setTrailGeometry(trailForm);
				}
				frame->resetCursor();
			}
		}
	}

	break;

	case 3:
	{
		for (int i=tcMin;i<tcMax;i++){
			Frame* frame = extern_trajectories_thirdSet.getFrame(i);
			if(frame==NULL){
				//			cerr<<"Trajectory not available in getTrail(), third data set"<<endl;
			}else {

				TrajectoryPoint* point=NULL;
				while(NULL!=(point=frame->getNextElement())){
					extern_pedestrians_thirdSet[point->getIndex()]->plotTrail(point->getX(),point->getY(),point->getZ());
					extern_pedestrians_thirdSet[point->getIndex()]->setTrailGeometry(trailForm);
				}
				frame->resetCursor();
			}
		}
	}

	break;

	}
}


void TimerCallback::takeScreenshot(vtkRenderWindow *renderWindow){
	static int imageID=0;
	vtkWindowToImageFilter * winToImFilter  = vtkWindowToImageFilter::New();
	winToImFilter->SetInput( renderWindow );
	//winToImFilter->SetMagnification(4);
	//renderWindow->Delete();
	//vtkPostScriptWriter * image  = vtkPostScriptWriter::New();
	vtkPNGWriter * image  = vtkPNGWriter::New();
	image->SetInput( winToImFilter->GetOutput());
	winToImFilter->Delete();

	QString screenshots;
	SystemSettings::getOutputDirectory(screenshots);
	//create directory if not exits
	if(!QDir(screenshots).exists()){
		QDir dir;
		if(!dir.mkpath (screenshots )){
			//Debug::Error("could not create directory: %s",screenshots.toStdString().c_str());
			//try with the current directory
			screenshots="";
		}
	}


	char filename[256]={0};
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
void TimerCallback::takeScreenshotSequence(vtkRenderWindow* renderWindow){
	static int imageID=0;
	vtkWindowToImageFilter * winToImFilter  = vtkWindowToImageFilter::New();
	winToImFilter->SetInput( renderWindow );
	//renderWindow->Delete();
	vtkPNGWriter * image  = vtkPNGWriter::New();
	//vtkPostScriptWriter * image  = vtkPostScriptWriter::New();
	image->SetInput( winToImFilter->GetOutput());
	winToImFilter->Delete();

	QString screenshots;
	SystemSettings::getOutputDirectory(screenshots);

	screenshots.append("./png_seq_"+QDateTime::currentDateTime().toString("yyMMdd_hh_mm")+"_"+SystemSettings::getFilenamePrefix());

	//create directory if not exits
	if(!QDir(screenshots).exists()){
		QDir dir;
		if(!dir.mkpath (screenshots )){
			cerr<<"could not create directory: "<< screenshots.toStdString();
			//try with the current directory
			screenshots="./png_seq_"+QDateTime::currentDateTime().toString("yyMMdd_hh_mm")+"_"+SystemSettings::getFilenamePrefix();
		}
	}


	char filename[30]={0};
	sprintf(filename,"/tmp_%d.png",imageID++);
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

void TimerCallback::setTextActor(vtkTextActor* ra){
	runningTime=ra;
}


//
//WindowCallback* WindowCallback::New()
//{
//	WindowCallback *cb = new WindowCallback;
//	return cb;
//}


//void WindowCallback::Execute(vtkObject *caller, unsigned long eventId,
//		void *callData){
//
//
//}
