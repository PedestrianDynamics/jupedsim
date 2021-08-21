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
#include "geometry/GeometryFactory.h"

// forwarded classes
class QThread;
class QObject;
class QColor;

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
class TrailPlotter;
class PointPlotter;

extern Pedestrian** extern_pedestrians_firstSet;

extern PointPlotter* extern_trail_plotter;

extern vtkTensorGlyph* extern_glyphs_pedestrians;
extern vtkTensorGlyph* extern_glyphs_pedestrians_3D;
extern vtkTensorGlyph* extern_glyphs_directions;
extern vtkActor* extern_glyphs_pedestrians_actor_2D;
extern vtkActor* extern_glyphs_pedestrians_actor_3D;
extern vtkActor* extern_glyphs_directions_actor;

extern SyncData extern_trajectories_firstSet;
// extern vtkSmartPointer<vtkSphereSource> extern_mysphere;


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
    void setGeometry(FacilityGeometry* _geometry);

    GeometryFactory& getGeometry();

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

    /// change the background color of the rendering windows
    void setBackgroundColor(const QColor &col);

    /// change the walls color
    void setWallsColor(const QColor &color);

    /// change the floor color
    void setFloorColor(const QColor &color);

    /// change the obstacle color
    void setObstacleColor(const QColor &color);

    /// change the exits color.
    void setExitsColor(const QColor &color);

    /// change the exits color.
    void setNavLinesColor(const QColor &color);

    /// show / hide the walls
    void showWalls(bool status);

    /// show/ hide the exits
    void showDoors(bool status);

    /// show/ hide the exits
    void showNavLines(bool status);

    /// show/ hide the floor
    void showFloor(bool status);

    /// show/ hide the obstacles
    void showObstacle(bool status);

    /// show/hide the gradeint field
    void showGradientField(bool status);

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

    void QcolorToDouble(const QColor& col, double* rgb);

    void Create2dAgent();

private:
    //FacilityGeometry* _geometry;
    GeometryFactory _geometry;
    vtkRenderer* _renderer;
    vtkRenderWindow* _renderWindow;
    vtkRenderWindowInteractor* _renderWinInteractor;
    vtkAxesActor* _axis;
    vtkTextActor* _runningTime;
    vtkCamera* _topViewCamera;
    QString _winTitle;

    float _framePerSecond;

};

#endif /* THREADVISUALISATION_H_ */
