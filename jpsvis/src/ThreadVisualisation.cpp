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


#include <QThread>
#include <QString>
#include <QObject>
#include <QMessageBox>

// we don't need this when compiling with cmake
// http://www.vtk.org/Wiki/VTK/VTK_6_Migration/Factories_now_require_defines
// all modules need to be initialized in vtk6
//#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
//#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)


#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkAssembly.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAxisActor.h>
#include <vtkAxesActor.h>
#include <vtkTextActor.h>
#include <vtkLight.h>
#include <vtkScalarBarActor.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkActor2DCollection.h>
#include <vtkRegularPolygonSource.h>
#include <vtkLabeledDataMapper.h>
#include <vtkDiskSource.h>
#include <vtkConeSource.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkActor.h>
#include <vtkLightKit.h>
#include <vtkPolyLine.h>

#include "geometry/FacilityGeometry.h"
#include "geometry/GeometryFactory.h"
#include "geometry/LinePlotter2D.h"

#include "ThreadVisualisation.h"
#include "Pedestrian.h"
#include "TimerCallback.h"
#include "Frame.h"
#include "TrajectoryPoint.h"
#include "SyncData.h"
#include "InteractorStyle.h"
#include "SystemSettings.h"
#include "geometry/PointPlotter.h"
#include "Log.h"


#define VTK_CREATE(type, name) \
    vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


ThreadVisualisation::ThreadVisualisation(QObject *parent):
    QThread(parent)
{
    _renderer=NULL;
    _renderWindow=NULL;
    _renderWinInteractor=NULL;
    _runningTime=vtkTextActor::New();;
    _framePerSecond=25;
    _axis=NULL;
    _winTitle="header without room caption";
    //_geometry=new FacilityGeometry();
}

ThreadVisualisation::~ThreadVisualisation()
{

    if(_axis)
        _axis->Delete();

    if(extern_glyphs_pedestrians_3D) extern_glyphs_pedestrians_3D->Delete();
    if(extern_glyphs_pedestrians_actor_3D) extern_glyphs_pedestrians_actor_3D->Delete();
    if(extern_glyphs_pedestrians) extern_glyphs_pedestrians->Delete();
    if(extern_glyphs_pedestrians_actor_2D) extern_glyphs_pedestrians_actor_2D->Delete();
    if(extern_pedestrians_labels) extern_pedestrians_labels->Delete();
    //show directions of movement
    if(extern_glyphs_directions) extern_glyphs_directions->Delete();
    if(extern_glyphs_directions_actor) extern_glyphs_directions_actor->Delete();

    _runningTime->Delete();

}

void ThreadVisualisation::setFullsreen(bool status)
{
    _renderWindow->SetFullScreen(status);
}

void ThreadVisualisation::slotSetFrameRate(float fps)
{
    _framePerSecond=fps;
}


void ThreadVisualisation::run()
{
    //deactivate the output windows
    vtkObject::GlobalWarningDisplayOff();

    //emit signalStatusMessage("running");


    // Create the renderer
    _renderer = vtkRenderer::New();
    // set the background
    //renderer->SetBackground(.00,.00,.00);
    _renderer->SetBackground(1.0,1.0,1.0);
    //add the geometry
    _geometry.Init(_renderer);
    //_geometry->CreateActors();
    //_renderer->AddActor(_geometry->getActor2D());
    //_renderer->AddActor(_geometry->getActor3D());

    initGlyphs2D();
    initGlyphs3D();

    //create the trails
    extern_trail_plotter = new PointPlotter();
    _renderer->AddActor(extern_trail_plotter->getActor());

    // add axis
    //axis= vtkAxesActor::New();
    //axis->SetScale(10);
    //axis->SetConeRadius(30);
    //axis->SetXAxisLabelText("1 Meter");
    //axis->SetYAxisLabelText("1 Meter");
    //axis->SetZAxisLabelText("1 Meter");
    //axis->AxisLabelsOff();
    //axis->SetCylinderRadius(100);
    //axis->set
    //axis->SetTotalLength(1000,1000,1000);
    //axis->SetVisibility(true);
    //renderer->AddActor(axis);

    //add big circle at null point
    // {
    //      vtkSphereSource* org = vtkSphereSource::New();
    //     org->SetRadius(10);
    //     vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    //     mapper->SetInputConnection(org->GetOutputPort());
    //     org->Delete();
    //     //------
    //     // create actor
    //     vtkActor* actor = vtkActor::New();
    //     actor->SetMapper(mapper);
    //     mapper->Delete();
    //     actor->GetProperty()->SetColor(.1,.10,0.0);
    //     _renderer->AddActor(actor);
    //     actor->Delete();
    //      mysphere;
    //     //
    // }
    // {
    //     vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
    //     mapper->SetInputConnection(extern_mysphere->GetOutputPort());
    //     //------
    //     // create actor
    //     vtkActor* actor = vtkActor::New();
    //     actor->SetMapper(mapper);
    //     mapper->Delete();
    //     actor->GetProperty()->SetColor(.1,.10,0.0);
    //     _renderer->AddActor(actor);
    //     actor->Delete();
    //     //
    // }

    // {
    //      // Train
    //      double p0[3] = {1.0*100, 4.0*100, 0.0};
    //      double p1[3] = {3.0*100, 4.0*100, 0.0};
    //      double p2[3] = {3.0*100, 5.0*100, 0.0};
    //      double p3[3] = {1.0*100, 5.0*100, 0.0};
    //      double p4[3] = {1.0*100, 4.0*100, 0.0};
    //      // Create a vtkPoints object and store the points in it
    //      VTK_CREATE(vtkPoints, points);

    //      points->InsertNextPoint(p0);
    //      points->InsertNextPoint(p1);
    //      points->InsertNextPoint(p2);
    //      points->InsertNextPoint(p3);
    //      points->InsertNextPoint(p4);

    //      VTK_CREATE(vtkPolyLine, polyLine);

    //      polyLine->GetPointIds()->SetNumberOfIds(5);
    //      for(unsigned int i = 0; i < 5; i++)
    //      {
    //           polyLine->GetPointIds()->SetId(i,i);
    //      }

    //      // Create a cell array to store the lines in and add the lines to it
    //      VTK_CREATE(vtkCellArray, cells);
    //      cells->InsertNextCell(polyLine);

    //      // Create a polydata to store everything in
    //      VTK_CREATE(vtkPolyData, polyData);

    //      // Add the points to the dataset
    //      polyData->SetPoints(points);

    //      // Add the lines to the dataset
    //      polyData->SetLines(cells);

    //      VTK_CREATE(vtkPolyDataMapper, mapper);
    //      mapper->SetInputData(polyData);


    //      VTK_CREATE(vtkActor, actor);
    //      actor->SetMapper(mapper);
    //      actor->GetProperty()->SetColor(.1,.10,0.0);
    //     _renderer->AddActor(actor);
    // }

    // Create the render window
    _renderWindow = vtkRenderWindow::New();
    _renderWindow->AddRenderer( _renderer );
    _renderWindow->SetSize(960, 800);
    //renderWindow->SetSize(640, 480);

    // add the legend
    //if(SystemSettings::getShowLegend())
    //	initLegend();

    //add the running time frame
    _runningTime->SetTextScaleModeToViewport();
    //runningTime->SetTextScaleModeToProp();
    //runningTime->SetMinimumSize(10,10);
    //runningTime->SetMinimumSize(180,80);
    //runningTime->SetInput(txt);
    //runningTime->SetDisplayPosition(500,700);
    _runningTime->SetVisibility(SystemSettings::getOnScreenInfos());

    // set the properties of the caption
    vtkTextProperty* tprop = _runningTime->GetTextProperty();
    //tprop->SetFontFamilyToArial();
    //tprop->BoldOn();
    //tprop->SetLineSpacing(1.0);
    tprop->SetFontSize(10);
    tprop->SetColor(1.0,0.0,0.0);

    _renderer->AddActor2D(_runningTime);

    //CAUTION: this is necessary for WIN32 to update the window name
    // but his will freeze your system on linux
#if defined(_WIN32) || defined(__APPLE__)
    _renderWindow->Render();
#endif

    _renderWindow->SetWindowName(_winTitle.toStdString().c_str());

    // Create an interactor
    _renderWinInteractor = vtkRenderWindowInteractor::New();
    _renderWindow->SetInteractor( _renderWinInteractor );
    _renderWinInteractor->Initialize();

    //add a light kit
    {
        //VTK_CREATE(vtkLightKit, lightKit);
        //lightKit->SetKeyLightIntensity(1);

        //lightKit->SetKeyLightWarmth(5);
        //lightKit->SetBackLightAngle(30,40);
        //lightKit->SetKeyLightWarmth(2.0);
        //lightKit->SetMaintainLuminance(true);
        //lightKit->AddLightsToRenderer(renderer);
    }


    if(SystemSettings::get2D()) {
        _renderer->GetActiveCamera()->OrthogonalizeViewUp();
        _renderer->GetActiveCamera()->ParallelProjectionOn();
        _renderer->ResetCamera();
    }

    //create a timer for rendering the window
    TimerCallback *renderingTimer = new TimerCallback();
    int timer= _renderWinInteractor->CreateRepeatingTimer(1000.0/_framePerSecond);
    renderingTimer->SetRenderTimerId(timer);
    renderingTimer->setTextActor(_runningTime);
    _renderWinInteractor->AddObserver(vtkCommand::TimerEvent,renderingTimer);





    //create the necessary connections
    QObject::connect(renderingTimer, SIGNAL(signalRunningTime(unsigned long )),
                     this->parent(), SLOT(slotRunningTime(unsigned long )));

    QObject::connect(renderingTimer, SIGNAL(signalFrameNumber(unsigned long, unsigned long )),
                     this->parent(), SLOT(slotFrameNumber(unsigned long, unsigned long )));

    QObject::connect(renderingTimer, SIGNAL(signalRenderingTime(int)),
                     this->parent(), SLOT(slotRenderingTime(int)));

    // Create my interactor style
    InteractorStyle* style = InteractorStyle::New();
    _renderWinInteractor->SetInteractorStyle( style );
    style->Delete();


    //create special camera for the virtual reality feeling
    //renderer->GetActiveCamera()->SetRoll(90);
    //renderer->GetRenderers()->GetFirstRenderer()->GetActiveCamera();

    //Pedestrian::setCamera(renderer->GetActiveCamera());
    //renderer->ResetCamera();

    //renderWinInteractor->Initialize();
    // Initialize and enter interactive mode
    // should be called after the observer has been added
    //renderWindow->Modified();

    //style->SetKeyPressActivationValue('R');
    //style->SetKeyPressActivation(true);
    //renderWindow->SetCurrentCursor(VTK_CURSOR_CROSSHAIR);

    //save the top view  camera
    _topViewCamera=vtkCamera::New();
    //renderer->GetActiveCamera()->Modified();
    _topViewCamera->DeepCopy(_renderer->GetActiveCamera());

    //update all (restored) system settings
    setGeometryVisibility2D(SystemSettings::get2D());
    setGeometryVisibility3D(!SystemSettings::get2D());
    setGeometryVisibility(SystemSettings::getShowGeometry());
    setOnscreenInformationVisibility(SystemSettings::getOnScreenInfos());
    showFloor(SystemSettings::getShowFloor());
    showWalls(SystemSettings::getShowWalls());
    showObstacle(SystemSettings::getShowObstacles());
    showDoors(SystemSettings::getShowExits());
    showNavLines(SystemSettings::getShowNavLines());
    setGeometryLabelsVisibility(SystemSettings::getShowGeometryCaptions());
    setBackgroundColor(SystemSettings::getBackgroundColor());
    setWallsColor(SystemSettings::getWallsColor());
    setObstacleColor(SystemSettings::getObstacleColor());
    setFloorColor(SystemSettings::getFloorColor());
    setExitsColor(SystemSettings::getExitsColor());
    setNavLinesColor(SystemSettings::getNavLinesColor());
    //FIXME:
    showGradientField(SystemSettings::getShowGradientField());
    _renderWinInteractor->Start();


#ifdef __APPLE__
    //InitMultiThreading();

    //dispatch_async(main_q, ^(void){
    //          is_main_thread(); //Unfortunately not
    //          std::cout << "now spinning the visualizer" << std::endl;
    //                  renderWinInteractor->Start();

    //});
    //[[NSThread new] start];
    //#include <objc/objc.h>
    //NSThread* myThread = [[NSThread alloc] initWithTarget:self selector:@selector(workerThreadFunction:) object:nil];
    //[myThread start];
#endif
    //emit signalStatusMessage("Idle");
    emit signal_controlSequences("CONTROL_RESET");


    // still debugging. TODO, check the source of the leak while using cocoa
#ifndef __APPLE__
    //clear some stuffs
    //delete extern_trail_plotter;
    finalize();

    _renderer->Delete();
    _renderWindow->Delete();
    _renderWinInteractor->Delete();
    _topViewCamera->Delete();
    _renderer=NULL;
    delete renderingTimer;
#endif

}


void ThreadVisualisation::slotControlSequence(const char* para)
{

    //cout <<"control sequence received: " <<sex<<endl;
}

void ThreadVisualisation::setGeometryVisibility( bool status)
{

    if(SystemSettings::get2D())
    {
        _geometry.Set2D(status);
    } else
    {
       _geometry.Set3D(status);
    }
}

/// show / hide the walls
void ThreadVisualisation::showWalls(bool status)
{
    _geometry.ShowWalls(status);
}

/// show/ hide the exits
void ThreadVisualisation::showDoors(bool status)
{
    _geometry.ShowDoors(status);
}

void ThreadVisualisation::showNavLines(bool status)
{
    _geometry.ShowNavLines(status);
}

void ThreadVisualisation::showFloor(bool status)
{
    _geometry.ShowFloor(status);
}

void ThreadVisualisation::showObstacle(bool status)
{
    _geometry.ShowObstacles(status);
}
void ThreadVisualisation::showGradientField(bool status)
{
    _geometry.ShowGradientField(status);
}
void  ThreadVisualisation::initGlyphs2D()
{
    if(extern_glyphs_pedestrians) extern_glyphs_pedestrians->Delete();
    if(extern_glyphs_pedestrians_actor_2D) extern_glyphs_pedestrians_actor_2D->Delete();
    if(extern_pedestrians_labels) extern_pedestrians_labels->Delete();
    //show directions of movement
    if(extern_glyphs_directions) extern_glyphs_directions->Delete();
    if(extern_glyphs_directions_actor) extern_glyphs_directions_actor->Delete();

    extern_glyphs_pedestrians = vtkTensorGlyph::New();
    extern_glyphs_pedestrians_actor_2D = vtkActor::New();
    extern_pedestrians_labels =  vtkActor2D::New();
    extern_glyphs_directions=vtkTensorGlyph::New();
    extern_glyphs_directions_actor=vtkActor::New();

    //glyphs with ellipsoids
    //    VTK_CREATE (vtkSphereSource, agentShape);
    //    agentShape->SetRadius(30);
    //    agentShape->SetPhiResolution(20);
    //    agentShape->SetThetaResolution(20);

    //now create the glyphs with ellipses
    VTK_CREATE (vtkDiskSource, agentShape);
    agentShape->SetCircumferentialResolution(20);
    agentShape->SetInnerRadius(0);
    agentShape->SetOuterRadius(30);

    //    {
    //        //personal space
    //        VTK_CREATE (vtkDiskSource, perSpace);
    //        perSpace->SetCircumferentialResolution(20);
    //        perSpace->SetInnerRadius(0);
    //        perSpace->SetOuterRadius(30);
    //        //forehead
    //        perSpace->SetCircumferentialResolution(20);
    //        perSpace->SetInnerRadius(0);
    //        perSpace->SetOuterRadius(30);
    //        //backhead

    //        //Append the two meshes
    //        VTK_CREATE (vtkAppendPolyData, appendFilter);
    //#if VTK_MAJOR_VERSION <= 5
    //        appendFilter->AddInputConnection(perSpace->GetProducerPort());
    //        appendFilter->AddInputConnection(input2->GetProducerPort());
    //#else
    //        appendFilter->AddInputData(perSpace);
    //        appendFilter->AddInputData(input2);
    //#endif
    //        appendFilter->Update();

    //        // Remove any duplicate points.
    //        VTK_CREATE (vtkCleanPolyData, cleanFilter);
    //        cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    //        cleanFilter->Update();
    //    }

    //speed the rendering using triangles stripers
    //vtkTriangleFilter *tris = vtkTriangleFilter::New();
    VTK_CREATE(vtkTriangleFilter, tris);
    tris->SetInputConnection(agentShape->GetOutputPort());
    //tris->GetOutput()->ReleaseData();

    //vtkStripper *strip = vtkStripper::New();
    VTK_CREATE(vtkStripper, strip);
    strip->SetInputConnection(tris->GetOutputPort());
    //strip->GetOutput()->ReleaseData();

    extern_glyphs_pedestrians->SetSourceConnection(strip->GetOutputPort());
    //extern_glyphs_pedestrians->SetSourceConnection(agentShape->GetOutputPort());

    //first frame
    auto&& frames=extern_trajectories_firstSet.GetFrames();
    if(frames.empty()) return;

    Frame * frame = frames.begin()->second;
//    std::cout << "FRAME " << frames.begin()->first << "\n";
    vtkPolyData* pData=NULL;

    if(frame) pData=frame->GetPolyData2D();
    
    extern_glyphs_pedestrians->SetInputConnection(agentShape->GetOutputPort());
    if (frame) extern_glyphs_pedestrians->SetInputData(pData);
    extern_glyphs_pedestrians->ThreeGlyphsOff();
    extern_glyphs_pedestrians->ExtractEigenvaluesOff();
    //_agents->SymmetricOn();
    //_agents->Update();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(extern_glyphs_pedestrians->GetOutputPort());
    //improve the performance

    VTK_CREATE(vtkLookupTable, lut);
    lut->SetHueRange(0.0,0.470);
    //lut->SetSaturationRange(0,0);
    lut->SetValueRange(1.0,1.0);
    lut->SetNanColor(0.2,0.2,0.2,0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();
    mapper->SetLookupTable(lut);

    extern_glyphs_pedestrians_actor_2D->SetMapper(mapper);
    //extern_glyphs_pedestrians_actor_2D->GetProperty()->BackfaceCullingOn();

    //if(extern_trajectories_firstSet.getNumberOfAgents()>0)
    _renderer->AddActor(extern_glyphs_pedestrians_actor_2D);

    //Show directions
    VTK_CREATE(vtkConeSource,agentDirection);
    agentDirection->SetHeight(40);
    agentDirection->SetRadius(15);

    VTK_CREATE(vtkTriangleFilter, tris2);
    tris2->SetInputConnection(agentDirection->GetOutputPort());
    VTK_CREATE(vtkStripper, strip2);
    strip2->SetInputConnection(tris2->GetOutputPort());

    extern_glyphs_directions->SetSourceConnection(strip2->GetOutputPort());
    extern_glyphs_directions->SetInputConnection(agentDirection->GetOutputPort());
    if (frame) extern_glyphs_directions->SetInputData(pData);
    extern_glyphs_directions->ThreeGlyphsOff();
    extern_glyphs_directions->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper2);
    mapper2->SetInputConnection(extern_glyphs_directions->GetOutputPort());
    mapper2->ScalarVisibilityOff(); //to set color
    mapper2->SetLookupTable(lut);

    extern_glyphs_directions_actor->SetMapper(mapper2);
    extern_glyphs_directions_actor->GetProperty()->SetColor(0,0,0); //black
    _renderer->AddActor2D(extern_glyphs_directions_actor);

    // structure for the labels
    VTK_CREATE(vtkLabeledDataMapper, labelMapper);
    extern_pedestrians_labels->SetMapper(labelMapper);
    labelMapper->SetFieldDataName("labels");
    labelMapper->SetLabelModeToLabelFieldData();
    _renderer->AddActor2D(extern_pedestrians_labels);
    extern_pedestrians_labels->SetVisibility(false);
}

void ThreadVisualisation::initGlyphs3D()
{

    if(extern_glyphs_pedestrians_3D) extern_glyphs_pedestrians_3D->Delete();
    if(extern_glyphs_pedestrians_actor_3D) extern_glyphs_pedestrians_actor_3D->Delete();

    extern_glyphs_pedestrians_3D = vtkTensorGlyph::New();
    extern_glyphs_pedestrians_actor_3D = vtkActor::New();

    //now create the glyphs with zylinders
    VTK_CREATE (vtkCylinderSource, agentShape);
    agentShape->SetHeight(160);
    agentShape->SetRadius(20);
    //agentShape->SetCenter(0,0,80);
    agentShape->SetResolution(20);
    /*
        //VTK_CREATE (vtkAssembly, agentShape);
        vtk3DSImporter* importer = vtk3DSImporter::New();
        importer->SetFileName("data/140404_charles.3ds");
        importer->Read();
        importer->Update();
        //importer->GetRenderer()->GetLights();
        //importer->GetRenderWindow()->GetInteractor()->Start();

        ////collect all the elements from the 3ds
        vtkActorCollection* collection=importer->GetRenderer()x->GetActors();
        vtkActor *actorCharlie= collection->GetLastActor();
        actorCharlie->InitPathTraversal();
        vtkMapper *mapperCharlie=actorCharlie->GetMapper();
        mapperCharlie->Update();
        //_agents3D->SetColorGlyphs(false);
        vtkPolyData *dataCharlie=vtkPolyData::SafeDownCast(mapperCharlie->GetInput());

        //strip the data, again
        //speed the rendering using triangles stripers
        VTK_CREATE(vtkTriangleFilter, tris);
        tris->SetInputData(dataCharlie);
        VTK_CREATE(vtkStripper, agentShape);
        agentShape->SetInputConnection(tris->GetOutputPort());
    */

    //speed the rendering using triangles stripers
    //vtkTriangleFilter *tris = vtkTriangleFilter::New();
    VTK_CREATE(vtkTriangleFilter, tris);
    tris->SetInputConnection(agentShape->GetOutputPort());
    //tris->GetOutput()->ReleaseData();

    //vtkStripper *strip = vtkStripper::New();
    VTK_CREATE(vtkStripper, strip);
    strip->SetInputConnection(tris->GetOutputPort());
    //strip->GetOutput()->ReleaseData();

    extern_glyphs_pedestrians_3D->SetSourceConnection(strip->GetOutputPort());

    //first frame
    auto&& frames=extern_trajectories_firstSet.GetFrames();
    if(frames.empty()) return;

    Frame * frame = frames.begin()->second;

    vtkPolyData* pData=NULL;
    if(frame) pData=frame->GetPolyData2D();

    extern_glyphs_pedestrians_3D->SetInputConnection(strip->GetOutputPort());
    if (frame )extern_glyphs_pedestrians_3D->SetInputData(pData);

    extern_glyphs_pedestrians_3D->ThreeGlyphsOff();
    extern_glyphs_pedestrians_3D->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(extern_glyphs_pedestrians_3D->GetOutputPort());

    VTK_CREATE(vtkLookupTable, lut);
    lut->SetHueRange(0.0,0.470);
    //lut->SetSaturationRange(0,0);
    lut->SetValueRange(1.0,1.0);
    lut->SetNanColor(0.2,0.2,0.2,0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();
    mapper->SetLookupTable(lut);

    extern_glyphs_pedestrians_actor_3D->SetMapper(mapper);
    extern_glyphs_pedestrians_actor_3D->GetProperty()->BackfaceCullingOn();
    //if(extern_trajectories_firstSet.getNumberOfAgents()>0)
    _renderer->AddActor(extern_glyphs_pedestrians_actor_3D);

    extern_glyphs_pedestrians_actor_3D->SetVisibility(false);
}

void  ThreadVisualisation::init()
{
}

void ThreadVisualisation::finalize()
{

}

void ThreadVisualisation::QcolorToDouble(const QColor &col, double *rgb)
{
    rgb[0]=(double)col.red()/255.0;
    rgb[1]=(double)col.green()/255.0;
    rgb[2]=(double)col.blue()/255.0;
}

void ThreadVisualisation::initLegend(/*std::vector scalars*/)
{
    //lookup table
    vtkLookupTable* lut =  vtkLookupTable::New();
    lut->SetHueRange(0.0,0.566);
    lut->SetTableRange(20.0, 50.0);
    //lut->SetSaturationRange(0,0);
    //lut->SetValueRange(0.0,1.0);
    lut->SetNumberOfTableValues(50);
    lut->Build();

    //vtkPolyData* polyData = vtkPolyData::New();
    //polyData->sets
    //polyData->SetPoints(pts);
    //polyData->GetPointData()->SetScalars(scalars);
    //vtkPolyDataMapper* mapper =vtkPolyDataMapper::New();
    //mapper->SetLookupTable(lut);
    //  mapper->SetInput(polyData->GetPolyDataOutput());
    //  mapper->SetScalarRange(randomColors->GetPolyDataOutput()->GetScalarRange());

    vtkTextProperty* titleProp = vtkTextProperty::New();
    titleProp->SetFontSize(14);

    vtkTextProperty* labelProp = vtkTextProperty::New();
    labelProp->SetFontSize(10);

    //	cerr<<"label: " <<labelProp->GetFontSize()<<endl;
    //	cerr<<"     : " <<labelProp->GetFontFamilyAsString()<<endl;
    //
    //	cerr<<"title: " <<titleProp->GetFontSize()<<endl;
    //	cerr<<"     : " <<titleProp->GetFontFamilyAsString()<<endl;

    vtkScalarBarActor* scalarBar = vtkScalarBarActor::New();
    scalarBar->SetLookupTable(lut);
    //scalarBar->SetLookupTable(mapper->GetLookupTable());
    scalarBar->SetTitle("Velocities ( cm/s )");
    scalarBar->SetTitleTextProperty(titleProp);
    scalarBar->SetLabelTextProperty (labelProp);
    scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    //	scalarBar->GetPositionCoordinate()->SetValue(0.1, 0.05);
    scalarBar->GetPositionCoordinate()->SetValue(0.005, 0.90075);
    scalarBar->SetOrientationToVertical();
    scalarBar->SetNumberOfLabels(7);
    scalarBar->SetMaximumNumberOfColors(20);
    scalarBar->SetWidth(0.105);
    scalarBar->SetHeight(0.10);
    //scalarBar->SetPickable(1);
    //scalarBar->SetTextPositionToPrecedeScalarBar();
    //scalarBar->SetLabelFormat("%-#5.1f");
    _renderer->AddActor2D(scalarBar);
    _renderer->Render();

}


void ThreadVisualisation::setAxisVisible(bool status)
{
    _axis->SetVisibility(status);
}

void ThreadVisualisation::setCameraPerspective(int mode,int degree)
{
    if(_renderer==NULL) return;

    switch (mode) {
    case 1: //TOP oder RESET
        _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
        break;
    case 2://TOP Rotate [range:-180-->180]
        _topViewCamera->Roll(degree);
        _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
        _topViewCamera->Roll(-degree);
        break;
    case 3://Side Rotate [range:-80-->80]
        _topViewCamera->Elevation(-degree);
        _renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
        _topViewCamera->Elevation(degree);
        break;
    case 4: { // agent virtual reality
        //vtkCamera *camera = renderer->GetActiveCamera();
        //camera->SetRoll(-90);
        //std::cout <<"roll       [ "<< camera->GetRoll()<<" ]"<<std::endl;
        //double roll=camera->GetRoll();
        //camera->Roll(-90-camera->GetRoll());
        //camera->Modified();
    }

        break;
    }
    _renderer->ResetCamera();

}

void ThreadVisualisation::setBackgroundColor(const QColor& col)
{
    double  bgcolor[3];
    QcolorToDouble(col,bgcolor);
    if (_renderer!=NULL)
        _renderer->SetBackground(bgcolor);
}

void ThreadVisualisation::setWindowTitle(QString title)
{
    if(title.isEmpty())return;
    _winTitle=title;
}

/// @todo check this construct
void ThreadVisualisation::setGeometry(FacilityGeometry* geometry)
{
    //this->_geometry=geometry;
    cout<<"dont call me"<<endl;
    exit(0);
}

GeometryFactory &ThreadVisualisation::getGeometry()
{
    //delete the old object
    //delete _geometry;
    //_geometry=new FacilityGeometry();
    return _geometry;
}

void ThreadVisualisation::setWallsColor(const QColor &color)
{
    double  rbgColor[3];
    QcolorToDouble(color,rbgColor);
    _geometry.ChangeWallsColor(rbgColor);
}

void ThreadVisualisation::setFloorColor(const QColor &color)
{
    double  rbgColor[3];
    QcolorToDouble(color,rbgColor);
    _geometry.ChangeFloorColor(rbgColor);
}

void ThreadVisualisation::setObstacleColor(const QColor &color)
{
    double  rbgColor[3];
    QcolorToDouble(color,rbgColor);
    _geometry.ChangeObstaclesColor(rbgColor);
}

void ThreadVisualisation::setGeometryLabelsVisibility(int v)
{
    _geometry.ShowGeometryLabels(v);
}

void ThreadVisualisation::setExitsColor(const QColor &color)
{
    double  rbgColor[3];
    QcolorToDouble(color,rbgColor);
    // std::cout << "ENTER: " << rbgColor[0] << ", " << rbgColor[1] << ", " << rbgColor[2] << "\n";
// HH
    _geometry.ChangeExitsColor(rbgColor);
}

void ThreadVisualisation::setNavLinesColor(const QColor &color)
{
    double  rbgColor[3];
    QcolorToDouble(color,rbgColor);
    _geometry.ChangeNavLinesColor(rbgColor);
}

/// enable/disable 2D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility2D(bool status)
{
    _geometry.Set2D(status);
}

/// enable/disable 3D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility3D(bool status)
{
    _geometry.Set3D(status);
}

void ThreadVisualisation::setOnscreenInformationVisibility(bool show)
{
    _runningTime->SetVisibility(show);
}

void ThreadVisualisation::Create2dAgent()
{

}
