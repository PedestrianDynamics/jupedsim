/**
 * @file    Simulation.cpp
 * @date Created on: Dec 15, 2010
 * Copyright (C) <2009-2011>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section description
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 */

#include "Simulation.h"

using namespace std;

Simulation::Simulation() {
	_nPeds = 0;
	_tmax = 0;
	_seed=8091983;
	_deltaT = 0;
	_building = NULL;
	_distribution = NULL;
	_direction = NULL;
	_model = NULL;
	_solver = NULL;
	_iod = new IODispatcher();
	_fps=1;
}

Simulation::~Simulation() {
	delete _building;
	delete _distribution;
	delete _direction;
	delete _model;
	delete _solver;
	delete _iod;
}

/************************************************
// Setter-Funktionen
 ************************************************/


void Simulation::SetPedsNumber(int i) {
	_nPeds = i;
}


/************************************************
// Getter-Funktionen
 ************************************************/


int Simulation::GetPedsNumber() const {
	return _nPeds;
}

Building * Simulation::GetBuilding() const {
	return _building;
}


void Simulation::InitArgs(ArgumentParser* args) {
	char tmp[CLENGTH];
	string s = "Parameter:\n";

	switch (args->GetLog()) {
		case 0:
			// no log file
			//Log = new OutputHandler();
			break;
		case 1:
			if(Log) delete Log;
			Log = new STDIOHandler();
			break;
		case 2:
		{
			char name[CLENGTH]="";
			sprintf(name,"%s.P0.dat",args->GetErrorLogFile().c_str());
			if(Log) delete Log;
			Log = new FileHandler(name);
		}
		break;
		default:
			printf("Wrong option for Logfile!\n\n");
			exit(0);
	}


	if(args->GetPort()!=-1){
		switch(args->GetFileFormat())
		{
		case FORMAT_XML_PLAIN_WITH_MESH:
		case FORMAT_XML_PLAIN:
		{
			OutputHandler* travisto = new TraVisToHandler(args->GetHostname(),
					args->GetPort());
			_iod->AddIO(travisto);
			break;
		}
		case FORMAT_XML_BIN:
		{
			Log->Write("INFO: \tFormat xml-bin not yet supported in streaming\n");
			exit(0);
			break;
		}
		case FORMAT_PLAIN:
		{
			Log->Write("INFO: \tFormat plain not yet supported in streaming\n");
			exit(0);
			break;
		}
		case FORMAT_VTK:
		{
			Log->Write("INFO: \tFormat vtk not yet supported in streaming\n");
			exit(0);
			break;
		}
		}

		s.append("\tonline streaming enabled \n");
	}

	if(args->GetTrajectoriesFile().empty()==false)
	{
		switch (args->GetFileFormat())
		{
		case FORMAT_XML_PLAIN:
		{
			OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
			_iod->AddIO(tofile);
			break;
		}
		case FORMAT_XML_PLAIN_WITH_MESH:
		{
			OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
			if(_iod) delete _iod;
			_iod = new TrajectoriesXML_MESH();
			_iod->AddIO(tofile);
			break;
		}
		case FORMAT_XML_BIN:
		{
			Log->Write("INFO: \tFormat xml-bin not yet supported\n");
			exit(0);
			break;
		}
		case FORMAT_PLAIN:
		{
			Log->Write("INFO: \tFormat plain not yet supported\n");
			OutputHandler* file = new FileHandler(args->GetTrajectoriesFile().c_str());

			if(_iod) delete _iod;
			_iod = new TrajectoriesFLAT();
			_iod->AddIO(file);
			Log->Write("INFO: \tFormat plain maybe not yet implemented ?");
			exit(0);
			break;
		}
		case FORMAT_VTK:
		{
			Log->Write("INFO: \tFormat vtk not yet supported\n");
			OutputHandler* file = new FileHandler((args->GetTrajectoriesFile() +".vtk").c_str());

			if(_iod) delete _iod;
			_iod = new TrajectoriesVTK();
			_iod->AddIO(file);
			//exit(0);
			break;
		}
		}

	}


	_distribution = new PedDistributor(args->GetV0Mu(), args->GetV0Sigma(), args->GetBmaxMu(),
			args->GetBmaxSigma(), args->GetBminMu(), args->GetBminSigma(), args->GetAtauMu(),
			args->GetAtauSigma(), args->GetAminMu(), args->GetAminSigma(), args->GetTauMu(),
			args->GetTauSigma());
	s.append(_distribution->writeParameter());
	_distribution->InitDistributor(args->GetProjectFile());
	// define how the navigation line is crossed
	int direction = args->GetExitStrategy();
	sprintf(tmp, "\tDirection to the exit: %d\n", direction);
	s.append(tmp);
	switch (direction) {
		case 1:
			_direction = new DirectionMiddlePoint();
			break;
		case 2:
			_direction = new DirectionMinSeperation();
			break;
		case 3:
			_direction = new DirectionMinSeperationShorterLine();
			break;
		case 4:
			_direction = new DirectionInRangeBottleneck();
			break;
	}
	_model = new GCFMModel(_direction, args->GetNuPed(), args->GetNuWall(), args->GetDistEffMaxPed(),
			args->GetDistEffMaxWall(), args->GetIntPWidthPed(), args->GetIntPWidthWall(),
			args->GetMaxFPed(), args->GetMaxFWall());
	s.append("\tModel: GCFMModel\n");
	s.append(_model->writeParameter());

	// ODE solver
	int solver = args->GetSolver();
	sprintf(tmp, "\tODE Loeser: %d\n", solver);
	s.append(tmp);
	switch (solver) {
		case 1:
			_solver = new EulerSolverLC(_model);
			break;
		case 2:
			_solver = new VelocityVerletSolver(_model);
			break;
		case 3:
			_solver = new LeapfrogSolver(_model);
			break;
	}
	_tmax = args->GetTmax();
	sprintf(tmp, "\tt_max: %f\n", _tmax);
	s.append(tmp);
	_deltaT = args->Getdt();
	sprintf(tmp, "\tdt: %f\n", _deltaT);
	s.append(tmp);

	_fps=args->Getfps();
	sprintf(tmp, "\tfps: %f\n", _fps);
	s.append(tmp);

	// Routing
	vector< pair<int, RoutingStrategy> >  routers=  args->GetRoutingStrategy();
	RoutingEngine* routingEngine= new RoutingEngine();

	for (unsigned int r= 0;r<routers.size();r++){

		RoutingStrategy strategy=routers[r].second;
		int routerID=routers[r].first;

		switch (strategy) {
		case ROUTING_LOCAL_SHORTEST:
		{
			Router* router=new GlobalRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy local shortest added\n");
			break;
		}
		case ROUTING_GLOBAL_SHORTEST:
		{
                           
			Router* router=new GlobalRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy global shortest added\n");
			break;
		}
		case ROUTING_QUICKEST:
		{
			Router* router=new QuickestPathRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy quickest path added\n");
			break;
		}
		case ROUTING_DYNAMIC:
		{
			Router* router=new GraphRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy graph router added\n");
			break;
		}
		case ROUTING_NAV_MESH:
		{
			Router* router=new MeshRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy nav_mesh  router added\n");
			break;
		}
		case ROUTING_DUMMY:
		{
			Router* router=new DummyRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy dummy router added\n");
			break;
		}
		case ROUTING_SAFEST:
		{
			Router* router=new SafestPathRouter();
			router->SetID(routerID);
			router->SetStrategy(strategy);
			routingEngine->AddRouter(router);
			s.append("\tRouting Strategy safest path router added\n");
			break;
		}
		case ROUTING_UNDEFINED:
		default:
			cout<<"router not available"<<endl;
			exit(EXIT_FAILURE);
			break;
		}
	}
	s.append("\n");

	// IMPORTANT: do not change the order in the following..
	_building = new Building();
	_building->SetRoutingEngine(routingEngine);
	_building->SetProjectFilename(args->GetProjectFile());
	_building->SetProjectRootDir(args->GetProjectRootDir());

	_building->LoadBuildingFromFile();
	_building->LoadRoutingInfo(args->GetProjectFile());
	_building->AddSurroundingRoom();
	_building->InitGeometry(); // create the polygons
	_building->LoadTrafficInfo();
	// in the case the navigation mesh should be written to a file
	if(args->GetNavigationMesh()!=""){
		Log->Write("INFO: \tWriting the navigation mesh to: " + args->GetNavigationMesh());
		//Navigation mesh implementation
		NavMesh* nv= new NavMesh(_building);
		nv->BuildNavMesh();
		//nv->WriteToFile("../pedunc/examples/stadium/arena.nav");
		nv->WriteToFile(args->GetNavigationMesh()+".nav");
		nv->WriteToFileTraVisTo(args->GetNavigationMesh());
		//nv->WriteScenario();
		exit(EXIT_FAILURE);
		//iod->WriteGeometryRVO(pBuilding);exit(EXIT_FAILURE);
		//iod->WriteNavMeshORCA(pBuilding);exit(EXIT_FAILURE);
	}

	_nPeds=_distribution->Distribute(_building);

	//using linkedcells
	if (args->GetLinkedCells()){
		s.append("\tusing Linked-Cells for spatial queries\n");
		_building->InitGrid(args->GetLinkedCellSize());
	}else {
		_building->InitGrid(-1);
	}

	// initialize the routing engine before doing any other things
	routingEngine->Init(_building);

	//this is very specific to the gcfm model
	_building->InitPhiAllPeds(_deltaT);


	//pBuilding->WriteToErrorLog();

	//get the seed
	_seed=args->GetSeed();

	// perform a general check to the .
	_building->SanityCheck();
}


int Simulation::RunSimulation() {
	int frameNr = 1; // Frame Number
	int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
	writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
	double t=0.0;


	// writing the header

	_iod->WriteHeader(_nPeds, _fps, _building,_seed);
	_iod->WriteGeometry(_building);
	_iod->WriteFrame(0,_building);

	//first initialisation needed by the linked-cells
	 Update();

	// main program loop
	for (t = 0; t < _tmax && _nPeds > 0; ++frameNr) {
		t = 0 + (frameNr - 1) * _deltaT;
		// solve ODE: berechnet Kräfte und setzt neue Werte für x und v
		_solver->solveODE(t, t + _deltaT, _building);
		// gucken ob Fußgänger in neuen Räumen/Unterräumen
		Update();
		// ggf. Ausgabe für TraVisTo
		if (frameNr % writeInterval == 0) {
			_iod->WriteFrame(frameNr / writeInterval, _building);
		}
//		if (frameNr % 1000 == 0) {
//			cout<<"                        \r";
//                cout<<">> Frames : "<<frameNr<<"\n";
//			cout.flush();
//		}
	}
	// writing the footer
	_iod->WriteFooter();

	//return the evacuation time
	return (int) t;
}


// TODO: make the building class more independent by moving the update routing here.
void Simulation::Update() {
	//_building->Update();
	_building->UpdateVerySlow();
	//someone might have leave the building
	_nPeds=_building->GetAllPedestrians().size();
	// update the global time
	Pedestrian::SetGlobalTime(Pedestrian::GetGlobalTime()+_deltaT);
	//update the cells position
	_building->UpdateGrid();

}
