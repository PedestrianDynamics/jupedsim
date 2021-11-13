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
#pragma once

#include "InteractorStyle.h"
#include "Settings.h"
#include "TrajectoryData.h"
#include "geometry/GeometryFactory.h"
#include "geometry/PointPlotter.h"
#include "trains/train.h"

#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QThread>
#include <vtkGlyph3D.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTensorGlyph.h>

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
class TrajectoryData;
class FacilityGeometry;
class TrailPlotter;
class PointPlotter;


class Visualisation : public QObject
{
    Q_OBJECT

public:
    Visualisation(
        QObject * parent,
        vtkRenderWindow * renderWindow,
        Settings * _settings,
        TrajectoryData * trajectories);
    virtual ~Visualisation();
    void start();
    void stop();
    void pauseRendering(bool paused);

    void update();
    void renderFrame();

    void setAxisVisible(bool status);

    void setFullsreen(bool status);

    /// set the camera to one of TOP/FRONT/SIDE
    void setCameraPerspective(int mode, int degree = 0);

    /// load and display the geometry where
    /// the pedestrians will move
    void setGeometry(FacilityGeometry * _geometry);

    GeometryFactory & getGeometry();

    /// this is for convenience and will be delete later
    void setWindowTitle(QString title);

    /// shutdown the thread
    // void shutdown();

    void setGeometryLabelsVisibility(int v);

    /// set geometry visibility
    void setGeometryVisibility(bool status);

    void setTrainData(
        std::map<std::string, std::shared_ptr<TrainType>> && trainTypes,
        std::map<int, std::shared_ptr<TrainTimeTable>> && trainTimeTable);

    /// enable/disable 2D
    void setGeometryVisibility2D(bool status);

    /// enable/disable 3D
    void setGeometryVisibility3D(bool status);

    /// change the background color of the rendering windows
    void setBackgroundColor(const QColor & col);

    /// change the walls color
    void setWallsColor(const QColor & color);

    /// change the floor color
    void setFloorColor(const QColor & color);

    /// change the obstacle color
    void setObstacleColor(const QColor & color);

    /// change the exits color.
    void setExitsColor(const QColor & color);

    /// change the exits color.
    void setNavLinesColor(const QColor & color);

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
    /// not implemented
    void showStairs(bool status);

    void setOnscreenInformationVisibility(bool show);

    void onExecute();

    void onMouseMove(double x, double y, double z);

    /// make a png screenshot of the renderwindows
    void takeScreenshot();

    /// Returns the fps the trajectory data was recorded.
    /// If no trajectory data was loaded, i.e. only geometry is rendered the will return 0
    /// @return fps of the trajectory data.
    double trajectoryRecordingFps() const;

    /// Changes the replay speed, i.e. the numer of frames advaned per render call.
    /// A negative number will make the animation play backwards.
    /// @param frames to move per iteration.
    void ChangeReplaySpeed(int frames);

    /// Returns the current replay speed in flames. The number may be negative to indicate the
    /// replay running backwards.
    /// @return reply_speed in frames per iteration
    int replaySpeed() const { return _replay_speed; }

signals:
    void signalFrameNumber(int frame);
    void signalMaxFramesUpdated(int num_frames);
    void signalMousePositionUpdated(double x, double y, double z);

private:
    /// initialize the legend
    void initLegend(/*std::vector scalars*/);

    /// initialize the datasets
    void init();

    /// initialize the datasets
    void initGlyphs2D();

    // initialize the 3D agents
    void initGlyphs3D();

    // finalize the datasets
    void finalize();

    void QcolorToDouble(const QColor & col, double * rgb);

    /// take png screenshots sequence
    void takeScreenshotSequence();


private:
    Settings * _settings;
    TrajectoryData * _trajectories;
    GeometryFactory _geometry;
    std::map<std::string, std::shared_ptr<TrainType>> _trainTypes;
    std::map<int, std::shared_ptr<TrainTimeTable>> _trainTimeTables;
    vtkRenderWindow * _renderWindow;
    vtkSmartPointer<vtkRenderer> _renderer;
    vtkSmartPointer<vtkAxesActor> _axis;
    vtkSmartPointer<vtkTextActor> _runningTime;
    vtkSmartPointer<vtkCamera> _topViewCamera;
    vtkSmartPointer<vtkTensorGlyph> _glyphs_pedestrians;
    vtkSmartPointer<vtkActor> _glyphs_directions_actor;
    vtkSmartPointer<vtkActor> _glyphs_pedestrians_actor_2D;
    vtkSmartPointer<vtkActor2D> _pedestrians_labels;
    vtkSmartPointer<vtkTensorGlyph> _glyphs_directions;
    vtkSmartPointer<vtkTensorGlyph> _glyphs_pedestrians_3D;
    vtkSmartPointer<vtkActor> _glyphs_pedestrians_actor_3D;
    vtkSmartPointer<vtkTextActor> runningTime{};
    char runningTimeText[50] = {};
    QString _winTitle;
    vtkSmartPointer<vtkCallbackCommand> _timer_cb;
    int _timer_id = 1;
    std::unique_ptr<PointPlotter> _trail_plotter{nullptr};
    bool is_pause{true};
    int _replay_speed{1};
};
