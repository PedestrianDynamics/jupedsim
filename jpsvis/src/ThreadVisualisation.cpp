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
#include "Debug.h"
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkActor.h>

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

ThreadVisualisation::~ThreadVisualisation() {
	//		if(renderer)
	//			renderer->Delete();
	//		if(renderWindow)
	//			renderWindow->Delete();
	//		if(renderWinInteractor)
	//			renderWinInteractor->Delete();
	//		if(geometry)
	//			geometry->Delete();

	if(axis)
		axis->Delete();
	delete geometry;
	//		vtkRenderer* renderer;
	//		vtkRenderWindow* renderWindow;
	//		vtkRenderWindowInteractor* renderWinInteractor;

}

void ThreadVisualisation::setFullsreen(bool status){
	renderWindow->SetFullScreen(status);
}

void ThreadVisualisation::slotSetFrameRate(float fps){
	framePerSecond=fps;
}


void ThreadVisualisation::run(){

    //deactivate the output windows
    vtkObject::GlobalWarningDisplayOff();

	//emit signalStatusMessage("running");

	// Create the renderer
	renderer = vtkRenderer::New();
	// set the background
	//renderer->SetBackground(.00,.00,.00);
	renderer->SetBackground(1.0,1.0,1.0);
	//add the geometry
    renderer->AddActor(geometry->getActor2D());
    renderer->AddActor(geometry->getActor3D());

    if(SystemSettings::get2D()==true)
    {
        //Sh
    }

	//initialize the datasets
    //init();
	initGlyphs2D();
    initGlyphs3D();

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


//	if(SystemSettings::get2D()){
//        renderer->GetActiveCamera()->OrthogonalizeViewUp();
//        renderer->GetActiveCamera()->ParallelProjectionOn();
//        renderer->ResetCamera();
//	}



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
		double col[3]={82.0/255,218.0 /255.0,255.0/255.0};
		double wallcol[3]={180.0/255,180.0/255.0,180.0/255.0};
		double exitcol[3]={175.0/255,175.0/255.0,255.0/255.0};
		setExitsColor(exitcol);
		setWallsColor(wallcol);
		//showDoors(false);
	}
	//renderWinInteractor->Initialize();
	// Initialize and enter interactive mode
	// should be called after the observer has been added
    //renderWindow->Modified();

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
	finalize();

	renderer->Delete();
	renderWindow->Delete();
	renderWinInteractor->Delete();
    _topViewCamera->Delete();
	renderer=NULL;
}


void ThreadVisualisation::slotControlSequence(const char* para){

	//cout <<"control sequence received: " <<sex<<endl;
}

void ThreadVisualisation::setGeometryVisibility( bool status){
	if(geometry){
		if(SystemSettings::get2D()){
			geometry->set2D(status);
		}else{
			geometry->set3D(status);
		}
		//geometry->getActor()->SetVisibility(status);
		//geometry->getActor()->Modified();
	}
}

/// show / hide the walls
void ThreadVisualisation::showWalls(bool status){
	if(geometry){
		geometry->showWalls(status);
	}
}

/// show/ hide the exits
void ThreadVisualisation::showDoors(bool status){
	if(geometry){
		geometry->showDoors(status);
	}
}

void  ThreadVisualisation::initGlyphs2D()
{

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


#if VTK_MAJOR_VERSION <= 5
    extern_glyphs_pedestrians->SetSource(agentShape->GetOutput());
#else
    extern_glyphs_pedestrians->SetInputConnection(agentShape->GetOutputPort());
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
    extern_glyphs_pedestrians_actor_2D->GetProperty()->BackfaceCullingOn();
    renderer->AddActor(extern_glyphs_pedestrians_actor_2D);

    //VTK_CREATE(vtkActor, actor);
    //actor->SetMapper(mapper);
    //renderer->AddActor(actor);

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

#if VTK_MAJOR_VERSION <= 5
    extern_glyphs_pedestrians_3D->SetSource(agentShape->GetOutput());
#else
    extern_glyphs_pedestrians_3D->SetInputConnection(agentShape->GetOutputPort());
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
    extern_glyphs_pedestrians_actor_3D->GetProperty()->BackfaceCullingOn();
    renderer->AddActor(extern_glyphs_pedestrians_actor_3D);

//    VTK_CREATE(vtkActor, actor);
//    actor->SetMapper(mapper);
//    renderer->AddActor(actor);
}

void  ThreadVisualisation::init(){
//	//get the datasets parameters.
//	// CAUTION: the functions will return 0 if no datasets were initialized
//	int numOfAgents1=extern_trajectories_firstSet.getNumberOfAgents();
//	int numOfAgents2=extern_trajectories_secondSet.getNumberOfAgents();
//	int numOfAgents3=extern_trajectories_thirdSet.getNumberOfAgents();

//	// super Pedestrians are declared extern
//	// CAUTION: the start ID is 1.
//	if(numOfAgents1>0){
//		//get the first frame from the trajectories and initialize pedes positions
//		Frame * frame = extern_trajectories_firstSet.getNextFrame();
//		// this is not usual, but may happen
//		//just get out if the frame is empty
//		if(frame==NULL) {
//			cerr<<"FATAL 1: Frame is null, the first dataset was not initialised"<<endl;
//			//exit(1);

//		}

//		//extern_pedestrians_firstSet = new Pedestrian*[numOfAgents1];
//		extern_pedestrians_firstSet =(Pedestrian **)malloc(numOfAgents1*sizeof(Pedestrian*));

//		//extern_pedestrians_firstSet =(Pedestrian **)malloc(numOfAgents1*sizeof(Pedestrian*));
//		if(extern_pedestrians_firstSet==NULL){
//			cerr<<"could not allocate memory"<<endl;
//			exit(1);
//		}

//		//The initialisation is just to avoid
//		// pedestrians having not defined (0,0,0) position at beginning.

//		for(int i=0;i<numOfAgents1;i++){

//			TrajectoryPoint* point=NULL;
//			int color[3];
//			SystemSettings::getPedestrianColor(0,color);

//			if( (NULL!=frame) && (NULL!=(point=frame->getNextElement()))){
//				extern_pedestrians_firstSet[i]=new Pedestrian(i,point->getX(),point->getY(),point->getZ());
//				// they are all of type 1 (belonging to the first sets)
//				//	extern_pedestrians_firstSet[i]->setType(1);
//				extern_pedestrians_firstSet[i]->setColor(color);

//			}else{

//				extern_pedestrians_firstSet[i]=new Pedestrian(i,0.0,0.0,0.0);
//				extern_pedestrians_firstSet[i]->setVisibility(false);

//				//extern_pedestrians_firstSet[i]->initVisibility(false);
//				//extern_pedestrians_firstSet[i]->setType(1);
//				extern_pedestrians_firstSet[i]->setColor(color);
//			}

//		}

//		//CAUTION: reset the fucking counter.
//		//TODO: include the reset cursor in the getnextFrame routine
//		// which shall be executed when null is returned
//		if (NULL!=frame)
//			frame->resetCursor();

//		// init the pedestrians sizes
//		QStringList heights=extern_trajectories_firstSet.getInitialHeights();
//		for(int i=0;i<heights.size()-1;i+=2){
//			bool ok=false;
//			int id = heights[i].toInt(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			double size= heights[i+1].toDouble(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			//caution: id start with 0
//			extern_pedestrians_firstSet[id-1]->setSize(size);
//		}

//		//init the pedestrians colors,
//		// overwrite the previously set colors
//		if(SystemSettings::getPedestrianColorProfileFromFile()){
//			QStringList colors=extern_trajectories_firstSet.getInitialColors();
//			for(int i=0;i<colors.size()-1;i+=2){
//				bool ok=false;
//				int id = colors[i].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				int color= colors[i+1].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				//cout << id<< " :"<<size<<endl;
//				//caution: id start with 0
//				extern_pedestrians_firstSet[id-1]->setColor(color);

//			}
//		}

//		//all initialisations are done , now create the actors
//		for(int i=0;i<numOfAgents1;i++){
//			renderer->AddActor((vtkProp*)extern_pedestrians_firstSet[i]->getActor());
//			renderer->AddActor((vtkProp*)extern_pedestrians_firstSet[i]->getTrailActor());

//		}
//	}

//	// initialize the second dataset
//	if(numOfAgents2>0){
//		Frame * frame = extern_trajectories_secondSet.getNextFrame();
//		extern_pedestrians_secondSet =(Pedestrian **)malloc(numOfAgents2*sizeof(Pedestrian*));
//		for(int i=0;i<numOfAgents2;i++){

//			// this is not usual, but may happen
//			//just get out if the frame is empty
//			if(frame==NULL) {
//				cerr<<"FATAL 2: Frame is null, the second dataset was not initialised"<<endl;
//				//exit(1);
//				//return;
//			}

//			TrajectoryPoint* point=NULL;
//			int color[3];
//			SystemSettings::getPedestrianColor(1,color);

//			if( (NULL!=frame) && (NULL!=(point=frame->getNextElement()))){
//				extern_pedestrians_secondSet[i]=new Pedestrian(i,point->getX(),point->getY(),point->getZ());
//				// they are all of type 1 (belonging to the first sets)
//				extern_pedestrians_secondSet[i]->setColor(color);
//				//extern_pedestrians_secondSet[i]->setType(2);
//				//extern_pedestrians_firstSet[i]->CreateActor();
//			}else{
//				extern_pedestrians_secondSet[i]=new Pedestrian(i,0.0,0.0,0.0);
//				extern_pedestrians_secondSet[i]->setColor(color);
//				//				extern_pedestrians_secondSet[i]->setType(2);
//				//extern_pedestrians_secondSet[i]->initVisibility(false);
//				extern_pedestrians_secondSet[i]->setVisibility(false);
//			}

//			//renderer->AddActor((vtkProp*)extern_pedestrians_secondSet[i]->getActor());
//			//renderer->AddActor((vtkProp*)extern_pedestrians_secondSet[i]->getTrailActor());
//		}
//		//CAUTION: reset the fucking counter
//		// the frame objects are passed by reference, so the "cursor" stays
//		// at the last index used.
//		frame->resetCursor();

//		// init the pedestians sizes
//		QStringList tokens=extern_trajectories_secondSet.getInitialHeights();
//		for(int i=0;i<tokens.size()-1;i+=2){

//			bool ok=false;
//			int id = tokens[i].toInt(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			double size= tokens[i+1].toDouble(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			extern_pedestrians_secondSet[id-1]->setSize(size);
//		}

//		//init the pedestrians colors,
//		// overwrite the previously set colors
//		if(SystemSettings::getPedestrianColorProfileFromFile()){
//			QStringList colors=extern_trajectories_secondSet.getInitialColors();
//			for(int i=0;i<colors.size()-1;i+=2){
//				bool ok=false;
//				int id = colors[i].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				int color= colors[i+1].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				//caution: id start with 0
//				extern_pedestrians_secondSet[id-1]->setColor(color);

//			}
//		}

//		//all initialisations are done , now create the actors
//		for(int i=0;i<numOfAgents2;i++){
//			renderer->AddActor((vtkProp*)extern_pedestrians_secondSet[i]->getActor());
//			renderer->AddActor((vtkProp*)extern_pedestrians_secondSet[i]->getTrailActor());

//		}

//	}

//	if(numOfAgents3>0){

//		Frame * frame = extern_trajectories_thirdSet.getNextFrame();
//		extern_pedestrians_thirdSet =(Pedestrian **)malloc(numOfAgents3*sizeof(Pedestrian*));
//		for(int i=0;i<numOfAgents3;i++){

//			// this is not usual, but may happen
//			//just get out if the frame is empty
//			if(frame==NULL) {
//				cerr<<"FATAL 3: Frame is null, the third dataset was not initialised"<<endl;
//				//exit(1);

//			}

//			TrajectoryPoint* point=NULL;
//			int color[3];
//			SystemSettings::getPedestrianColor(2,color);

//			if( (NULL!=frame) && (NULL!=(point=frame->getNextElement()))){
//				extern_pedestrians_thirdSet[i]=new Pedestrian(i,point->getX(),point->getY(),point->getZ());
//				// they are all of type 1 (belonging to the first sets)
//				//extern_pedestrians_thirdSet[i]->setType(3);
//				extern_pedestrians_thirdSet[i]->setColor(color);
//				//extern_pedestrians_firstSet[i]->CreateActor();
//			}else{
//				extern_pedestrians_thirdSet[i]=new Pedestrian(i,0.0,0.0,0.0);
//				//				extern_pedestrians_thirdSet[i]->setType(3);
//				extern_pedestrians_thirdSet[i]->setColor(color);
//				//extern_pedestrians_thirdSet[i]->initVisibility(false);
//				extern_pedestrians_thirdSet[i]->setVisibility(false);
//			}

//			//renderer->AddActor((vtkProp*)extern_pedestrians_thirdSet[i]->getActor());
//			//renderer->AddActor((vtkProp*)extern_pedestrians_thirdSet[i]->getTrailActor());
//		}
//		//CAUTION: reset the fucking counter.
//		frame->resetCursor();

//		// init the pedestians sizes
//		QStringList tokens=extern_trajectories_thirdSet.getInitialHeights();
//		for(int i=0;i<tokens.size()-1;i+=2){
//			bool ok=false;
//			int id = tokens[i].toInt(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			double size= tokens[i+1].toDouble(&ok);
//			if(!ok) {cerr<<"skipping size arguments" <<endl;continue;}
//			//cout << id<< " :"<<size<<endl;
//			extern_pedestrians_thirdSet[id-1]->setSize(size);
//		}
//		//init the pedestrians colors,
//		// overwrite the previously set colors
//		if(SystemSettings::getPedestrianColorProfileFromFile()){
//			QStringList colors=extern_trajectories_thirdSet.getInitialColors();
//			for(int i=0;i<colors.size()-1;i+=2){
//				bool ok=false;
//				int id = colors[i].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				int color= colors[i+1].toInt(&ok);
//				if(!ok) {cerr<<"skipping color arguments" <<endl;continue;}
//				//caution: id start with 0
//				extern_pedestrians_thirdSet[id-1]->setColor(color);
//			}
//		}

//		//all initialisations are done , now create the actors
//		for(int i=0;i<numOfAgents3;i++){
//			renderer->AddActor((vtkProp*)extern_pedestrians_thirdSet[i]->getActor());
//			renderer->AddActor((vtkProp*)extern_pedestrians_thirdSet[i]->getTrailActor());

//		}

//	}
}


void ThreadVisualisation::finalize(){
	//get the datasets parameters.
	// CAUTION: the functions will return 0 if no datasets were initialized
	int numOfAgents1=extern_trajectories_firstSet.getNumberOfAgents();
	int numOfAgents2=extern_trajectories_secondSet.getNumberOfAgents();
	int numOfAgents3=extern_trajectories_thirdSet.getNumberOfAgents();


	if(numOfAgents1>0){
		//extern_trajectories_firstSet.clear();
		for(int i=0;i<numOfAgents1;i++){
			delete(extern_pedestrians_firstSet[i]);
		}
		free(extern_pedestrians_firstSet);
		extern_pedestrians_firstSet=NULL;
	}

	if(numOfAgents2>0){
		//extern_trajectories_secondSet.clear();
		for(int i=0;i<numOfAgents2;i++){
			delete(extern_pedestrians_secondSet[i]);
		}
		free(extern_pedestrians_secondSet);
		extern_pedestrians_secondSet=NULL;
	}

	if(numOfAgents3>0){
		//extern_trajectories_thirdSet.clear();
		for(int i=0;i<numOfAgents3;i++){
			delete(extern_pedestrians_thirdSet[i]);
		}
		free(extern_pedestrians_thirdSet);
		extern_pedestrians_thirdSet=NULL;
	}

}

/// compute the relative delays to the datasets

void  ThreadVisualisation::computeDelays(){

	unsigned long delay1[2];
	unsigned long delay2[2];
	unsigned long delay3[2];

	bool first_set_loaded = (extern_trajectories_firstSet.getNumberOfAgents()>0)?true:false;
	bool second_set_loaded = (extern_trajectories_secondSet.getNumberOfAgents()>0)?true:false;
	bool third_set_loaded = (extern_trajectories_thirdSet.getNumberOfAgents()>0)?true:false;

	if(first_set_loaded)
		extern_trajectories_firstSet.getDelayAbsolute(delay1);
	if(second_set_loaded)
		extern_trajectories_secondSet.getDelayAbsolute(delay2);
	if(third_set_loaded)
		extern_trajectories_thirdSet.getDelayAbsolute(delay3);

	// three datasets were loaded
	if(first_set_loaded & second_set_loaded  & third_set_loaded){
		// relative delays are computed with respect to the first dataset
		extern_trajectories_secondSet.computeDelayRelative(delay1);
		extern_trajectories_thirdSet.computeDelayRelative(delay1);

		signed long delay_first=extern_trajectories_firstSet.getDelayRelative();
		signed long delay_second=extern_trajectories_secondSet.getDelayRelative();
		signed long delay_third=extern_trajectories_thirdSet.getDelayRelative();

		if(delay_first<0) {
			delay_second+=-delay_first;
			delay_third+=-delay_first;
			delay_first=0;
		}
		if(delay_second<0){
			delay_first+=-delay_second;
			delay_third+=-delay_second;
			delay_second=0;
		}
		if(delay_third<0){
			delay_first+=-delay_third;
			delay_second+=-delay_third;
			delay_third=0;
		}
		Debug::Messages("delay first data set: %f \n",delay_first);
		Debug::Messages("delay second data set: %f \n",delay_second);
		Debug::Messages("delay third data set: %f \n",delay_third);

		//at this point all delays should be positive
		// this step is no loner necessary
		extern_trajectories_firstSet.setDelayRelative(delay_first);
		extern_trajectories_secondSet.setDelayRelative(delay_second);
		extern_trajectories_thirdSet.setDelayRelative(delay_third);


		extern_trajectories_firstSet.setFrameCursorOffset(- (delay_first*framePerSecond/1000.0f));
		extern_trajectories_secondSet.setFrameCursorOffset(- (delay_second*framePerSecond/1000.0f));
		extern_trajectories_thirdSet.setFrameCursorOffset(- (delay_third*framePerSecond/1000.0f));

	}

	else if (first_set_loaded & second_set_loaded){
		// relative delays are computed with respect to the first dataset
		extern_trajectories_secondSet.computeDelayRelative(delay1);

		signed long delay_first=extern_trajectories_firstSet.getDelayRelative();
		signed long delay_second=extern_trajectories_secondSet.getDelayRelative();

		//implement me in a while loop
		if(delay_first<0) {
			delay_second+=-delay_first;
			delay_first=0;
		}
		if(delay_second<0){
			delay_first+=-delay_second;
			delay_second=0;
		}

		if(delay_first<0) {
			delay_second+=-delay_first;
			delay_first=0;
		}
		if(delay_second<0){
			delay_first+=-delay_second;
			delay_second=0;
		}

		Debug::Messages("delay first data set: %f \n",delay_first);
		Debug::Messages("delay second data set: %f \n",delay_second);

		//at this point all delays should be positive
		extern_trajectories_firstSet.setDelayRelative(delay_first);
		extern_trajectories_secondSet.setDelayRelative(delay_second);

		extern_trajectories_firstSet.setFrameCursorOffset(- (delay_first*framePerSecond/1000.0f));
		extern_trajectories_secondSet.setFrameCursorOffset(- (delay_second*framePerSecond/1000.0f));


	}else if (first_set_loaded & third_set_loaded){
		Debug::Error("Hudston, we got a problem");
	}else if (third_set_loaded & second_set_loaded){
		Debug::Error("Hudston, the matter is serious");
	}else{
		//cerr<<"Only one dataset loaded !,no delays/offsets are computed"<<endl;
	}

}

void ThreadVisualisation::initLegend(/*std::vector scalars*/){

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


void ThreadVisualisation::setAxisVisible(bool status){
	axis->SetVisibility(status);
}

void ThreadVisualisation::setCameraPerspective(int mode){
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

	case 4:// agent virtual reality
	{
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

void ThreadVisualisation::setBackgroundColor(double* color){
	if (renderer!=NULL)
		renderer->SetBackground(color);
}

void ThreadVisualisation::setWindowTitle(QString title){
	if(title.isEmpty())return;
	winTitle=title;
}

/// @todo check this construct
void ThreadVisualisation::setGeometry(FacilityGeometry* geometry){
	this->geometry=geometry;
}

FacilityGeometry* ThreadVisualisation::getGeometry() {
	//if(geometry==NULL){ //FIXME TODO restore me
	geometry=new FacilityGeometry();
	//}
	return geometry;
}

void ThreadVisualisation::setWallsColor(double* color){
	geometry->changeWallsColor(color);
}

void ThreadVisualisation::setGeometryLabelsVisibility(int v){
	geometry->showGeometryLabels(v);
}

void ThreadVisualisation::setExitsColor(double* color){
	geometry->changeExitsColor(color);
}

/// enable/disable 2D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility2D(bool status){
	if(geometry!=NULL) //FIXME this should never happens
		geometry->set2D(status);
}

/// enable/disable 3D
/// todo: consider disabling the 2d option in the 3d, and vice-versa
void ThreadVisualisation::setGeometryVisibility3D(bool status){
	if(geometry!=NULL) //FIXME this should never happens, seems to be called by slotReset() !
		geometry->set3D(status);
}

void ThreadVisualisation::setOnscreenInformationVisibility(bool show){
	runningTime->SetVisibility(show);
}

//void ThreadVisualisation::WindowModifiedCallback( vtkObject*
//		caller, unsigned long eventId,void* clientData ,void* callData )
//{
//	vtkRenderWindow* window = static_cast<vtkRenderWindow*>(caller);
//	int* windowSize = window->GetSize();
//
//
//	int *pos=renderWindow->GetSize();
//	runningTime->SetPosition(pos[1]/2,pos[0]);
//
//}
