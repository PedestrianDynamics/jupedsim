/**
 * @headerfile TimerCallback.cpp
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

#ifndef TIMERCALLBACK_H_
#define TIMERCALLBACK_H_

#include <iostream>
#include <vtkCommand.h>

#include "SyncData.h"

#ifdef WIN32
class vtkAVIWriter;
#endif
//
#ifdef __linux__
class vtkFFMPEGWriter;
#endif


//#include <vtkGlyph3D.h>
#include <vtkTensorGlyph.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>

class SyncData;
class QObject;
class QString;
class Pedestrian;
class Pedestrian;
class vtkCommand;
class vtkRenderWindow;
class vtkWindowToImageFilter;
class Frame;
class vtkPNGWriter;
class vtkTextActor;
class vtkActor2D;

//extern variables

extern bool extern_force_system_update;
extern bool extern_is_pause;

extern bool extern_shutdown_visual_thread;
extern bool extern_recording_enable;
extern bool extern_launch_recording;
extern bool extern_fullscreen_enable;
extern bool extern_take_screenshot;

extern Pedestrian** extern_pedestrians_firstSet;
extern Pedestrian** extern_pedestrians_secondSet;
extern Pedestrian** extern_pedestrians_thirdSet;

extern vtkSmartPointer<vtkActor2D> extern_pedestrians_labels;
extern vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians;
extern vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians_3D;
extern vtkSmartPointer<vtkActor> extern_glyphs_pedestrians_actor_2D;
extern vtkSmartPointer<vtkActor> extern_glyphs_pedestrians_actor_3D;


extern SyncData extern_trajectories_firstSet;
extern SyncData extern_trajectories_secondSet;
extern SyncData extern_trajectories_thirdSet;

//states if the datasets are loaded.
extern bool extern_first_dataset_loaded;
extern bool extern_second_dataset_loaded;
extern bool extern_third_dataset_loaded;

//states whether the loaded datasets are visible
extern bool extern_first_dataset_visible;
extern bool extern_second_dataset_visible;
extern bool extern_third_dataset_visible;



class TimerCallback :public QObject, public vtkCommand{

	Q_OBJECT

private:
	int RenderTimerId;
	vtkWindowToImageFilter* windowToImageFilter;
	vtkPNGWriter *pngWriter;
	vtkTextActor* runningTime;
	char runningTimeText[50];

#ifdef WIN32
	vtkAVIWriter* pAVIWriter;
#endif

#ifdef __linux__
	vtkFFMPEGWriter* pAVIWriter;
#endif



public:
	static TimerCallback *New();

	virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData);

	void SetRenderTimerId(int tid);

	void setTextActor(vtkTextActor* runningTime);

private:
	///updates system global changes, like fullscreen, ffw and soone
	void updateSettings(vtkRenderWindow* renderWindow);

	/// make a png screenshot of the renderwindows
	void takeScreenshot(vtkRenderWindow* renderWindow);

	/// take png screenshots sequence
	void takeScreenshotSequence(vtkRenderWindow* renderWindow);

	/// set all pedestrians invisible.
	/// this is necessary for blending the peds
	/// who are not longer in the system.
	void setAllPedestriansInvisible();

	/// create directory
	//bool createDirectory( const std::string& ac_sPath );

	/// mark pedestrians as out of the system
	/// when they were present in the last frame
	/// but not in the current
	/// @param previous Frame current Frame datasetID (1, 2 or 3)
	// void checkIfOutOfSystem(Frame* previous, Frame* current,int datasetID);

	/// extract from the
	//void getTrail(int datasetID,double* trailX, double*  trailY, double* trailZ, int frameNumber,int trailCount);
	void getTrail(int datasetID,int frameNumber);


	Q_SIGNALS:
	void signalStatusMessage(QString msg);
	void signalFrameNumber(unsigned long timems);
	void signalRunningTime(unsigned long timems);
	void signalRenderingTime(int fps);

};


/**
 * A callback for windows changes
 */

//class WindowCallback: public vtkCommand{
//public:
//	static WindowCallback *New();
//	virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData);
//};


#endif /* TIMERCALLBACK_H_ */


