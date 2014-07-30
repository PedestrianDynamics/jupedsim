/**
 * \file        Simulation.cpp
 * \date        Dec 15, 2010
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * The Simulation class represents a simulation of pedestrians
 * based on a certain model in a specific scenario. A simulation is defined by
 * various parameters and functions.
 *
 *
 **/


#include "Simulation.h"

#include "math/GCFMModel.h"
#include "math/GompertzModel.h"

using namespace std;

OutputHandler* Log;

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
     _argsParser=NULL;
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
               OutputHandler* travisto = new SocketHandler(args->GetHostname(), args->GetPort());
               Trajectories* output= new TrajectoriesJPSV06();
               output->SetOutputHandler(travisto);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_XML_BIN: {
               Log->Write("INFO: \tFormat xml-bin not yet supported in streaming\n");
               //exit(0);
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
               Trajectories* output= new TrajectoriesJPSV05();
               output->SetOutputHandler(tofile);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_PLAIN: {
               OutputHandler* file = new FileHandler(args->GetTrajectoriesFile().c_str());
               Trajectories* output= new  TrajectoriesFLAT();
               output->SetOutputHandler(file);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_VTK: {
               Log->Write("INFO: \tFormat vtk not yet supported\n");
               OutputHandler* file = new FileHandler((args->GetTrajectoriesFile() +".vtk").c_str());
               Trajectories* output= new  TrajectoriesVTK();
               output->SetOutputHandler(file);
               _iod->AddIO(output);
               break;
          }

          case FORMAT_XML_PLAIN_WITH_MESH: {
               //OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
               //if(_iod) delete _iod;
               //_iod = new TrajectoriesXML_MESH();
               //_iod->AddIO(tofile);
               break;
          }
          case FORMAT_XML_BIN: {
               // OutputHandler* travisto = new SocketHandler(args->GetHostname(), args->GetPort());
               // Trajectories* output= new TrajectoriesJPSV06();
               // output->SetOutputHandler(travisto);
               // _iod->AddIO(output);
               break;
          }
          }
     }

     _distribution = new PedDistributor();
     _distribution->InitDistributor(_argsParser);
     //s.append(_distribution->writeParameter());

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
     int model =  args->GetModel();
     if(model == 1) { //GCFM
          _model = new GCFMModel(_direction, args->GetNuPed(), args->GetNuWall(), args->GetDistEffMaxPed(),
                                 args->GetDistEffMaxWall(), args->GetIntPWidthPed(), args->GetIntPWidthWall(),
                                 args->GetMaxFPed(), args->GetMaxFWall());
          s.append("\tModel: GCFMModel\n");
          s.append(_model->writeParameter());
     } else if (model == 2) { //Gompertz
          _model = new GompertzModel(_direction, args->GetNuPed(), args->GetaPed(), args->GetbPed(), args->GetcPed(),
                                     args->GetNuWall(), args->GetaWall(), args->GetbWall(), args->GetcWall() );
          s.append("\tModel: GompertzModel\n");
          s.append(_model->writeParameter());
     }
     // ODE solver
     int solver = args->GetSolver();
     sprintf(tmp, "\tODE Solver: %d\n", solver);
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
     sprintf(tmp, "\tnCPU: %d\n", args->GetMaxOpenMPThreads());
     s.append(tmp);
     _tmax = args->GetTmax();
     sprintf(tmp, "\tt_max: %f\n", _tmax);
     s.append(tmp);
     _deltaT = args->Getdt();
     sprintf(tmp, "\tdt: %f\n", _deltaT);
     s.append(tmp);

     _fps=args->Getfps();
     sprintf(tmp, "\tfps: %f\n", _fps);
     s.append(tmp);

     // Route choice
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
     //size of the cells/GCFM/Gompertz
     if(args->GetDistEffMaxPed()>args->GetLinkedCellSize()){
         Log->Write("ERROR: the linked-cell size [%f] should be bigger than the force range [%f]",args->GetLinkedCellSize(),args->GetDistEffMaxPed());
         exit(EXIT_FAILURE);
     }

     //read the events
     _em = new EventManager(_building);
     _em->SetProjectFilename(args->GetProjectFile());
     _em->SetProjectRootDir(args->GetProjectRootDir());
     _em->readEventsXml();
     _em->listEvents();
}


int Simulation::RunSimulation()
{
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
          // solve ODE
          _solver->solveODE(t, t + _deltaT, _building);
          // update and check if pedestrians change rooms
          Update();
          _em->Update_Events(t,_deltaT);
          // trajectories output
          if (frameNr % writeInterval == 0) {
               _iod->WriteFrame(frameNr / writeInterval, _building);
          }

     }
     // writing the footer
     _iod->WriteFooter();


     if(_argsParser->GetFileFormat()==FORMAT_XML_BIN) {

          delete _iod;
          _iod=NULL;

//              char tmp[CLENGTH];
//              int f= frameNr / writeInterval ;
//              sprintf(tmp,"<frameCount>%07d</frameCount>",f);
//              string frameCount (tmp);

          char replace[CLENGTH];
          // open the file and replace the 8th line
          sprintf(replace,"sed -i '9s/.*/ %d /' %s", frameNr/ writeInterval, _argsParser->GetTrajectoriesFile().c_str());
          system(replace);
     }

     //return the evacuation time
     return (int) t;
}


// TODO: make the building class more independent by moving the update routing here.
void Simulation::Update()
{
     //_building->Update();
     _building->UpdateVerySlow();
     //someone might have leave the building
     _nPeds=_building->GetAllPedestrians().size();
     // update the global time
     Pedestrian::SetGlobalTime(Pedestrian::GetGlobalTime()+_deltaT);
     //update the cells position
     _building->UpdateGrid();

}

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
