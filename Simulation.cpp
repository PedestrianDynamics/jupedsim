/*Simulation.cpp
  Implements the methods of the class Simulation.
  Copyright (C) <2009-2010>  <Jonas Mehlich and Mohcine Chraibi>

  This file is part of OpenPedSim.

  OpenPedSim is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  OpenPedSim is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.

 */
#include "Simulation.h"

/************************************************
// Konstruktoren
 ************************************************/

Simulation::Simulation() {
	pOnline = false; // true wenn online simulation (TraVisTo)
	pLinkedCells = false;
	pActionPt = 0; // on or off, wird für GCFM benötigt
	pNPeds = 0; // number of pedestrians, Default 10
	pNpedsGlobal=0;// number of pedestrians on all processors
	pTmax = 0;
	pDt = 0;
	pBuilding = NULL;
	pDistribution = NULL;
	pDirection = NULL;
	pModel = NULL;
	pSolver = NULL;
	iod = new IODispatcher();
	pTrajectories = new Trajectories();
}

Simulation::~Simulation() {
	delete pBuilding;
	delete pDistribution;
	delete pDirection;
	delete pModel;
	delete pSolver;
	delete iod;
	delete pTrajectories;
}

/************************************************
// Setter-Funktionen
 ************************************************/

int Simulation::SetOnline(bool o) {
	return pOnline = o;
}

int Simulation::SetNPeds(int i) {
	return pNPeds = i;
}

int Simulation::SetLinkedCells(bool l) {
	return pLinkedCells = l;
}

/************************************************
// Getter-Funktionen
 ************************************************/

bool Simulation::IsOnline() const {
	return pOnline;
}

bool Simulation::IsLinkedCells() {
	return pLinkedCells;
}

int Simulation::GetNPeds() const {
	return pNPeds;
}

int Simulation::GetNPedsGlobal() const {
	return pNpedsGlobal;
}


Building * Simulation::GetBuilding() const {
	return pBuilding;
}

/************************************************
// Sonstige-Funktionen
 ************************************************/

/* bekommt alle Konsolenoptionen vom ArgumentParser
 * und setzt die entsprechenden Parameter in der Simulation
 * */
void Simulation::InitArgs(ArgumentParser* args) {
	char tmp[CLENGTH];
	string s = "Parameter:\n";

	if(Log) delete Log;
	switch (args->GetLog()) {
		case 0:
			Log = new OutputHandler();
			break;
		case 1:
			Log = new STDIOHandler();
			break;
		case 2:
		{
			char name[CLENGTH]="";
			sprintf(name,"%s.P%d.dat",args->GetErrorLogFile().c_str(),pMPIDispatcher->GetMyRank());
			Log = new FileHandler(name);
		}
		break;
		default:
			printf("Wrong option for Logfile!\n\n");
			exit(0);
	}

	Log->write("INFO: \tOptionen an Simulation geben\n");

	// Online Simulation für TraVisTo?
	switch (args->GetTravisto()) {
		case 0:
			break;
		case 1:
		{
			//iod = new IODispatcher();
			char name[30]="";
			sprintf(name,"./P%d_Output.xml",pMPIDispatcher->GetMyRank());
			OutputHandler* tofile = new FileHandler(name);
			iod->AddIO(tofile);
			break;
		}

		case 2:
		{
			OutputHandler* tofile = new FileHandler("./Output.xml");
			iod->AddIO(tofile);
			OutputHandler* travisto = new TraVisToHandler();
			iod->AddIO(travisto);
			pOnline = true;
			break;
		}
		case 3: //hermes
			delete iod; // delete the previously allocated memory
			iod = new HermesIODispatcher(args->GetTrajOutputDir(),args->GetSeed(),args->GetScenarioID());
			break;
		default:
			printf("Wrong option for TraVisTo Output!\n\n");
			exit(0);
	}


	switch (args->GetTrajektorien()) {
		case 0:
			break;
		case 1:
		{
			OutputHandler* std = new STDIOHandler();
			pTrajectories->AddIO(std);
			break;
		}
		case 2:
		{
			OutputHandler* file = new FileHandler("./Trajektorien.dat");
			pTrajectories->AddIO(file);
			break;
		}
		default:
			printf("Wrong option for Trajectories!\n\n");
			exit(0);
	}

	if (pOnline)
		s.append("\tonline\n");

	// Linked Cells?
	pLinkedCells = args->GetLinkedCells();
	if (pLinkedCells)
		s.append("\tusing Linked-Cells\n");

//FIXME merge the methods and move all the methods to a sigle class PedDistributor

// Startverteilung der Fußgänger
//	int peddis = args->GetRandomize();
//	sprintf(tmp, "\tStartverteilung: %d\n", peddis);
//	s.append(tmp);
//	switch (peddis) {
//		case 1:
//			pDistribution = new Random2DSubroom(args->GetV0Mu(), args->GetV0Sigma(), args->GetBmaxMu(),
//					args->GetBmaxSigma(), args->GetBminMu(), args->GetBminSigma(), args->GetAtauMu(),
//					args->GetAtauSigma(), args->GetAminMu(), args->GetAminSigma(), args->GetTauMu(),
//					args->GetTauSigma());
//			break;
//		case 2:
//			pDistribution = new Random2DRoom(args->GetV0Mu(), args->GetV0Sigma(), args->GetBmaxMu(),
//					args->GetBmaxSigma(), args->GetBminMu(), args->GetBminSigma(), args->GetAtauMu(),
//					args->GetAtauSigma(), args->GetAminMu(), args->GetAminSigma(), args->GetTauMu(),
//					args->GetTauSigma());
//			break;
//	}
//	s.append(pDistribution->writeParameter());
//	pDistribution->InitDistributor(args->GetNumberFilename());

	pDistribution = new PedDistributor(args->GetV0Mu(), args->GetV0Sigma(), args->GetBmaxMu(),
			args->GetBmaxSigma(), args->GetBminMu(), args->GetBminSigma(), args->GetAtauMu(),
			args->GetAtauSigma(), args->GetAminMu(), args->GetAminSigma(), args->GetTauMu(),
			args->GetTauSigma());
	s.append(pDistribution->writeParameter());
	pDistribution->InitDistributor(args->GetNumberFilename());


	// Richtungswahl zum Ziel
	int direction = args->GetExitStrategy();
	sprintf(tmp, "\tRichtung zum Ausgang: %d\n", direction);
	s.append(tmp);
	switch (direction) {
		case 1:
			pDirection = new DirectionMiddlePoint();
			break;
		case 2:
			pDirection = new DirectionMinSeperation();
			break;
		case 3:
			pDirection = new DirectionMinSeperationShorterLine();
			break;
	}
	pModel = new GCFMModel(pDirection, args->GetNuPed(), args->GetNuWall(), args->GetDistEffMaxPed(),
			args->GetDistEffMaxWall(), args->GetIntPWidthPed(), args->GetIntPWidthWall(),
			args->GetMaxFPed(), args->GetMaxFWall());
	s.append("\tModel: GCFMModel\n");
	s.append(pModel->writeParameter());

	// ODE Löser
	int solver = args->GetSolver();
	sprintf(tmp, "\tODE Loeser: %d\n", solver);
	s.append(tmp);
	switch (solver) {
		case 1:
			if (pLinkedCells)
				pSolver = new EulerSolverLC(pModel);
			else
				pSolver = new EulerSolver(pModel);
			break;
		case 2:
			pSolver = new VelocityVerletSolver(pModel);
			break;
		case 3:
			pSolver = new LeapfrogSolver(pModel);
			break;
	}
	pTmax = args->GetTmax();
	sprintf(tmp, "\tt_max: %f\n", pTmax);
	s.append(tmp);
	pDt = args->Getdt();
	sprintf(tmp, "\tdt: %f\n", pDt);
	s.append(tmp);

	fps=args->Getfps();
	sprintf(tmp, "\tfps: %f\n", fps);
	s.append(tmp);

	// Routing
	int router = args->GetRoutingStrategy();
	sprintf(tmp, "\tRouting Strategy: %d\n", router);
	s.append(tmp);
	Routing* rout = NULL;
	switch (router) {
		case 1:
			Log->write("Warning: The router 1 is not longer available");
			exit(EXIT_FAILURE);
			break;
		case 2:
			rout = new GlobalRouter();
			break;
		case 3:
			rout = new QuickestPathRouter();
			break;
		case 4:
			rout = new CircleRouter();
			break;
	}

	// Building benötigt Routing
	pBuilding = new Building();
	pBuilding->SetRouting(rout);
	sprintf(tmp, "\tGeometrie: [%s]\n", args->GetGeometryFilename().c_str());
	s.append(tmp);
	Log->write("INFO: \t" + s);
	pBuilding->LoadBuilding(args->GetGeometryFilename());
	pBuilding->AddSurroundingRoom();

	pBuilding->LoadStatesOfDoors(args->GetDoorsStateFile());
	pBuilding->LoadStatesOfRooms(args->GetRoomsStateFile());

	pBuilding->InitGeometry(); // Polygone erzeugen

	//	//close some doors by default to improve the dynamic
	//	//room 060
	//	pBuilding->GetTransition("AR09URO07")->Close();
	//	pBuilding->GetTransition("AR09URO08")->Close();
	//
	//	//room 010
	//	pBuilding->GetTransition("AR0700279")->Close();
	//
	//	//room070
	//	pBuilding->GetTransition("AR09URS09")->Close();
	//
	//	//room080
	//	pBuilding->GetTransition("AR09URS13")->Close();
	//	pBuilding->GetTransition("AR09URS12")->Close();
	//
	//	//room090
	//	//pBuilding->GetTransition("AR02URW01")->Close();
	//
	//	//room100
	//	pBuilding->GetTransition("AR03URW02")->Close();
	//
	//
	//	//room 140
	//	pBuilding->GetTransition("AR03URW03")->Close();
	//	pBuilding->GetTransition("AR0300058")->Close();

	if(args->GetEvacuationType()=="normal")
		InitRoutineClearing();

	// initialise the routing engine before doing any other things
	rout->Init(pBuilding);


	// init pathway
	if(args->GetPathwayFile()!=""){
		char name[30]="";
		sprintf(name,"%s_p%d",args->GetPathwayFile().c_str(),pMPIDispatcher->GetMyRank());
		pBuilding->InitSavePedPathway(name);
	}

	InitSimulation();

	//set the final destinations only for the normal case
	if(args->GetEvacuationType()=="normal")
		DistributeDestinations();

	//using linkedcells?
	if (pLinkedCells){
		pBuilding->InitGrid(args->GetLinkedCellSize());
	}

	//pBuilding->WriteToErrorLog();
}

/* Setzt die Fußgänger in die einzelnen Räume
 * und intialisiert phi (Winkel/Ausrichtung der Ellipse)
 * */

int Simulation::InitSimulation() {
	// MPI:
	// initialize the mpi core
	// This should be done before the pedestrian are distributed
	pMPIDispatcher->Initialise(pBuilding);

	int nPeds = pDistribution->Distribute(pBuilding);
	pBuilding->InitPhiAllPeds();
	SetNPeds(nPeds);

	// MPI:
	// work only on the working area
	const vector<int>& workingArea=pBuilding->GetMPIDispatcher()->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* room = pBuilding->GetRoom(workingArea[wa]);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				sub->GetPedestrian(k)->Setdt(pDt);
			}
		}
	}

	pBuilding->InitRoomsAndSubroomsMap();


	//MPI:
	//pMPIDispatcher->Dump();

	Log->write("INFO: \tInit Simulation successful!!!\n");
	return 0;
}

/* Eigentliche Simulation
 * Rückgabewert:
 *   - Evakuierungszeit
 * */

int Simulation::RunSimulation() {
	int frameNr = 1; // Frame Number
	int writeInterval = (int) ((1. / fps) / pDt + 0.5);
	writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
	double t;

	// erste Augabe
	iod->WriteHeader(pNPeds, fps, pBuilding);
	iod->WriteGeometry(pBuilding);
	iod->WriteFrame(0, pBuilding);

	//first initialisation needed by the linked-cells
	if (pLinkedCells) Update();


	/************************************************************/
	/*************     main simulation loop *********************/
	/************************************************************/
	// bis Tmax oder alle Fußgänger raus
	for (t = 0; t < pTmax && !pMPIDispatcher->Shutdown() ; ++frameNr) {
		t = 0 + (frameNr - 1) * pDt;
		// solve ODE: berechnet Kräfte und setzt neue Werte für x und v
		pSolver->solveODE(t, t + pDt, pBuilding);
		// gucken ob Fußgänger in neuen Räumen/Unterräumen
		Update();
		// ggf. Ausgabe für TraVisTo
		if (frameNr % writeInterval == 0) {
			iod->WriteFrame(frameNr / writeInterval, pBuilding);
		}
		int write = (int) ((1. / 16) / pDt + 0.5);
		if (frameNr % write == 0) { // 16 frames pro Sekunde wie bei Experimenten
			pTrajectories->WriteFrame(frameNr / write, pBuilding);
		}
	}
	// Abschluss für TraVisTo
	iod->WriteFooter();

	//return the evacuation time
	return (int) t;
}

/* Geht alle Räume, dann Unterräume durch um zugucken
 * ob Fußgänger von einem Raum in einen anderen gesetzt
 * werden müßen
 * */

void Simulation::Update() {
	pBuilding->Update();
	// Neue Anzahl von Fußgänger, falls jemand ganz raus geht
	//	SetNPeds(pBuilding->GetAnzPedestrians());
	SetNPeds(pBuilding->GetAllPedestrians().size());

	//update the cells position
	if (pLinkedCells){
		//Grid::update(pBuilding);
		pMPIDispatcher->Update();
	}
}

void Simulation::SetMPIDispatcher(MPIDispatcher *mpi){
	pMPIDispatcher = mpi;
}

const MPIDispatcher* Simulation::GetMPIDispatcher() const {
	return pMPIDispatcher;
}


void Simulation::InitRoutineClearing(){
	//close some specific doors

	//	//close some doors by default to improve the dynamic
	//	//room 060
	//	pBuilding->GetTransition("AR09URO07")->Close();
	//	pBuilding->GetTransition("AR09URO08")->Close();
	//
	//	//room 010
	//	pBuilding->GetTransition("AR0700279")->Close();
	//
	//	//room070
	//	pBuilding->GetTransition("AR09URS09")->Close();
	//
	//	//room080
	//	pBuilding->GetTransition("AR09URS13")->Close();
	//	pBuilding->GetTransition("AR09URS12")->Close();
	//
	//	//room090
	//	//pBuilding->GetTransition("AR02URW01")->Close();
	//
	//	//room100
	//	pBuilding->GetTransition("AR03URW02")->Close();
	//
	//
	//	//room 140
	//	pBuilding->GetTransition("AR03URW03")->Close();
	//	pBuilding->GetTransition("AR0300058")->Close();

	//close all doors with NO_NAME als caption
	for (unsigned int roomID=0; roomID< pBuilding->GetAllRooms().size(); roomID++){

		Room* room = pBuilding->GetRoom(roomID);

		//take all transitions in that room
		const vector<int>& trans=room->GetAllTransitionsIDs();

		for(unsigned int t=0;t<trans.size();t++){

			//get the transition
			int transID=trans[t];
			//get the room at the other side of the transition
			Transition* tr=((Transition*)pBuilding->GetRouting()->GetGoal(transID));
			if(tr->GetCaption()=="No_Name"){
				int ntr1= tr->GetSubRoom1()->GetAllGoalIDs().size();
				int ntr2= tr->GetSubRoom2()->GetAllGoalIDs().size();

				if ((ntr1!=1)&&(ntr2!=1)){
					tr->Close();
				}
			}
		}
	}
}

/// assign pedestrians their final destinations
void Simulation::DistributeDestinations(){
	//percentage
	const vector<int>& workingArea=pBuilding->GetMPIDispatcher()->GetMyWorkingArea();
	for (unsigned int wa = 0; wa < workingArea.size(); wa++) {
		Room* room = pBuilding->GetRoom(workingArea[wa]);
		string caption=room->GetCaption();

		if(caption=="060"){
			// half of them to the parking
			int to_parking_total=room->GetAnzPedestrians()/3;
			int to_parking=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_parking<to_parking_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_PARKING_TOP);
						to_parking++;
					}else {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_010);
					}
				}
			}
		}

		if(caption=="070"){

			int to_room_020_total=room->GetAnzPedestrians()/5;
			int to_room_020=0;

			int to_room_010_total=room->GetAnzPedestrians()/5;
			int to_room_010=0;


			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_020<to_room_020_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_020);
						to_room_020++;
					}else if(to_room_010<to_room_010_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_010);
						to_room_010++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_OUT);
					}
				}
			}
		}

		if(caption=="080"){
			//
			int to_room_010_total=room->GetAnzPedestrians()/10;
			int to_room_010=0;

			int to_room_030_total=room->GetAnzPedestrians()/10;
			int to_room_030=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_010<to_room_010_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_010);
						to_room_010++;
					}else if(to_room_030<to_room_030_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_030);
						to_room_030++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_020);
					}
				}
			}
		}

		if(caption=="-090"){
			//
			int to_room_020_total=room->GetAnzPedestrians()/5;
			int to_room_020=0;

			int to_room_040_total=room->GetAnzPedestrians()/5;
			int to_room_040=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_040<to_room_040_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_040);
						to_room_040++;
					}else if(to_room_020<to_room_020_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_020);
						to_room_020++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_030);
					}
				}
			}
		}

		if(caption=="100"){

			int to_room_030_total=room->GetAnzPedestrians()/2;
			int to_room_030=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_030<to_room_030_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_030);
						to_room_030++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_040);
					}
				}
			}
		}

		if(caption=="110"){
			// the world is fine for the pedestrians in this room
		}

		if(caption=="120"){
			//
			int to_room_050_total=room->GetAnzPedestrians()/5;
			int to_room_050=0;

			int to_parking_bottom_total=room->GetAnzPedestrians()/5;
			int to_parking_bottom=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_050<to_room_050_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_050);
						to_room_050++;
					}else if(to_parking_bottom<to_parking_bottom_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_PARKING_BOTTOM);
						to_parking_bottom++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_040);
					}
				}
			}
		}

		if(caption=="130"){
			//
			int to_room_050_total=room->GetAnzPedestrians()/2;
			int to_room_050=0;

			int to_parking_bottom_total=room->GetAnzPedestrians()/4;
			int to_parking_bottom=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_050<to_room_050_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_050);
						to_room_050++;
					}else if(to_parking_bottom<to_parking_bottom_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_PARKING_BOTTOM);
						to_parking_bottom++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_040);
					}
				}
			}
		}

		if(caption=="140"){
			//
			int to_room_050_total=room->GetAnzPedestrians()/2;
			int to_room_050=0;

			int to_parking_bottom_total=room->GetAnzPedestrians()/4;
			int to_parking_bottom=0;

			for (int j = 0; j < room->GetAnzSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
					if(to_room_050<to_room_050_total){
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_ROOM_050);
						to_room_050++;
					}else if(to_parking_bottom<to_parking_bottom_total) {
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_PARKING_BOTTOM);
						to_parking_bottom++;
					}else{
						sub->GetPedestrian(k)->SetFinalDestination(FINAL_DEST_OUT);
					}
				}
			}
		}

	} // looping over the rooms
}
