/**
* @headerfile ThreadDataTransfert.cpp
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
* This class implements the thread that manages the main visulisation
*
* \brief visualise the data
*
*
*
*  Created on: 22.05.2009
*
*/


#ifndef THREADVISUALISATION_H_
#define THREADVISUALISATION_H_

#include <QThread>
#include <QObject>
#include <vtkGlyph3D.h>
#include <vtkTensorGlyph.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>

// forwarded classes
class QThread;
class QObject;

class vtkRenderer;
class vtkAssembly;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkActor;
class vtkAxesActor;
class vtkCamera;
class vtkTextActor;
class vtkObject;

class Pedestrian;
class SyncData;
class FacilityGeometry;

extern Pedestrian** extern_pedestrians_firstSet;
extern Pedestrian** extern_pedestrians_secondSet;
extern Pedestrian** extern_pedestrians_thirdSet;

//extern vtkSmartPointer<vtkGlyph3D> extern_glyphs_pedestrians;
extern vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians;
extern vtkSmartPointer<vtkTensorGlyph> extern_glyphs_pedestrians_3D;
extern vtkSmartPointer<vtkActor> extern_glyphs_pedestrians_actor_2D;
extern vtkSmartPointer<vtkActor> extern_glyphs_pedestrians_actor_3D;

extern SyncData extern_trajectories_firstSet;
extern SyncData extern_trajectories_secondSet;
extern SyncData extern_trajectories_thirdSet;

class ThreadVisualisation :public QThread {
	Q_OBJECT

public:
	ThreadVisualisation(QObject *parent = 0);
	virtual ~ThreadVisualisation();
	virtual void run();

	void setAxisVisible(bool status);

	void setFullsreen(bool status);

	/// set the camera to one of TOP/FRONT/SIDE
	void setCameraPerspective(int mode);

	/// load and display the geometry where
	/// the pedestrians will move
	void setGeometry(FacilityGeometry* geometry);

	FacilityGeometry* getGeometry();

	///this is for convenience and will be delete later
	void setWindowTitle(QString title);

	/// shutdown the thread
	//void shutdown();

	void setGeometryLabelsVisibility(int v);

	/// set geometry visibility
	void setGeometryVisibility(bool status);

	/// enable/disable 2D
	void setGeometryVisibility2D(bool status);

	/// enable/disable 3D
	void setGeometryVisibility3D(bool status);

	/// set trajectories/trail visibility
	void setTrailVisibility(bool status);

	/// change the background color of the rendering windows
	void setBackgroundColor(double* color);

	/// change the walls color
	void setWallsColor(double* color);

	/// change the exits color.
	void setExitsColor(double* color);

	/// show / hide the walls
	void showWalls(bool status);

	/// show/ hide the exits
	void showDoors(bool status);

	/// show / hide stairs
	///not implemented
	void showStairs(bool status);

	void setOnscreenInformationVisibility(bool show);

public Q_SLOTS:
	/**control sequence received*/
    void slotControlSequence(const char* para);

	/// set the frame rate in frames per second
	void slotSetFrameRate( float fps);


Q_SIGNALS:
    void signal_controlSequences(const char* para);

private:


	/// initialize the legend
	void initLegend(/*std::vector scalars*/);

	/// initialize the datasets
	void init();

	/// initialize the datasets
    void initGlyphs2D();

    //initialize the 3D agents
    void initGlyphs3D();

	//finalize the datasets
	void finalize();

	/// compute the relative delays to the datasets
	void computeDelays();

	/// window change events
	//static void WindowModifiedCallback(vtkObject* caller, long unsigned int  eventId, void* clientData, void* callData );


private:
	FacilityGeometry* geometry;
	vtkRenderer* renderer;
	vtkRenderWindow* renderWindow;
	vtkRenderWindowInteractor* renderWinInteractor;
	vtkAxesActor* axis;
	vtkTextActor* runningTime;
    vtkCamera* _topViewCamera;
	QString winTitle;

	float framePerSecond;
	double camPosTop[3];
	double camClipTop[3];
	double camFocalPointTop[3];
	double camViewUpTop[3];
	double camViewAngleTop;
	double camParallelScale;
	double camViewPlanNormalTop[3];

};

#endif /* THREADVISUALISATION_H_ */
