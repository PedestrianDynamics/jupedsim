

#include "Simulation.h"

/************************************************
// Konstruktoren
 ************************************************/

Simulation::Simulation() {
	pOnline = false; // true when online simulation (TraVisTo) is wished
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

	Log->write("INFO: \tOptionen an Simulation geben\n");

	// Online Simulation für TraVisTo?
	switch (args->GetTravisto()) {
		case 0:
			break;
		case 1:
		{
			//iod = new IODispatcher();
			OutputHandler* tofile = new FileHandler(args->GetTrajOutputDir().c_str());
			iod->AddIO(tofile);
			break;
		}

		case 2:
		{
			OutputHandler* tofile = new FileHandler(args->GetTrajOutputDir().c_str());
			iod->AddIO(tofile);
			OutputHandler* travisto = new TraVisToHandler();
			iod->AddIO(travisto);
			pOnline = true;
			break;
		}
		case 3: //plain text
			delete iod; // delete the previously allocated memory
			Log->write("ERROR:\tdon't use this writer, not implemented");
			exit(0);
			break;

		case 4: //flat format
		{
			OutputHandler* file = new FileHandler("./Trajektorien.dat");
			pTrajectories->AddIO(file);
		}
			break;

		default:
			printf("Wrong option for TraVisTo Output!\n\n");
			exit(0);
	}


	if (pOnline)
		s.append("\tonline\n");

	// Linked Cells?
	pLinkedCells = args->GetLinkedCells();
	if (pLinkedCells)
		s.append("\tusing Linked-Cells\n");

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
			rout = new GlobalRouter();
			//rout = new DummyRouter();
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

	//FIXME: why need routing here?
	pBuilding->SetRouting(rout);
	sprintf(tmp, "\tGeometrie: [%s]\n", args->GetGeometryFilename().c_str());
	s.append(tmp);
	Log->write("INFO: \t" + s);
	pBuilding->LoadBuilding(args->GetGeometryFilename());
	//pBuilding->AddSurroundingRoom();
	pBuilding->InitGeometry(); // Polygone erzeugen

	pBuilding->LoadTrafficInfo(args->GetTrafficFile());

	pBuilding->LoadRoutingInfo(args->GetRoutingFile());

	// initialise the routing engine before doing any other things
	rout->Init(pBuilding);

	// init pathway
	if(args->GetPathwayFile()!=""){
		char name[30]="";
		sprintf(name,"%s_p0",args->GetPathwayFile().c_str());
		pBuilding->InitSavePedPathway(name);
	}

	InitSimulation();

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
	int nPeds = pDistribution->Distribute(pBuilding);
	pBuilding->InitPhiAllPeds();
	SetNPeds(nPeds);

	for (int i = 0; i < pBuilding->GetAnzRooms(); i++) {
		Room* room = pBuilding->GetRoom(i);
		for (int j = 0; j < room->GetAnzSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetAnzPedestrians(); k++) {
				sub->GetPedestrian(k)->Setdt(pDt);
			}
		}
	}

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

	// writing the header
	iod->WriteHeader(pNPeds, fps, pBuilding);
	iod->WriteGeometry(pBuilding);
	iod->WriteFrame(0, pBuilding);

	//first initialisation needed by the linked-cells
	if (pLinkedCells) Update();


	// main program loop
	for (t = 0; t < pTmax && GetNPeds() > 0; ++frameNr) {
		t = 0 + (frameNr - 1) * pDt;
		// solve ODE: berechnet Kräfte und setzt neue Werte für x und v
		pSolver->solveODE(t, t + pDt, pBuilding);
		// gucken ob Fußgänger in neuen Räumen/Unterräumen
		Update();
		// ggf. Ausgabe für TraVisTo
		if (frameNr % writeInterval == 0) {
			iod->WriteFrame(frameNr / writeInterval, pBuilding);
		}
	}
	// writing the footer
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
	SetNPeds(pBuilding->GetAllPedestrians().size());

	//update the cells position
	if (pLinkedCells){
		pBuilding->UpdateGrid();
	}
}
