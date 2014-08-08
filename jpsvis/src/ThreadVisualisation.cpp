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

// all modules need to be initialized in vtk6
#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)


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
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkActor.h>
#include <vtkLightKit.h>


#include "geometry/FacilityGeometry.h"
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
#include "Debug.h"

//#include <vector>

#define VTK_CREATE(type, name) \
		vtkSmartPointer<type> name = vtkSmartPointer<type>::New()



ThreadVisualisation::ThreadVisualisation(QObject *parent):
    QThread(parent)
{
    renderer=NULL;
    renderWindow=NULL;
    renderWinInteractor=NULL;
    framePerSecond=25;
    axis=NULL;
    winTitle="header without room caption";
    geometry=new FacilityGeometry();

}

ThreadVisualisation::~ThreadVisualisation()
{

    if(axis)
        axis->Delete();
    delete geometry;

    if(extern_glyphs_pedestrians_3D) extern_glyphs_pedestrians_3D->Delete();
    if(extern_glyphs_pedestrians_actor_3D) extern_glyphs_pedestrians_actor_3D->Delete();
    if(extern_glyphs_pedestrians) extern_glyphs_pedestrians->Delete();
    if(extern_glyphs_pedestrians_actor_2D) extern_glyphs_pedestrians_actor_2D->Delete();
    if(extern_pedestrians_labels) extern_pedestrians_labels->Delete();
}

void ThreadVisualisation::setFullsreen(bool status)
{
    renderWindow->SetFullScreen(status);
}

void ThreadVisualisation::slotSetFrameRate(float fps)
{
    framePerSecond=fps;
}


void ThreadVisualisation::run()
{

    //deactivate the output windows
    //vtkObject::GlobalWarningDisplayOff();

    //emit signalStatusMessage("running");

    // Create the renderer
    renderer = vtkRenderer::New();
    // set the background
    //renderer->SetBackground(.00,.00,.00);
    renderer->SetBackground(1.0,1.0,1.0);
    //add the geometry
    geometry->CreateActors();
    renderer->AddActor(geometry->getActor2D());
    renderer->AddActor(geometry->getActor3D());

    //initialize the datasets
    //mem leak


    initGlyphs2D();
    initGlyphs3D();

    //create the trails
    extern_trail_plotter = new PointPlotter();
    renderer->AddActor(extern_trail_plotter->getActor());

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
    {
        vtkSphereSource* org = vtkSphereSource::New();
        org->SetRadius(300);

        vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(org->GetOutputPort());
        org->Delete();

        // create actor
        vtkActor* actor = vtkActor::New();
        actor->SetMapper(mapper);
        mapper->Delete();
        actor->GetProperty()->SetColor(.90,.90,0.0);
        //renderer->AddActor(actor);
    }
    //add another big circle at null point
    {
        vtkSphereSource* org = vtkSphereSource::New();
        org->SetRadius(300);
        //org->SetCenter(50,80,0);

        vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(org->GetOutputPort());
        org->Delete();

        // create actor
        vtkActor* actor = vtkActor::New();
        actor->SetMapper(mapper);
        mapper->Delete();
        actor->GetProperty()->SetColor(.90,.90,0.0);
        actor->SetPosition(5000,8000,0);
        //renderer->AddActor(actor);
    }


    // Create a real circle, not a sphere
    {
        VTK_CREATE(vtkRegularPolygonSource, polygonSource);
        polygonSource->GeneratePolygonOff();
        polygonSource->SetNumberOfSides(50);
        polygonSource->SetRadius(1000);
        polygonSource->SetCenter(0,0,0);
        polygonSource->Update();

        VTK_CREATE(vtkPolyDataMapper,mapper);
        mapper->SetInputConnection(polygonSource->GetOutputPort());
        VTK_CREATE(vtkActor,actor);
        actor->GetProperty()->SetColor(180.0/255,180.0/255.0,180.0/255.0);
        actor->SetMapper(mapper);
        //renderer->AddActor(actor);
    }

    //compute the relatives delays
    // necessary in the case several dataset were loaded
    computeDelays();

    // Create the render window
    renderWindow = vtkRenderWindow::New();
    renderWindow->AddRenderer( renderer );
    renderWindow->SetSize(960, 800);
    //renderWindow->SetSize(640, 480);
    //renderWindow->SetSize(800, 586);
    //renderWindow->SetSize(1280, 960);

    // add the legend
    //if(SystemSettings::getShowLegend())
    //	initLegend();

    //add the running time frame
    runningTime = vtkTextActor::New();
    runningTime->SetTextScaleModeToViewport();
    //runningTime->SetTextScaleModeToProp();
    //runningTime->SetMinimumSize(10,10);
    //runningTime->SetMinimumSize(180,80);
    //runningTime->SetInput(txt);
    //runningTime->SetDisplayPosition(500,700);
    runningTime->SetVisibility(SystemSettings::getOnScreenInfos());

    // set the properties of the caption
    vtkTextProperty* tprop = runningTime->GetTextProperty();
    //tprop->SetFontFamilyToArial();
    //tprop->BoldOn();
    //tprop->SetLineSpacing(1.0);
    tprop->SetFontSize(10);
    tprop->SetColor(1.0,0.0,0.0);

    renderer->AddActor2D(runningTime);

    //CAUTION: this is necessary for WIN32 to update the window name
    // but his will freeze your system on linux
#ifdef WIN32
    renderWindow->Render();
#endif

    renderWindow->SetWindowName(winTitle.toStdString().c_str());

    // Create an interactor
    renderWinInteractor = vtkRenderWindowInteractor::New();
    renderWindow->SetInteractor( renderWinInteractor );
    renderWinInteractor->Initialize();

    //add a light kit
    {
        VTK_CREATE(vtkLightKit, lightKit);
        //lightKit->SetKeyLightIntensity(1);

        //lightKit->SetKeyLightWarmth(5);
        //lightKit->SetBackLightAngle(30,40);
        //lightKit->SetKeyLightWarmth(2.0);
        //lightKit->SetMaintainLuminance(true);
        //lightKit->AddLightsToRenderer(renderer);
    }


    if(true || SystemSettings::get2D()) {
        renderer->GetActiveCamera()->OrthogonalizeViewUp();
        renderer->GetActiveCamera()->ParallelProjectionOn();
        renderer->ResetCamera();
    }

    //create a timer for rendering the window
    TimerCallback *renderingTimer = new TimerCallback();
    int timer= renderWinInteractor->CreateRepeatingTimer(1000.0/framePerSecond);
    renderingTimer->SetRenderTimerId(timer);
    renderingTimer->setTextActor(runningTime);
    renderWinInteractor->AddObserver(vtkCommand::TimerEvent,renderingTimer);


    //create the necessary connections
    QObject::connect(renderingTimer, SIGNAL(signalRunningTime(unsigned long )),
                     this->parent(), SLOT(slotRunningTime(unsigned long )));

    QObject::connect(renderingTimer, SIGNAL(signalFrameNumber(unsigned long )),
                     this->parent(), SLOT(slotFrameNumber(unsigned long )));

    QObject::connect(renderingTimer, SIGNAL(signalRenderingTime(int)),
                     this->parent(), SLOT(slotRenderingTime(int)));

    // Create my interactor style
    InteractorStyle* style = InteractorStyle::New();
    renderWinInteractor->SetInteractorStyle( style );
    style->Delete();


    //create special camera for the virtual reality feeling
    //renderer->GetActiveCamera()->SetRoll(90);
    //renderer->GetRenderers()->GetFirstRenderer()->GetActiveCamera();

    //Pedestrian::setCamera(renderer->GetActiveCamera());
    //renderer->ResetCamera();

    // just a workaround

    //if(LinePlotter2D::doorColorsToDefault)
    {
        //LinePlotter2D::doorColorsToDefault=false;
        double col[3]= {82.0/255,218.0 /255.0,255.0/255.0};
        double wallcol[3]= {180.0/255,180.0/255.0,180.0/255.0};
        double exitcol[3]= {175.0/255,175.0/255.0,255.0/255.0};
        double navlinecol[3]= {165.0/255,175.0/255.0,225.0/255.0};
        setExitsColor(exitcol);
        setWallsColor(wallcol);
        //setNavLinesColor(navlinecol);
        //showDoors(false);
    }
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
    _topViewCamera->DeepCopy(renderer->GetActiveCamera());

    //TODO: update all system settings
    setGeometryVisibility2D(SystemSettings::get2D());
    setGeometryVisibility3D(!SystemSettings::get2D());

    renderWinInteractor->Start();


    //emit signalStatusMessage("Idle");
    emit signal_controlSequences("CONTROL_RESET");


    //clear some stuffs
    delete extern_trail_plotter;
    finalize();

    renderer->Delete();
    renderWindow->Delete();
    renderWinInteractor->Delete();
    _topViewCamera->Delete();
    renderer=NULL;
}


void ThreadVisualisation::slotControlSequence(const char* para)
{

    //cout <<"control sequence received: " <<sex<<endl;
}

void ThreadVisualisation::setGeometryVisibility( bool status)
{
    if(geometry) {
        if(SystemSettings::get2D()) {
            geometry->set2D(status);
        } else {
            geometry->set3D(status);
        }
    }
}

/// show / hide the walls
void ThreadVisualisation::showWalls(bool status)
{
    if(geometry) {
        geometry->showWalls(status);
    }
}

/// show/ hide the exits
void ThreadVisualisation::showDoors(bool status)
{
    if(geometry) {
        geometry->showDoors(status);
    }
}

void ThreadVisualisation::showNavLines(bool status)
{
    if(geometry) {
        geometry->showNavLines(status);
    }
}

void ThreadVisualisation::showFloor(bool status)
{
    geometry->showFloor(status);
}

void  ThreadVisualisation::initGlyphs2D()
{
    if(extern_glyphs_pedestrians) extern_glyphs_pedestrians->Delete();
    if(extern_glyphs_pedestrians_actor_2D) extern_glyphs_pedestrians_actor_2D->Delete();
    if(extern_pedestrians_labels) extern_pedestrians_labels->Delete();

    extern_glyphs_pedestrians = vtkTensorGlyph::New();
    extern_glyphs_pedestrians_actor_2D = vtkActor::New();
    extern_pedestrians_labels =  vtkActor2D::New();

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

    //speed the rendering using triangles stripers
    vtkTriangleFilter *tris = vtkTriangleFilter::New();
    tris->SetInputConnection(agentShape->GetOutputPort());
    //tris->GetOutput()->ReleaseData();

    vtkStripper *strip = vtkStripper::New();
    strip->SetInputConnection(tris->GetOutputPort());
    //strip->GetOutput()->ReleaseData();

    extern_glyphs_pedestrians->SetSourceConnection(strip->GetOutputPort());
    //extern_glyphs_pedestrians->SetSourceConnection(agentShape->GetOutputPort());

    //first frame
    Frame * frame = extern_trajectories_firstSet.getFrame(0);
    vtkPolyData* pData=NULL;
    if(frame) pData=frame->GetPolyData2D();

#if VTK_MAJOR_VERSION <= 5
    extern_glyphs_pedestrians->SetSource(agentShape->GetOutput());
    if (frame )extern_glyphs_pedestrians->SetInput(pData);
#else
    extern_glyphs_pedestrians->SetInputConnection(agentShape->GetOutputPort());
    if (frame) extern_glyphs_pedestrians->SetInputData(pData);
#endif

    extern_glyphs_pedestrians->ThreeGlyphsOff();
    extern_glyphs_pedestrians->ExtractEigenvaluesOff();
    //_agents->SymmetricOn();
    //_agents->Update();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(extern_glyphs_pedestrians->GetOutputPort());
    //improve the performance
    mapper->GlobalImmediateModeRenderingOn();


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

    if(extern_trajectories_firstSet.getNumberOfAgents()>0)
        renderer->AddActor(extern_glyphs_pedestrians_actor_2D);

    // structure for the labels
    VTK_CREATE(vtkLabeledDataMapper, labelMapper);
    extern_pedestrians_labels->SetMapper(labelMapper);
    labelMapper->SetFieldDataName("labels");
    labelMapper->SetLabelModeToLabelFieldData();
    renderer->AddActor2D(extern_pedestrians_labels);
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
        vtkActorCollection* collection=importer->GetRenderer()->GetActors();
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

    extern_glyphs_pedestrians_3D->SetSourceConnection(agentShape->GetOutputPort());

    //first frame
    Frame * frame = extern_trajectories_firstSet.getFrame(0);
    vtkPolyData* pData=NULL;
    if(frame) pData=frame->GetPolyData2D();

#if VTK_MAJOR_VERSION <= 5
    extern_glyphs_pedestrians_3D->SetSource(agentShape->GetOutput());
    if (frame )extern_glyphs_pedestrians_3D->SetInput(pData);
#else
    extern_glyphs_pedestrians_3D->SetInputConnection(agentShape->GetOutputPort());
    if (frame )extern_glyphs_pedestrians_3D->SetInputData(pData);
#endif

    extern_glyphs_pedestrians_3D->ThreeGlyphsOff();
    extern_glyphs_pedestrians_3D->ExtractEigenvaluesOff();

    VTK_CREATE(vtkPolyDataMapper, mapper);
    mapper->SetInputConnection(extern_glyphs_pedestrians_3D->GetOutputPort());
    mapper->GlobalImmediateModeRenderingOn();

    VTK_CREATE(vtkLookupTable, lut);
    lut->SetHueRange(0.0,0.470);
    //lut->SetSaturationRange(0,0);
    lut->SetValueRange(1.0,1.0);
    lut->SetNanColor(0.2,0.2,0.2,0.5);
    lut->SetNumberOfTableValues(256);
    lut->Build();
    mapper->SetLookupTable(lut);

    extern_glyphs_pedestrians_actor_3D->SetMapper(mapper);
    //extern_glyphs_pedestrians_actor_3D->GetProperty()->BackfaceCullingOn();
    if(extern_trajectories_firstSet.getNumberOfAgents()>0)
        renderer->AddActor(extern_glyphs_pedestrians_actor_3D);

    extern_glyphs_pedestrians_actor_3D->SetVisibility(false);
}

void  ThreadVisualisation::init() {}


void ThreadVisualisation::finalize()
{
}

/// compute the relative delays to the datasets

void  ThreadVisualisation::computeDelays() {}

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
    renderer->AddActor2D(scalarBar);
    renderer->Render();

}


void ThreadVisualisation::setAxisVisible(bool status)
{
    axis->SetVisibility(status);
}

void ThreadVisualisation::setCameraPerspective(int mode)
{
    if(renderer==NULL) return;

    switch (mode) {
    case 1: //TOP oder RESET
        renderer->GetActiveCamera()->DeepCopy(_topViewCamera);
        break;

    case 2://SIDE

        break;

    case 3:
        //FRONT
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
}

void ThreadVisualisation::setBackgroundColor(double* color)
{
    if (renderer!=NULL)
        renderer->SetBackground(color);
}

void ThreadVisualisation::setWindowTitle(QString title)
{
    if(title.isEmpty())return;
    winTitle=title;
}

/// @todo check this construct
void ThreadVisualisation::setGeometry(FacilityGeometry* geometry)
{
    this->geometry=geometry;
}

FacilityGeometry* ThreadVisualisation::getGeometry()
{
    //if(geometry==NULL){
    geometry=new FacilityGeometry();
    //}
    return geometry;
}

void ThreadVisualisation::setWallsColor(double* color)
{
    geometry->changeWallsColor(color);
}

void ThreadVisualisation::setFloorColor(double *color)
{
    geometry->changeFloorColor(color);
}

void ThreadVisualisation::setGeometryLabelsVisibility(int v)
{
    geometry->showGeometryLabels(v);
}

void ThreadVisualisation::setExitsColor(double* color)
{
    geometry->changeExitsColor(color);
}

void ThreadVisualisation::setNavLinesColor(double *color)
{
    geometry->changeNavLinesColor(color);
}

/// enable/disable 2D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility2D(bool status)
{
    if(geometry!=NULL) //FIXME this should never happens
        geometry->set2D(status);
}

/// enable/disable 3D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility3D(bool status)
{
    if(geometry!=NULL) //FIXME this should never happens, seems to be called by slotReset() !
        geometry->set3D(status);
}

void ThreadVisualisation::setOnscreenInformationVisibility(bool show)
{
    runningTime->SetVisibility(show);
}

