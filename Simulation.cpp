
/**
 * @file Simulation.cpp
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
#include "math/GPU_GCFMModel.h"
#include "math/GCFMModel.h"

//#include <omp.h>
//#include "clSetup.h"

#include "math/GCFMModel.h"
#include "math/GompertzModel.h"



using namespace std;

Simulation::Simulation()
{
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
    _em=NULL;
    _profiling = false;
    _hpc = 0;
}

Simulation::~Simulation()
{
    delete _building;
    delete _distribution;
    delete _direction;
    delete _model;
    delete _solver;
    delete _iod;
    delete _em;
}

/************************************************
// Setter-Funktionen
 ************************************************/


void Simulation::SetPedsNumber(int i)
{
    _nPeds = i;
}



/************************************************
// Getter-Funktionen
 ************************************************/


int Simulation::GetPedsNumber() const
{
    return _nPeds;
}

Building * Simulation::GetBuilding() const
{
    return _building;
}

bool Simulation::GetProfileFlag(){
    return _profiling;
}

int Simulation::GetHPCFlag(){
    return _hpc;
}

void Simulation::InitArgs(ArgumentParser* args)
{
    char tmp[CLENGTH];
    string s = "Parameter:\n";


    _argsParser=args;
    switch (args->GetLog()) {
    case 0:
        // no log file
        //Log = new OutputHandler();
        break;
    case 1:
        if(Log) delete Log;
        Log = new STDIOHandler();
        break;
    case 2: {
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


    if(args->GetPort()!=-1) {
        switch(args->GetFileFormat()) {
        case FORMAT_XML_PLAIN_WITH_MESH:
        case FORMAT_XML_PLAIN: {
            OutputHandler* travisto = new TraVisToHandler(args->GetHostname(),
                    args->GetPort());
            _iod->AddIO(travisto);
            break;
        }
        case FORMAT_XML_BIN: {
            Log->Write("INFO: \tFormat xml-bin not yet supported in streaming\n");
            exit(0);
            break;
        }
        case FORMAT_PLAIN: {
            Log->Write("INFO: \tFormat plain not yet supported in streaming\n");
            exit(0);
            break;
        }
        case FORMAT_VTK: {
            Log->Write("INFO: \tFormat vtk not yet supported in streaming\n");
            exit(0);
            break;
        }
        }

        s.append("\tonline streaming enabled \n");
    }

    if(args->GetTrajectoriesFile().empty()==false) {
        switch (args->GetFileFormat()) {
        case FORMAT_XML_PLAIN: {
            OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
            _iod->AddIO(tofile);
            break;
        }
        case FORMAT_XML_PLAIN_WITH_MESH: {
            OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
            if(_iod) delete _iod;
            _iod = new TrajectoriesXML_MESH();
            _iod->AddIO(tofile);
            break;
        }
        case FORMAT_XML_BIN: {
            OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
            if(_iod) delete _iod;
            _iod = new TrajectoriesJPSV06();
            _iod->AddIO(tofile);
            //Log->Write("INFO: \tFormat xml-bin not yet supported\n");
            //exit(0);
            break;
        }
        case FORMAT_PLAIN: {
            Log->Write("INFO: \tFormat plain not yet supported\n");
            OutputHandler* file = new FileHandler(args->GetTrajectoriesFile().c_str());

            if(_iod) delete _iod;
            _iod = new TrajectoriesFLAT();
            _iod->AddIO(file);
            //Log->Write("INFO: \tFormat plain maybe not yet implemented ?");
            //exit(0);
            break;
        }
        case FORMAT_VTK: {
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
    case 5:
        _direction = new DirectionGeneral();
        break;
    default:
        cout<<"Direction strategy not available. Exit"<<endl;
        exit(EXIT_FAILURE);
        break;
    }

    switch (args->GetModel())
    {
    case MODEL_GFCM:
        if(args->GetHPCFlag()){
            _model = new GPU_GCFMModel(_direction, args->GetNuPed(), args->GetNuWall(), args->GetDistEffMaxPed(),
                    args->GetDistEffMaxWall(), args->GetIntPWidthPed(), args->GetIntPWidthWall(),
                    args->GetMaxFPed(), args->GetMaxFWall());
            s.append("\tModel: GCFMModel\n");
            s.append(_model->writeParameter());
        } else {
            _model = new GCFMModel(_direction, args->GetNuPed(), args->GetNuWall(), args->GetDistEffMaxPed(),
                    args->GetDistEffMaxWall(), args->GetIntPWidthPed(), args->GetIntPWidthWall(),
                    args->GetMaxFPed(), args->GetMaxFWall());
            s.append("\tModel: GCFMModel\n");
            s.append(_model->writeParameter());
        }
        break;

    case MODEL_GOMPERTZ:
        _model = new GompertzModel(_direction, args->GetNuPed(), args->GetaPed(), args->GetbPed(), args->GetcPed(),
                args->GetNuWall(), args->GetaWall(), args->GetbWall(), args->GetcWall() );
        s.append("\tModel: GompertzModel\n");
        s.append(_model->writeParameter());
        break;
    }

    // ODE solver
    int solver = args->GetSolver();
    sprintf(tmp, "\tODE Loeser: %d\n", solver);
    s.append(tmp);
    switch (solver) {
    case 1:
        _solver = new EulerSolver(_model);
        break;
        //case 2:
        //     _solver = new VelocityVerletSolver(_model);
        //     break;
        //case 3:
        //     _solver = new LeapfrogSolver(_model);
        //     break;
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

    for (unsigned int r= 0; r<routers.size(); r++) {

        RoutingStrategy strategy=routers[r].second;

        int routerID=routers[r].first;

        switch (strategy) {
        case ROUTING_LOCAL_SHORTEST: {
            Router* router=new GlobalRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy local shortest added\n");
            break;
        }
        case ROUTING_GLOBAL_SHORTEST: {

            Router* router=new GlobalRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy global shortest added\n");
            break;
        }
        case ROUTING_QUICKEST: {
            Router* router=new QuickestPathRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy quickest path added\n");
            break;
        }
        case ROUTING_DYNAMIC: {
            Router* router=new GraphRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy graph router added\n");
            break;
        }
        case ROUTING_NAV_MESH: {
            Router* router=new MeshRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy nav_mesh  router added\n");
            break;
        }
        case ROUTING_DUMMY: {
            Router* router=new DummyRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy dummy router added\n");
            break;
        }
        case ROUTING_SAFEST: {
            Router * router=new SafestPathRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy cognitive map router added\n");
            break;
        }
        case ROUTING_COGNITIVEMAP: {
            Router* router=new CognitiveMapRouter();
            router->SetID(routerID);
            router->SetStrategy(strategy);
            routingEngine->AddRouter(router);
            s.append("\tRouting Strategy dummy router added\n");
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
    //_building->AddSurroundingRoom();
    _building->InitGeometry(); // create the polygons
    _building->LoadTrafficInfo();
    // in the case the navigation mesh should be written to a file
    if(args->GetNavigationMesh()!="") {
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
    if (args->GetLinkedCells()) {
        s.append("\tusing Linked-Cells for spatial queries\n");
        _building->InitGrid(args->GetLinkedCellSize());
    } else {
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
    //size of the cells/GCFM
    if(args->GetDistEffMaxPed()>args->GetLinkedCellSize()){
        Log->Write(
                "ERROR: the linked-cell size [%f] should be bigger than the force range [%f]",
                args->GetLinkedCellSize(), args->GetDistEffMaxPed());
        exit(EXIT_FAILURE);
    }

    //read the events
    _em = new EventManager(_building);
    _em->SetProjectFilename(args->GetProjectFile());
    _em->SetProjectRootDir(args->GetProjectRootDir());
    _em->readEventsXml();
    _em->listEvents();

    //profiling enabled?
    _profiling = args->GetProfileFlag();
    //which hpc-architecture?
    _hpc = args->GetHPCFlag();
    //if architecture = gpu or xeonphi create buffer
    if(_hpc!=0){
        ((GPU_GCFMModel*) _model)->CreateBuffer(_building->GetNumberOfPedestrians());
        //initCL(_building->GetNumberOfPedestrians(),_hpc);
    }
}


int Simulation::RunSimulation() {

    time_t startinitSim, endinitSim, startfirstUpdate, endfirstUpdate,
            startLoop, endLoop, startSolveODE, endSolveODE, startLoopUpdate,
            endLoopUpdate, startEventUpdate, endEventUpdate;
    double initSimTime, firstUpdateTime, solveODETime = 0.0, loopUpdateTime =
            0.0, eventUpdateTime = 0.0, loopTime, upBuilding = 0.0,
            upPeds = 0.0, upTime = 0.0, upGrid = 0.0;

    if(GetProfileFlag()){
        time(&startinitSim);
    }
    int frameNr = 1; // Frame Number
    int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
    writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
    double t=0.0;

    // writing the header

    _iod->WriteHeader(_nPeds, _fps, _building,_seed);
    _iod->WriteGeometry(_building);
    _iod->WriteFrame(0,_building);

    _iod->WriteHeader(_nPeds, _fps, _building,_seed);
    _iod->WriteGeometry(_building);
    _iod->WriteFrame(0,_building);
    if(GetProfileFlag()){
        time(&endinitSim);
        initSimTime = difftime(endinitSim,startinitSim);
    }
    //first initialisation needed by the linked-cells
    if(GetProfileFlag())
        time(&startfirstUpdate);
    Update(upBuilding,upPeds,upTime,upGrid);
    if(GetProfileFlag()){
        time(&endfirstUpdate);
        firstUpdateTime = difftime(endfirstUpdate,startfirstUpdate);
    }
    // main program loop
    if(GetProfileFlag()){
        time(&startLoop);
        solveODETime = 0.0, loopUpdateTime = 0.0, eventUpdateTime = 0.0;
    }
    cout << "hpcFlag=" << _hpc << endl;
    for (t = 0; t < _tmax && _nPeds > 0; ++frameNr) {
        t = 0 + (frameNr - 1) * _deltaT;
        // solve ODE: berechnet Kräfte und setzt neue Werte für x und v
        if(GetProfileFlag())
            time(&startSolveODE);
        _solver->solveODE(t, t + _deltaT, _building);
        if(GetProfileFlag()){
            time(&endSolveODE);
            solveODETime += difftime(endSolveODE, startSolveODE);
        }

        // gucken ob Fußgänger in neuen Räumen/Unterräum
        if(GetProfileFlag())
            time(&startLoopUpdate);
        Update(upBuilding,upPeds,upTime,upGrid);
        if(GetProfileFlag()){
            time(&endLoopUpdate);
            loopUpdateTime += difftime(endLoopUpdate, startLoopUpdate);
        }

        //Eventupdate
        if(GetProfileFlag())
            time(&startEventUpdate);
        _em->Update_Events(t,_deltaT);
        if(GetProfileFlag()){
            time(&endEventUpdate);
            eventUpdateTime += difftime(endEventUpdate, startEventUpdate);
        }

        // ggf. Ausgabe für TraVisTo
        if (frameNr % writeInterval == 0) {
            _iod->WriteFrame(frameNr / writeInterval, _building);
            //cout << t << endl;
        }

    }
    if(GetProfileFlag()){
        time(&endLoop);
        loopTime = difftime(endLoop,startLoop);
        cout << "Messungen in Simulation.cpp: " << endl;
        cout << "\tInit Sim [s]: " << initSimTime << endl;
        cout << "\tFirst Update [s]: " << firstUpdateTime << endl;
        cout << "\tLoop [s]: " << loopTime << endl;
        cout << "\tMessungen in der Loop: " << endl;
        cout << "\t\tSolve ODE [s]: " << solveODETime << endl;
        cout << "\t\tUpdate [s]: " << loopUpdateTime << endl;
        cout << "\t\tMessungen in der Update(): " << endl;
        cout << "\t\t\tUpdate Building[s]: " << upBuilding << endl;
        cout << "\t\t\tUpdate Pedestrians[s]: " << upPeds << endl;
        cout << "\t\t\tUpdate GlobalTime[s]: " << upTime << endl;
        cout << "\t\t\tUpdate Grid[s]: " << upGrid << endl;
        cout << "\t\tEventUpdate [s]: " << eventUpdateTime << endl;
    }
    if(_hpc!=0)
        ((GPU_GCFMModel*) _model)->DeleteBuffers();
    // writing the footer
    _iod->WriteFooter();

    if(_argsParser->GetFileFormat()==FORMAT_XML_BIN)
    {
        delete _iod;
        _iod=NULL;
        char replace[CLENGTH];
        // open the file and replace the 8th line
        sprintf(replace,"sed -i '9s/.*/ %d /' %s", frameNr/ writeInterval, _argsParser->GetTrajectoriesFile().c_str());
        system(replace);
    }

    //return the evacuation time
    return (int) t;
}


// TODO: make the building class more independent by moving the update routing here.
void Simulation::Update(double &b, double &p, double &t, double &g) {
    time_t startBuilding, endBuilding, startPeds, endPeds, startTime, endTime, startGrid, endGrid;
    //_building->Update();
    if(GetProfileFlag())
        time(&startBuilding);
    _building->UpdateVerySlow();
    if(GetProfileFlag()){
        time(&endBuilding);
        // b += difftime(endBuilding,startBuilding);
        b += endBuilding-startBuilding;
        time(&startPeds);
    }
    //someone might have leave the building
    _nPeds=_building->GetAllPedestrians().size();
    if(GetProfileFlag()){
        time(&endPeds);
        p += difftime(endPeds,startPeds);
        time(&startTime);
    }
    // update the global time
    Pedestrian::SetGlobalTime(Pedestrian::GetGlobalTime()+_deltaT);
    if(GetProfileFlag()){
        time(&endTime);
        t += difftime(endTime,startTime);
        time(&startGrid);
    }
    //update the cells position
    _building->UpdateGrid();
    if(GetProfileFlag()){
        time(&endGrid);
        g += difftime(endGrid,startGrid);
    }
}
//void Simulation::Update()
//{
//     //_building->Update();
//     _building->UpdateVerySlow();
//     //someone might have leave the building
//     _nPeds=_building->GetAllPedestrians().size();
//     // update the global time
//     Pedestrian::SetGlobalTime(Pedestrian::GetGlobalTime()+_deltaT);
//     //update the cells position
//     _building->UpdateGrid();
//
//}

void Simulation::PrintStatistics()
{
    Log->Write("\nEXIT USAGE:");
    const map<int, Transition*>& transitions = _building->GetAllTransitions();
    map<int, Transition*>::const_iterator itr;
    for(itr = transitions.begin(); itr != transitions.end(); ++itr) {
        Transition* goal =  itr->second;
        if(goal->IsExit()) {
            Log->Write("Exit ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",goal->GetID(),goal->GetDoorUsage(),goal->GetLastPassingTime());
        }
    }
}
