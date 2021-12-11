/**
 * @file ThreadDataTransfert.cpp
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

#include "Visualisation.h"

#include "Frame.h"
#include "FrameElement.h"
#include "InteractorStyle.h"
#include "Log.h"
#include "TrajectoryPoint.h"
#include "general/Macros.h"
#include "geometry/FacilityGeometry.h"
#include "geometry/GeometryFactory.h"
#include "geometry/LinePlotter2D.h"
#include "geometry/Point.h"
#include "geometry/PointPlotter.h"

#include <MainWindow.h>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QThread>
#include <vtkActor.h>
#include <vtkActor2DCollection.h>
#include <vtkAssembly.h>
#include <vtkAxesActor.h>
#include <vtkAxisActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDiskSource.h>
#include <vtkFileOutputWindow.h>
#include <vtkFloatArray.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLabeledDataMapper.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkLine.h>
#include <vtkLookupTable.h>
#include <vtkOutputWindow.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkStripper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTriangleFilter.h>
#include <vtkWindowToImageFilter.h>

Visualisation::Visualisation(
    QObject * parent,
    vtkRenderWindow * renderWindow,
    Settings * settings,
    TrajectoryData * trajectories) :
    QObject(parent),
    _settings(settings),
    _trajectories(trajectories),
    _renderWindow(renderWindow),
    _renderer(vtkRenderer::New()),
    _runningTime(vtkTextActor::New())
{
    _renderWindow->AddRenderer(_renderer);
    _winTitle = "header without room caption";
    _renderer->SetBackground(1.0, 1.0, 1.0);
}

Visualisation::~Visualisation() {}

void Visualisation::setFullsreen(bool status) {}

void Visualisation::start()
{
    _renderer->RemoveAllViewProps();
    _renderer->RemoveAllObservers();
    _geometry.Init(_renderer);

    initGlyphs2D();
    initGlyphs3D();

    // create the trails
    _trail_plotter = std::make_unique<PointPlotter>();
    _renderer->AddActor(_trail_plotter->getActor());

    // Create the render window

    // add the running time frame
    _runningTime->SetTextScaleModeToViewport();
    _runningTime->SetVisibility(_settings->showInfos);

    // set the properties of the caption
    vtkTextProperty * tprop = _runningTime->GetTextProperty();
    tprop->SetFontSize(computeFontSize());
    tprop->SetColor(1.0, 0.0, 0.0);

    _renderer->AddActor2D(_runningTime);

    // Create an interactor
    auto * interactor = _renderWindow->GetInteractor();
    vtkNew<InteractorStyle> myStyle;
    myStyle->SetVisualisation(this);
    interactor->SetInteractorStyle(myStyle);

    if(_settings->mode == RenderMode::MODE_2D) {
        _renderer->GetActiveCamera()->OrthogonalizeViewUp();
        _renderer->GetActiveCamera()->ParallelProjectionOn();
    }

    // create a timer for rendering the window
    _timer_cb = vtkCallbackCommand::New();
    _timer_cb->SetCallback(
        [](vtkObject * caller, long unsigned int eventId, void * clientData, void * callData) {
            reinterpret_cast<Visualisation *>(clientData)->onExecute();
        });
    _timer_cb->SetClientData(this);
    if(_trajectories->getFrameCount() > 0) {
        _timer_id = interactor->CreateRepeatingTimer(1000.0 / _trajectories->getFps());
    }
    runningTime = _runningTime;
    interactor->AddObserver(vtkCommand::TimerEvent, _timer_cb);


    // create the necessary connections
    QObject::connect(
        this,
        &Visualisation::signalFrameNumber,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotFrameNumber);

    QObject::connect(
        this,
        &Visualisation::signalMousePositionUpdated,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotMousePositionUpdated);

    QObject::connect(
        this,
        &Visualisation::signalMaxFramesUpdated,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotUpdateNumFrames);

    emit signalMaxFramesUpdated(_trajectories->getFrameCount());

    // save the top view  camera
    _topViewCamera = vtkCamera::New();
    _topViewCamera->DeepCopy(_renderer->GetActiveCamera());

    // update all (restored) system settings
    setGeometryVisibility2D(_settings->mode == RenderMode::MODE_2D);
    setGeometryVisibility3D(_settings->mode == RenderMode::MODE_3D);
    setGeometryVisibility(_settings->showGeometry);
    setOnscreenInformationVisibility(_settings->showInfos);
    showFloor(_settings->showFloor);
    showWalls(_settings->showWalls);
    // showDoors(_settings->showDoors);
    showDoors(true);
    setGeometryLabelsVisibility(_settings->showGeometryCaptions);
    setBackgroundColor(_settings->bgColor);
    setWallsColor(_settings->wallsColor);
    setFloorColor(_settings->floorColor);
    setExitsColor(_settings->exitsColor);
    _renderer->ResetCamera();
    emit signalMaxFramesUpdated(_trajectories->getFrameCount());
    emit signalFrameNumber(0);
    _renderWindow->Render();
}

void Visualisation::stop()
{
    QObject::disconnect(
        this,
        &Visualisation::signalFrameNumber,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotFrameNumber);

    QObject::disconnect(
        this,
        &Visualisation::signalMousePositionUpdated,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotMousePositionUpdated);

    QObject::disconnect(
        this,
        &Visualisation::signalMaxFramesUpdated,
        dynamic_cast<MainWindow *>(this->parent()),
        &MainWindow::slotUpdateNumFrames);

    _trail_plotter.reset();
    _renderWindow->GetInteractor()->DestroyTimer(_timer_id);
    _renderWindow->GetInteractor()->RemoveAllObservers();
}

void Visualisation::update()
{
    auto polyData2D = _trajectories->currentFrame()->GetPolyData2D();
    _glyphs_pedestrians->SetInputData(polyData2D);
    _glyphs_pedestrians->Update();
    _pedestrians_labels->GetMapper()->SetInputDataObject(polyData2D);
    _pedestrians_labels->GetMapper()->Update();
    _glyphs_directions->SetInputData(polyData2D);
    _glyphs_directions->Update();
    auto polyData3D = _trajectories->currentFrame()->GetPolyData3D();
    _glyphs_pedestrians_3D->SetInputData(polyData3D);
    _glyphs_pedestrians_3D->Update();
}
void Visualisation::renderFrame()
{
    // TODO(kkratz): Updating visibility from settings should probably be cached.
    // TODO(kkratz): Updating visibility should be done from Update.
    _glyphs_pedestrians_actor_2D->SetVisibility(_settings->mode == RenderMode::MODE_2D);
    _glyphs_pedestrians_actor_3D->SetVisibility(_settings->mode == RenderMode::MODE_3D);
    _pedestrians_labels->SetVisibility(_settings->showAgentsCaptions);
    _glyphs_directions_actor->SetVisibility(_settings->showAgentDirections);
    _trail_plotter->SetVisibility(_settings->showTrajectories);
    _renderer->GetRenderWindow()->GetInteractor()->Render();
}

void Visualisation::setGeometryVisibility(bool status)
{
    if(_settings->mode == RenderMode::MODE_2D) {
        _geometry.Set2D(status);
    } else {
        _geometry.Set3D(status);
    }
}
void Visualisation::setTrainData(
    std::map<std::string, std::shared_ptr<TrainType>> && trainTypes,
    std::map<int, std::shared_ptr<TrainTimeTable>> && trainTimeTable)
{
    _trainTypes      = trainTypes;
    _trainTimeTables = trainTimeTable;
}

/// show / hide the walls
void Visualisation::showWalls(bool status)
{
    _geometry.ShowWalls(status);
}

/// show/ hide the exits
void Visualisation::showDoors(bool status)
{
    _geometry.ShowDoors(status);
}

void Visualisation::showNavLines(bool status)
{
    _geometry.ShowNavLines(status);
}

void Visualisation::showFloor(bool status)
{
    _geometry.ShowFloor(status);
}

void Visualisation::showObstacle(bool status)
{
    _geometry.ShowObstacles(status);
}
void Visualisation::showGradientField(bool status)
{
    _geometry.ShowGradientField(status);
}
void Visualisation::initGlyphs2D()
{
    _glyphs_pedestrians_actor_2D = vtkActor::New();
    _pedestrians_labels          = vtkActor2D::New();
    _glyphs_directions           = vtkTensorGlyph::New();
    _glyphs_directions_actor     = vtkActor::New();
    _glyphs_pedestrians          = vtkTensorGlyph::New();

    // now create the glyphs with ellipses
    VTK_CREATE(vtkDiskSource, agentShape);
    agentShape->SetCircumferentialResolution(20);
    agentShape->SetInnerRadius(0);
    agentShape->SetOuterRadius(30);

    // speed the rendering using triangles stripers
    VTK_CREATE(vtkTriangleFilter, tris);
    tris->SetInputConnection(agentShape->GetOutputPort());

    VTK_CREATE(vtkStripper, strip);
    strip->SetInputConnection(tris->GetOutputPort());

    _glyphs_pedestrians->SetSourceConnection(strip->GetOutputPort());

    // first frame

    _glyphs_pedestrians->SetInputConnection(agentShape->GetOutputPort());
    _glyphs_pedestrians->ThreeGlyphsOff();
    _glyphs_pedestrians->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(_glyphs_pedestrians->GetOutputPort());

    VTK_CREATE(vtkLookupTable, lut);
    lut->SetHueRange(0.0, 0.470);
    lut->SetValueRange(1.0, 1.0);
    lut->SetNanColor(0.2, 0.2, 0.2, 0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();
    mapper->SetLookupTable(lut);

    _glyphs_pedestrians_actor_2D->SetMapper(mapper);

    _renderer->AddActor(_glyphs_pedestrians_actor_2D);

    // Show directions
    VTK_CREATE(vtkConeSource, agentDirection);
    agentDirection->SetHeight(40);
    agentDirection->SetRadius(15);

    VTK_CREATE(vtkTriangleFilter, tris2);
    tris2->SetInputConnection(agentDirection->GetOutputPort());
    VTK_CREATE(vtkStripper, strip2);
    strip2->SetInputConnection(tris2->GetOutputPort());

    _glyphs_directions->SetSourceConnection(strip2->GetOutputPort());
    _glyphs_directions->SetInputConnection(agentDirection->GetOutputPort());
    _glyphs_directions->ThreeGlyphsOff();
    _glyphs_directions->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper2);
    mapper2->SetInputConnection(_glyphs_directions->GetOutputPort());
    mapper2->ScalarVisibilityOff(); // to set color
    mapper2->SetLookupTable(lut);

    _glyphs_directions_actor->SetMapper(mapper2);
    _glyphs_directions_actor->GetProperty()->SetColor(0, 0, 0); // black
    _renderer->AddActor2D(_glyphs_directions_actor);

    // structure for the labels
    VTK_CREATE(vtkLabeledDataMapper, labelMapper);
    _pedestrians_labels->SetMapper(labelMapper);
    labelMapper->SetFieldDataName("labels");
    labelMapper->SetLabelModeToLabelFieldData();
    _renderer->AddActor2D(_pedestrians_labels);
    _pedestrians_labels->SetVisibility(false);
}

void Visualisation::initGlyphs3D()
{
    _glyphs_pedestrians_3D       = vtkTensorGlyph::New();
    _glyphs_pedestrians_actor_3D = vtkActor::New();

    // now create the glyphs with zylinders
    VTK_CREATE(vtkCylinderSource, agentShape);
    agentShape->SetHeight(160);
    agentShape->SetRadius(20);
    agentShape->SetResolution(20);

    // speed the rendering using triangles stripers
    VTK_CREATE(vtkTriangleFilter, tris);
    tris->SetInputConnection(agentShape->GetOutputPort());

    VTK_CREATE(vtkStripper, strip);
    strip->SetInputConnection(tris->GetOutputPort());

    _glyphs_pedestrians_3D->SetSourceConnection(strip->GetOutputPort());
    _glyphs_pedestrians_3D->SetInputConnection(strip->GetOutputPort());
    _glyphs_pedestrians_3D->ThreeGlyphsOff();
    _glyphs_pedestrians_3D->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(_glyphs_pedestrians_3D->GetOutputPort());

    VTK_CREATE(vtkLookupTable, lut);
    lut->SetHueRange(0.0, 0.470);
    lut->SetValueRange(1.0, 1.0);
    lut->SetNanColor(0.2, 0.2, 0.2, 0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();
    mapper->SetLookupTable(lut);

    _glyphs_pedestrians_actor_3D->SetMapper(mapper);
    _glyphs_pedestrians_actor_3D->GetProperty()->BackfaceCullingOn();
    _renderer->AddActor(_glyphs_pedestrians_actor_3D);

    _glyphs_pedestrians_actor_3D->SetVisibility(false);
}

void Visualisation::init() {}

void Visualisation::finalize() {}

void Visualisation::QcolorToDouble(const QColor & col, double * rgb)
{
    rgb[0] = (double) col.red() / 255.0;
    rgb[1] = (double) col.green() / 255.0;
    rgb[2] = (double) col.blue() / 255.0;
}

void Visualisation::initLegend(/*std::vector scalars*/)
{
    // lookup table
    vtkLookupTable * lut = vtkLookupTable::New();
    lut->SetHueRange(0.0, 0.566);
    lut->SetTableRange(20.0, 50.0);
    lut->SetNumberOfTableValues(50);
    lut->Build();

    vtkTextProperty * titleProp = vtkTextProperty::New();
    titleProp->SetFontSize(14);

    vtkTextProperty * labelProp = vtkTextProperty::New();
    labelProp->SetFontSize(10);

    vtkScalarBarActor * scalarBar = vtkScalarBarActor::New();
    scalarBar->SetLookupTable(lut);
    scalarBar->SetTitle("Velocities ( cm/s )");
    scalarBar->SetTitleTextProperty(titleProp);
    scalarBar->SetLabelTextProperty(labelProp);
    scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    scalarBar->GetPositionCoordinate()->SetValue(0.005, 0.90075);
    scalarBar->SetOrientationToVertical();
    scalarBar->SetNumberOfLabels(7);
    scalarBar->SetMaximumNumberOfColors(20);
    scalarBar->SetWidth(0.105);
    scalarBar->SetHeight(0.10);
    _renderer->AddActor2D(scalarBar);
    _renderer->Render();
}


void Visualisation::setAxisVisible(bool status)
{
    _axis->SetVisibility(status);
}

void Visualisation::setCameraPerspective(int mode, int degree)
{
    if(_renderer == NULL)
        return;

    switch(mode) {
        case 1: // TOP oder RESET
            _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
            break;
        case 2: // TOP Rotate [range:-180-->180]
            _topViewCamera->Roll(degree);
            _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
            _topViewCamera->Roll(-degree);
            break;
        case 3: // Side Rotate [range:-80-->80]
            _topViewCamera->Elevation(-degree);
            _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
            _topViewCamera->Elevation(degree);
            break;
        case 4: { // agent virtual reality
        }

        break;
    }
    _renderer->ResetCamera();
}

void Visualisation::setBackgroundColor(const QColor & col)
{
    double bgcolor[3];
    QcolorToDouble(col, bgcolor);
    if(_renderer != NULL)
        _renderer->SetBackground(bgcolor);
}

void Visualisation::setWindowTitle(QString title)
{
    if(title.isEmpty())
        return;
    _winTitle = title;
}

/// @todo check this construct
void Visualisation::setGeometry(FacilityGeometry * geometry)
{
    cout << "dont call me" << endl;
    exit(0);
}

GeometryFactory & Visualisation::getGeometry()
{
    return _geometry;
}

void Visualisation::setWallsColor(const QColor & color)
{
    double rbgColor[3];
    QcolorToDouble(color, rbgColor);
    _geometry.ChangeWallsColor(rbgColor);
}

void Visualisation::setFloorColor(const QColor & color)
{
    double rbgColor[3];
    QcolorToDouble(color, rbgColor);
    _geometry.ChangeFloorColor(rbgColor);
}

void Visualisation::setObstacleColor(const QColor & color)
{
    double rbgColor[3];
    QcolorToDouble(color, rbgColor);
    _geometry.ChangeObstaclesColor(rbgColor);
}

void Visualisation::setGeometryLabelsVisibility(int v)
{
    _geometry.ShowGeometryLabels(v);
}

void Visualisation::setExitsColor(const QColor & color)
{
    double rbgColor[3];
    QcolorToDouble(color, rbgColor);
    // HH
    _geometry.ChangeExitsColor(rbgColor);
}

void Visualisation::setNavLinesColor(const QColor & color)
{
    double rbgColor[3];
    QcolorToDouble(color, rbgColor);
    _geometry.ChangeNavLinesColor(rbgColor);
}

/// enable/disable 2D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void Visualisation::setGeometryVisibility2D(bool status)
{
    _geometry.Set2D(status);
}

/// enable/disable 3D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void Visualisation::setGeometryVisibility3D(bool status)
{
    _geometry.Set3D(status);
}

void Visualisation::setOnscreenInformationVisibility(bool show)
{
    _runningTime->SetVisibility(show);
}

vtkSmartPointer<vtkPolyData>
getTrainData(Point trainStart, Point trainEnd, std::vector<Point> doorPoints, double elevation)

{
    float factor = 100.0;

    double pt[3] = {1.0, 0.0, 0.0}; // to convert from Point
    // Create the polydata where we will store all the geometric data
    vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();

    // Create a vtkPoints container and store the points in it
    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();

    pt[0] = factor * trainStart._x;
    pt[1] = factor * trainStart._y;
    pt[2] = factor * elevation;
    pts->InsertNextPoint(pt);

    for(auto p : doorPoints) {
        pt[0] = factor * p._x;
        pt[1] = factor * p._y;
        pt[2] = factor * elevation;
        pts->InsertNextPoint(pt);
    }
    pt[0] = factor * trainEnd._x;
    pt[1] = factor * trainEnd._y;
    pt[2] = factor * elevation;
    pts->InsertNextPoint(pt);


    // Add the points to the polydata container
    linesPolyData->SetPoints(pts);


    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();


    // Create the first line (between Origin and P0)
    for(int i = 0; i <= doorPoints.size(); i += 2) {
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, i);
        line->GetPointIds()->SetId(1, i + 1);

        lines->InsertNextCell(line);
        lines->InsertNextCell(line);
        line = nullptr;
    }

    // Add the lines to the polydata container
    linesPolyData->SetLines(lines);
    return linesPolyData;
}

void Visualisation::onExecute()
{
    vtkRenderWindowInteractor * const iren = _renderWindow->GetInteractor();

    int frameNumber         = 0;
    int nPeds               = 0;
    static bool isRecording = false;


    if(_trajectories->getFrameCount() > 0) {
        if(!is_pause) {
            _trajectories->moveFrameBy(_replay_speed);
        }
        const auto * frame = _trajectories->currentFrame();

        nPeds = frame->Size();
        update();
        auto FrameElements = frame->GetFrameElements();

        if(_settings->showTrajectories) {
            const auto & elements = frame->GetFrameElements();
            for(auto && e : elements) {
                _trail_plotter->PlotPoint(e.pos, e.color);
            }
        }
    }

    frameNumber = _trajectories->currentIndex();
    emit signalFrameNumber(frameNumber);

    double now      = frameNumber * iren->GetTimerDuration(_timer_id) / 1000;
    int countTrains = 0;
    char label[100];

    for(auto tab : _trainTimeTables) {
        VTK_CREATE(vtkTextActor3D, textActor);
        auto trainType = tab.second->type;
        sprintf(label, "%s_%d", trainType.c_str(), tab.second->id);
        auto trainId      = tab.second->id;
        auto trackStart   = tab.second->pstart;
        auto trackEnd     = tab.second->pend;
        auto trainOffset  = tab.second->train_offset;
        auto reversed     = tab.second->reversed;
        auto train        = _trainTypes[trainType];
        auto train_length = train->_length;
        auto doors        = train->_doors;
        std::vector<Point> doorPoints;
        auto mapper         = tab.second->mapper;
        auto actor          = tab.second->actor;
        double elevation    = tab.second->elevation;
        auto txtActor       = tab.second->textActor;
        auto trackDirection = (reversed) ? (trackStart - trackEnd) : (trackEnd - trackStart);
        trackDirection      = trackDirection.Normalized();
        auto trainStart     = (reversed) ? trackEnd + trackDirection * trainOffset :
                                           trackStart + trackDirection * trainOffset;
        auto trainEnd = (reversed) ? trackEnd + trackDirection * (trainOffset + train_length) :
                                     trackStart + trackDirection * (trainOffset + train_length);

        for(auto door : doors) {
            Point trainDirection = trainEnd - trainStart;
            ;
            trainDirection = trainDirection.Normalized();
            Point point1   = trainStart + trainDirection * (door._distance);
            Point point2   = trainStart + trainDirection * (door._distance + door._width);
            doorPoints.push_back(point1);
            doorPoints.push_back(point2);
        } // doors
        // FIXME(kkratz): This is broken!
        static int once = 1;
        if(once) {
            auto data = getTrainData(trainStart, trainEnd, doorPoints, elevation);
            mapper->SetInputData(data);
            actor->SetMapper(mapper);
            actor->GetProperty()->SetLineWidth(10);
            actor->GetProperty()->SetOpacity(0.1); // feels cool!
            if(trainType == "RE") {
                actor->GetProperty()->SetColor(
                    std::abs(0.0 - _renderer->GetBackground()[0]),
                    std::abs(1.0 - _renderer->GetBackground()[1]),
                    std::abs(1.0 - _renderer->GetBackground()[2]));
            } else {
                actor->GetProperty()->SetColor(
                    std::abs(0.9 - _renderer->GetBackground()[0]),
                    std::abs(0.9 - _renderer->GetBackground()[1]),
                    std::abs(1.0 - _renderer->GetBackground()[2]));
            }
            // text
            txtActor->GetTextProperty()->SetOpacity(0.7);
            double pos_x = 50 * (trainStart._x + trainEnd._x + 0.5);
            double pos_y = 50 * (trainStart._y + trainEnd._y + 0.5);

            txtActor->SetPosition(pos_x, pos_y + 2, 20);
            txtActor->SetInput(label);
            txtActor->GetTextProperty()->SetFontSize(30);
            txtActor->GetTextProperty()->SetBold(true);
            if(trainType == "RE") {
                txtActor->GetTextProperty()->SetColor(
                    std::abs(0.0 - _renderer->GetBackground()[0]),
                    std::abs(1.0 - _renderer->GetBackground()[1]),
                    std::abs(1.0 - _renderer->GetBackground()[2]));
            } else {
                txtActor->GetTextProperty()->SetColor(
                    std::abs(0.9 - _renderer->GetBackground()[0]),
                    std::abs(0.9 - _renderer->GetBackground()[1]),
                    std::abs(0.5 - _renderer->GetBackground()[2]));
            }
            txtActor->SetVisibility(false);
        }
        if((now >= tab.second->tin) && (now <= tab.second->tout)) {
            actor->SetVisibility(true);
            txtActor->SetVisibility(true);
        } else {
            actor->SetVisibility(false);
            txtActor->SetVisibility(false);
        }
        if(once) {
            _renderer->AddActor(actor);
            _renderer->AddActor(txtActor);
            if(countTrains == _trainTimeTables.size())
                once = 0;
        }

        countTrains++;
    } // time table


    int * winSize       = _renderWindow->GetSize();
    static int lastWinX = winSize[0] + 1; // +1 to trigger a first change
    static int lastWinY = winSize[1];
    sprintf(
        runningTimeText,
        "Pedestrians: %d      Time: %ld Sec",
        nPeds,
        frameNumber * iren->GetTimerDuration(_timer_id) / 1000);
    runningTime->SetInput(runningTimeText);
    runningTime->Modified();

    if((lastWinX != winSize[0]) || (lastWinY != winSize[1]) /*|| (frameNumber<10)*/) {
        static std::string winBaseName(_renderWindow->GetWindowName());
        std::string winName = winBaseName;
        std::string s;
        winName.append(" [ ");
        s = QString::number(winSize[0]).toStdString();
        winName.append(s);
        winName.append(" x ");
        s = QString::number(winSize[1]).toStdString();
        winName.append(s);
        winName.append(" ] -->");

        int posY = winSize[1] * (1.0 - 30.0 / 536.0);
        int posX = winSize[0] * (1.0 - 450.0 / 720.0);
        runningTime->SetPosition(posX, posY);
        _renderWindow->SetWindowName(winName.c_str());

        lastWinX = winSize[0];
        lastWinY = winSize[1];
    }

    renderFrame();

    if(_settings->recordPNGsequence) {
        takeScreenshotSequence();
    }
}

void Visualisation::onMouseMove(double x, double y, double z)
{
    emit signalMousePositionUpdated(x, y, z);
}

void Visualisation::takeScreenshot()
{
    static int imageID                     = 0;
    vtkWindowToImageFilter * winToImFilter = vtkWindowToImageFilter::New();
    winToImFilter->SetInput(_renderWindow);
    vtkPNGWriter * image = vtkPNGWriter::New();
    image->SetInputConnection(winToImFilter->GetOutputPort());
    winToImFilter->Delete();

    QString screenshots = QDir::homePath() + QDir::separator() + "Desktop" + QDir::separator() +
                          "JPSvis_Files" + QDir::separator();
    // create directory if not exits
    if(!QDir(screenshots).exists()) {
        QDir dir;
        if(!dir.mkpath(screenshots)) {
            // try with the current directory
            screenshots = "";
        }
    }


    char filename[256] = {0};
    std::string date   = QString(QDateTime::currentDateTime().toString("yyMMdd_hh")).toStdString();

    sprintf(filename, "travisto_snap_%sh_%d.png", date.c_str(), imageID++);

    // append the prefix
    screenshots += _settings->filesPrefix;
    screenshots += QString(filename);
    image->SetFileName(screenshots.toStdString().c_str());
    winToImFilter->Modified();

    image->Write();
    image->Delete();
}

int Visualisation::computeFontSize()
{
    const double defaultDpi       = 96.0;
    const double desiredFontSize  = 8.0;
    const auto desiredScaleFactor = defaultDpi / desiredFontSize;
    const auto dpi                = _renderWindow->GetDPI();
    return dpi / desiredScaleFactor;
}

/// take png screenshot sequence
void Visualisation::takeScreenshotSequence()
{
    static int imageID                     = 0;
    vtkWindowToImageFilter * winToImFilter = vtkWindowToImageFilter::New();
    winToImFilter->SetInput(_renderWindow);
    vtkPNGWriter * image = vtkPNGWriter::New();
    image->SetInputConnection(winToImFilter->GetOutputPort());
    winToImFilter->Delete();

    QString screenshots = QDir::homePath() + QDir::separator() + "Desktop" + QDir::separator() +
                          "JPSvis_Files" + QDir::separator();

    screenshots.append(
        "./png_seq_" + QDateTime::currentDateTime().toString("yyMMddhh") + "_" +
        _settings->filesPrefix);
    screenshots.truncate(screenshots.size() - 1);

    // create directory if not exits
    if(!QDir(screenshots).exists()) {
        QDir dir;
        if(!dir.mkpath(screenshots)) {
            cerr << "could not create directory: " << screenshots.toStdString();
            // try with the current directory
            screenshots = "./png_seq_" + QDateTime::currentDateTime().toString("yyMMdd") + "_" +
                          _settings->filesPrefix;
            screenshots.truncate(screenshots.size() - 1);
        }
    }


    char filename[30] = {0};
    sprintf(filename, "/tmp_%07d.png", imageID++);
    screenshots.append(filename);
    image->SetFileName(screenshots.toStdString().c_str());
    winToImFilter->Modified();

    image->Write();
    image->Delete();
}

double Visualisation::trajectoryRecordingFps() const
{
    return _trajectories->getFps();
}

void Visualisation::ChangeReplaySpeed(int frames)
{
    _replay_speed = frames;
}

void Visualisation::pauseRendering(bool paused)
{
    is_pause = paused;
}
