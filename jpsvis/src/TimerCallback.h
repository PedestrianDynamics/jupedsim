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

#include "SyncData.h"

#include <iostream>
#include <vtkCommand.h>

#ifdef WIN32
class vtkAVIWriter;
#endif
//
#ifdef __linux__
class vtkFFMPEGWriter;
#endif


//#include <vtkGlyph3D.h>
#include "trains/train.h"

#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTensorGlyph.h>

class SyncData;
class QObject;
class QString;
class Pedestrian;
class vtkCommand;
class vtkRenderWindow;
class vtkWindowToImageFilter;
class Frame;
class vtkPNGWriter;
class vtkTextActor;
class vtkActor2D;
class TrailPlotter;
class PointPlotter;

// extern variables

extern bool extern_force_system_update;
extern bool extern_is_pause;

extern bool extern_shutdown_visual_thread;
extern bool extern_recording_enable;
extern bool extern_launch_recording;
extern bool extern_fullscreen_enable;
extern bool extern_take_screenshot;

extern Pedestrian ** extern_pedestrians_firstSet;

extern vtkActor2D * extern_pedestrians_labels;
extern vtkTensorGlyph * extern_glyphs_pedestrians;
extern vtkTensorGlyph * extern_glyphs_pedestrians_3D;
extern vtkTensorGlyph * extern_glyphs_directions;
extern vtkActor * extern_glyphs_pedestrians_actor_2D;
extern vtkActor * extern_glyphs_pedestrians_actor_3D;
extern vtkActor * extern_glyphs_directions_actor;
extern PointPlotter * extern_trail_plotter;

extern SyncData extern_trajectories_firstSet;

extern SyncData mysphere;
extern std::map<std::string, std::shared_ptr<TrainType>> extern_trainTypes;
extern std::map<int, std::shared_ptr<TrainTimeTable>> extern_trainTimeTables;
// states if the datasets are loaded.
extern bool extern_first_dataset_loaded;

// states whether the loaded datasets are visible
extern bool extern_first_dataset_visible;


class TimerCallback : public QObject, public vtkCommand
{
    Q_OBJECT

private:
    int RenderTimerId;
    vtkWindowToImageFilter * windowToImageFilter;
    vtkPNGWriter * pngWriter;
    vtkTextActor * runningTime;
    char runningTimeText[50];

#ifdef WIN32
    vtkAVIWriter * pAVIWriter;
#endif

#ifdef __linux__
    vtkFFMPEGWriter * pAVIWriter;
#endif


public:
    static TimerCallback * New();

    virtual void Execute(vtkObject * caller, unsigned long eventId, void * callData);

    void SetRenderTimerId(int tid);

    void setTextActor(vtkTextActor * runningTime);
    // vtkSmartPointer<vtkActor> setTrainActor(
    //      Point trainStart, Point trainEnd, std::vector<Point> doorPoints);

    vtkSmartPointer<vtkPolyData>
    getTrainData(Point trainStart, Point trainEnd, std::vector<Point> doorPoints, double elevation);


private:
    /// updates system global changes, like fullscreen, ffw and soone
    void updateSettings(vtkRenderWindow * renderWindow);

    /// make a png screenshot of the renderwindows
    void takeScreenshot(vtkRenderWindow * renderWindow);

    /// take png screenshots sequence
    void takeScreenshotSequence(vtkRenderWindow * renderWindow);

    /// extract from the
    void getTrail(int datasetID, int frameNumber);

    /// update the virtual camera
    void updateVirtualCamera(Frame * frame, vtkRenderer * renderer);


Q_SIGNALS:
    void signalStatusMessage(QString msg);
    void signalFrameNumber(unsigned long timems, unsigned long minframe);
    void signalRunningTime(unsigned long timems);
    void signalRenderingTime(int fps);
};


/**
 * A callback for windows changes
 */

// class WindowCallback: public vtkCommand{
// public:
//	static WindowCallback *New();
//	virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData);
//};


#endif /* TIMERCALLBACK_H_ */
