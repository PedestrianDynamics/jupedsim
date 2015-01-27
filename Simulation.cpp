/**
 * \file        Simulation.cpp
 * \date        Dec 15, 2010
 * \version     v0.6
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
//#include "geometry/Goal.h"

#ifdef _OPENMP
#include <omp.h>

#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

using namespace std;

OutputHandler* Log;

Simulation::Simulation(const ArgumentParser& args)
{
     _nPeds = 0;
     _tmax = 0;
     _seed = 8091983;
     _deltaT = 0;
     _building = nullptr;
     //_direction = NULL;
     _operationalModel = nullptr;
     _solver = nullptr;
     _iod = new IODispatcher();
     _fps = 1;
     _em = nullptr;
     _hpc = -1;
     _profiling = false;
     _argsParser = args;
}

Simulation::~Simulation()
{
     delete _solver;
     delete _iod;
     delete _em;
}

long Simulation::GetPedsNumber() const
{
     return _nPeds;
}

bool Simulation::InitArgs(const ArgumentParser& args)
{
     char tmp[CLENGTH];
     string s = "Parameter:\n";

     switch (args.GetLog())
     {
     case 0:
          // no log file
          //Log = new OutputHandler();
          break;
     case 1:
          if (Log)
               delete Log;
          Log = new STDIOHandler();
          break;
     case 2: {
          char name[CLENGTH] = "";
          sprintf(name, "%s.P0.dat", args.GetErrorLogFile().c_str());
          if (Log)
               delete Log;
          Log = new FileHandler(name);
     }
          break;
     default:
          printf("Wrong option for Logfile!\n\n");
          return false;
     }

     if (args.GetPort() != -1) {
          switch (args.GetFileFormat())
          {
          case FORMAT_XML_PLAIN_WITH_MESH:
          case FORMAT_XML_PLAIN: {
               OutputHandler* travisto = new SocketHandler(args.GetHostname(),
                         args.GetPort());
               Trajectories* output = new TrajectoriesJPSV06();
               output->SetOutputHandler(travisto);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_XML_BIN: {
               Log->Write(
                         "INFO: \tFormat xml-bin not yet supported in streaming\n");
               //exit(0);
               break;
          }
          case FORMAT_PLAIN: {
               Log->Write(
                         "INFO: \tFormat plain not yet supported in streaming\n");
               return false;
          }
          case FORMAT_VTK: {
               Log->Write(
                         "INFO: \tFormat vtk not yet supported in streaming\n");
               return false;
          }
              default: {
                  return false;
              }
          }

          s.append("\tonline streaming enabled \n");
     }

     if (args.GetTrajectoriesFile().empty() == false) {
          switch (args.GetFileFormat())
          {
          case FORMAT_XML_PLAIN: {
               OutputHandler* tofile = new FileHandler(
                         args.GetTrajectoriesFile().c_str());
               Trajectories* output = new TrajectoriesJPSV05();
               output->SetOutputHandler(tofile);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_PLAIN: {
               OutputHandler* file = new FileHandler(
                         args.GetTrajectoriesFile().c_str());
               Trajectories* output = new TrajectoriesFLAT();
               output->SetOutputHandler(file);
               _iod->AddIO(output);
               break;
          }
          case FORMAT_VTK: {
               Log->Write("INFO: \tFormat vtk not yet supported\n");
               OutputHandler* file = new FileHandler(
                         (args.GetTrajectoriesFile() + ".vtk").c_str());
               Trajectories* output = new TrajectoriesVTK();
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
              default: {
                  break;
              }
          }
     }



     _operationalModel = args.GetModel();
     s.append(_operationalModel->GetDescription());

     // ODE solver which is never used!
     auto solver = args.GetSolver();
     sprintf(tmp, "\tODE Solver: %d\n", solver);
     s.append(tmp);

     sprintf(tmp, "\tnCPU: %d\n", args.GetMaxOpenMPThreads());
     s.append(tmp);
     _tmax = args.GetTmax();
     sprintf(tmp, "\tt_max: %f\n", _tmax);
     s.append(tmp);
     _deltaT = args.Getdt();
     sprintf(tmp, "\tdt: %f\n", _deltaT);
     s.append(tmp);

     _fps = args.Getfps();
     sprintf(tmp, "\tfps: %f\n", _fps);
     s.append(tmp);

     _routingEngine = args.GetRoutingEngine();
     auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(_argsParser.GetProjectFile(), _argsParser.GetAgentsParameters(),_argsParser.GetSeed()));
     // IMPORTANT: do not change the order in the following..
     _building = std::unique_ptr<Building>(new Building(args.GetProjectFile(), args.GetProjectRootDir(), *_routingEngine, *distributor, args.GetLinkedCellSize()));

     //perform customs initialisation, like computing the phi for the gcfm
     //this should be called after the routing engine has been initialised
     // because a direction is needed for this initialisation.
     if(_operationalModel->Init(_building.get())==false)
          return false;

     //other initializations
     const vector<Pedestrian*>& allPeds = _building->GetAllPedestrians();
     for (Pedestrian *ped : allPeds) {
          ped->Setdt(_deltaT);
     }
     _nPeds = allPeds.size();
     //pBuilding->WriteToErrorLog();

     //get the seed
     _seed = args.GetSeed();

     //size of the cells/GCFM/Gompertz
     if (args.GetDistEffMaxPed() > args.GetLinkedCellSize()) {
          Log->Write(
                    "ERROR: the linked-cell size [%f] should be bigger than the force range [%f]",
                    args.GetLinkedCellSize(), args.GetDistEffMaxPed());
          return false;
     }

     //read and initialize events
     _em = new EventManager(_building.get());
     if(_em->ReadEventsXml()==false)
     {
          Log->Write("ERROR: \tCould not initialize events handling");
     }
     _em->ListEvents();

     //which hpc-architecture?
     _hpc = args.GetHPCFlag();
     //if programming model = ocl create buffers and make the setup
     //if(_hpc==1){
     //((GPU_ocl_GCFMModel*) _model)->CreateBuffer(_building->GetNumberOfPedestrians());
     //((GPU_ocl_GCFMModel*) _model)->initCL(_building->GetNumberOfPedestrians());
     //}
     //_building->SaveGeometry("test.sav.xml");

     //if(_building->SanityCheck()==false)
     //     return false;

     //everything went fine
     return true;
}

int Simulation::RunSimulation()
{
     int frameNr = 1; // Frame Number
     int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
     writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
     double t = 0.0;

     // writing the header
     _iod->WriteHeader(_nPeds, _fps, _building.get(), _seed);
     _iod->WriteGeometry(_building.get());
     _iod->WriteFrame(0, _building.get());

     //first initialisation needed by the linked-cells
     UpdateRoutesAndLocations();

     //needed to control the execution time PART 1
     //in the case you want to run in no faster than realtime
     //time_t starttime, endtime;
     //time(&starttime);

     // main program loop
     while (_nPeds > 0 && t < _tmax)
     {
          t = 0 + (frameNr - 1) * _deltaT;

          // update the positions
          _operationalModel->ComputeNextTimeStep(t, _deltaT, _building.get());

          //update the routes and locations
          UpdateRoutesAndLocations();

          //update the events
          //_em->Update_Events(t);
          _em->ProcessEvent();

          //other updates
          //someone might have left the building
          _nPeds = _building->GetAllPedestrians().size();

          //update the linked cells
          _building->UpdateGrid();

          // update the global time
          Pedestrian::SetGlobalTime(t);

          // write the trajectories
          if (0 == frameNr % writeInterval) {
               _iod->WriteFrame(frameNr / writeInterval, _building.get());
          }

          // needed to control the execution time PART 2
          // time(&endtime);
          // double timeToWait=t-difftime(endtime, starttime);
          // clock_t goal = timeToWait*1000 + clock();
          // while (goal > clock());
         ++frameNr;

     }
     // writing the footer
     _iod->WriteFooter();

     //if(_hpc==1)
     //  ((GPU_GCFMModel*) _model)->DeleteBuffers();

     //temporary work around since the total number of frame is only available at the end of the simulation.
     if (_argsParser.GetFileFormat() == FORMAT_XML_BIN)
     {
          delete _iod;
          _iod = NULL;
          //reopen the file and write the missing information

          // char tmp[CLENGTH];
          // int f= frameNr / writeInterval ;
          // sprintf(tmp,"<frameCount>%07d</frameCount>",f);
          // string frameCount (tmp);

          char replace[CLENGTH];
          // open the file and replace the 8th line
          sprintf(replace, "sed -i '9s/.*/ %d /' %s", frameNr / writeInterval,
                    _argsParser.GetTrajectoriesFile().c_str());
          int result = system(replace);
          Log->Write("INFO:\t Updating the framenumber exits with code [%d]",
                    result);
     }

     //return the evacuation time
     return (int) t;
}

void Simulation::UpdateRoutesAndLocations()
{
     //pedestrians to be deleted
     //you should better create this in the constructor and allocate it once.
     vector<Pedestrian*> pedsToRemove;
     pedsToRemove.reserve(100); //just reserve some space

     // collect all pedestrians in the simulation.
     const vector<Pedestrian*>& allPeds = _building->GetAllPedestrians();
     const map<int, Goal*>& goals = _building->GetAllGoals();

     unsigned long nSize = allPeds.size();
     int nThreads = omp_get_max_threads();
     int partSize = nSize / nThreads;

#pragma omp parallel  default(shared) num_threads(nThreads)
     {
          const int threadID = omp_get_thread_num();
          int start = threadID * partSize;
          int end = (threadID + 1) * partSize - 1;
          if ((threadID == nThreads - 1))
               end = nSize - 1;

          for (int p = start; p <= end; ++p) {
               Pedestrian* ped = allPeds[p];
               Room* room = _building->GetRoom(ped->GetRoomID());
               SubRoom* sub0 = room->GetSubRoom(ped->GetSubRoomID());

               //set the new room if needed
               if ((ped->GetFinalDestination() == FINAL_DEST_OUT)
                         && (room->GetCaption() == "outside")) {
#pragma omp critical
                    pedsToRemove.push_back(ped);
               } else if ((ped->GetFinalDestination() != FINAL_DEST_OUT)
                         && (goals.at(ped->GetFinalDestination())->Contains(
                                   ped->GetPos()))) {
#pragma omp critical
                    pedsToRemove.push_back(ped);
               }

               // reposition in the case the pedestrians "accidently left the room" not via the intended exit.
               // That may happen if the forces are too high for instance
               // the ped is removed from the simulation, if it could not be reassigned
               else if (!sub0->IsInSubRoom(ped))
               {
                    bool assigned = false;
                    auto& allRooms = _building->GetAllRooms();

                    for (auto&& it_room : allRooms)
                    {
                         auto&& room=it_room.second;
                         for (auto&& it_sub : room->GetAllSubRooms())
                         {
                              auto&& sub=it_sub.second;
                              auto&& old_room =allRooms.at(ped->GetRoomID());
                              auto&& old_sub =old_room->GetSubRoom(
                                        ped->GetSubRoomID());
                              if ((sub->IsInSubRoom(ped->GetPos()))
                                        && (sub->IsDirectlyConnectedWith(
                                                  old_sub)))
                              {
                                   ped->SetRoomID(room->GetID(),
                                             room->GetCaption());
                                   ped->SetSubRoomID(sub->GetSubRoomID());
                                   ped->ClearMentalMap(); // reset the destination
                                   //ped->FindRoute();

                                   //the agent left the old iroom
                                   //actualize the egress time for that iroom
                                   old_room->SetEgressTime(ped->GetGlobalTime());

                                   assigned = true;
                                   break;
                              }
                         }
                         if (assigned)
                              break; // stop the loop
                    }

                    if (!assigned) {
#pragma omp critical
                         pedsToRemove.push_back(ped);
                         //the agent left the old room
                         //actualize the egress time for that room
                         allRooms.at(ped->GetRoomID())->SetEgressTime(ped->GetGlobalTime());

                    }
               }

               //finally actualize the route
               if (ped->FindRoute() == -1) {
                    //a destination could not be found for that pedestrian
                    Log->Write("ERROR: \tCould not find a route for pedestrian %d",ped->GetID());
                    //exit(EXIT_FAILURE);
#pragma omp critical
                    pedsToRemove.push_back(ped);
               }
          }
     }

     // remove the pedestrians that have left the building
     for (unsigned int p = 0; p < pedsToRemove.size(); p++) {
          _building->DeletePedestrian(pedsToRemove[p]);
     }

     //    temporary fix for the safest path router
     //    if (dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1)))
     //    {
     //         SafestPathRouter* spr = dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1));
     //         spr->ComputeAndUpdateDestinations(_allPedestians);
     //    }
}

void Simulation::PrintStatistics()
{
     Log->Write("\nRooms Egress Time:");
     Log->Write("==================");
     Log->Write("id\tcaption\tegress time (s)");

     for(const auto& it:_building->GetAllRooms())
     {
          auto&& room=it.second;
          if(room->GetCaption()!="outside")
          Log->Write("%d\t%s\t%.2f",room->GetID(),room->GetCaption().c_str(),room->GetEgressTime());
     }

     Log->Write("\nUsage of Exits");
     Log->Write("==========");
     for (const auto& itr : _building->GetAllTransitions()) {
          Transition* goal = itr.second;
          if (goal->IsExit()) {
               Log->Write(
                         "Exit ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                         goal->GetID(), goal->GetDoorUsage(),
                         goal->GetLastPassingTime());
          }
     }

     Log->Write("\n");
}
