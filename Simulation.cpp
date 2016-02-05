/**
 * \file        Simulation.cpp
 * \date        Dec 15, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#include "math/GradientModel.h"
#include "pedestrian/AgentsQueue.h"
#include "pedestrian/AgentsSourcesManager.h"

#ifdef _USE_PROTOCOL_BUFFER
#include "matsim/HybridSimulationManager.h"
#endif

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
     _seed = 8091983;
     _deltaT = 0;
     _building = nullptr;
     //_direction = NULL;
     _operationalModel = nullptr;
     _solver = nullptr;
     _iod = new IODispatcher();
     _fps = 1;
     _em = nullptr;
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

     if (args.GetTrajectoriesFile().empty() == false)
     {
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
     sprintf(tmp, "\tt_max: %f\n", args.GetTmax());
     s.append(tmp);
     _deltaT = args.Getdt();
     sprintf(tmp, "\tdt: %f\n", _deltaT);
     _periodic = args.IsPeriodic();
     sprintf(tmp, "\t periodic: %d\n", _periodic);
     s.append(tmp);

     _fps = args.Getfps();
     sprintf(tmp, "\tfps: %f\n", _fps);
     s.append(tmp);
     //Log->Write(s.c_str());

     _routingEngine = args.GetRoutingEngine();
     auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(_argsParser.GetProjectFile(), _argsParser.GetAgentsParameters(),_argsParser.GetSeed()));
     // IMPORTANT: do not change the order in the following..
     _building = std::unique_ptr<Building>(new Building(args.GetProjectFile(), args.GetProjectRootDir(), *_routingEngine, *distributor, args.GetLinkedCellSize()));

     // Initialize the agents sources that have been collected in the pedestrians distributor
     _agentSrcManager.SetBuilding(_building.get());
     for (const auto& src: distributor->GetAgentsSources())
     {
          _agentSrcManager.AddSource(src);
          //src->Dump();
     }

#ifdef _USE_PROTOCOL_BUFFER
     //initialize the hybrid mode if defined
     if(nullptr!=(_hybridSimManager=args.GetHybridSimManager()))
     {
          Log->Write("INFO:\t performing hybrid simulation");
     }
#endif

     //perform customs initialisation, like computing the phi for the gcfm
     //this should be called after the routing engine has been initialised
     // because a direction is needed for this initialisation.
     if(_operationalModel->Init(_building.get())==false)
          return false;

     //other initializations
     for (auto&& ped: _building->GetAllPedestrians()) {
          ped->Setdt(_deltaT);
     }
     _nPeds = _building->GetAllPedestrians().size();
     //_building->WriteToErrorLog();

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
     _em = new EventManager(_building.get(),args.GetSeed());
     if(_em->ReadEventsXml()==false)
     {
          Log->Write("ERROR: \tCould not initialize events handling");
     }
     _em->ListEvents();

     //_building->SaveGeometry("test.sav.xml");

     //if(_building->SanityCheck()==false)
     //     return false;

     //everything went fine
     return true;
}

int Simulation::RunStandardSimulation(double maxSimTime)
{
     RunHeader(_nPeds + _agentSrcManager.GetMaxAgentNumber());
     double t=RunBody(maxSimTime);
     RunFooter();
     return (int)t;
}

void Simulation::UpdateRoutesAndLocations()
{
     //pedestrians to be deleted
     //you should better create this in the constructor and allocate it once.
     vector<Pedestrian*> pedsToRemove;
     pedsToRemove.reserve(500); //just reserve some space

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
                              if (sub->IsDirectlyConnectedWith(old_sub)
                                        && sub->IsInSubRoom(ped->GetPos()))
                              {
                                   ped->SetRoomID(room->GetID(),
                                             room->GetCaption());
                                   ped->SetSubRoomID(sub->GetSubRoomID());
                                   //the agent left the old iroom
                                   //actualize the egress time for that iroom
                                   old_room->SetEgressTime(ped->GetGlobalTime());

                                   //the pedestrian did not used the door to exit the room
                                   //todo: optimize with distance square
                                   //if(ped->GetDistanceToNextTarget()>0.5)
                                   //{
                                   //   Log->Write("WARNING:\t pedestrian [%d] left the room in an unusual way. Please check",ped->GetID());
                                   //   Log->Write("        \t distance to previous target is %f",ped->GetDistanceToNextTarget());
                                   //}

                                   //also statistic for internal doors
                                   UpdateFlowAtDoors(*ped);

                                   ped->ClearMentalMap(); // reset the destination
                                   //ped->FindRoute();

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
                         //actualize the eggress time for that room
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

#ifdef _USE_PROTOCOL_BUFFER
     if (_hybridSimManager)
     {
          AgentsQueueOut::Add(pedsToRemove);
     }
     else
#endif
     {
          // remove the pedestrians that have left the building
          for (unsigned int p = 0; p < pedsToRemove.size(); p++)
          {
               UpdateFlowAtDoors(*pedsToRemove[p]);
               _building->DeletePedestrian(pedsToRemove[p]);
          }
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
     for (const auto& itr : _building->GetAllTransitions())
     {
          Transition* goal = itr.second;
          if (goal->GetDoorUsage())
          {
               Log->Write(
                         "\nExit ID [%d] used by [%d] pedestrians. Last passing time [%0.2f] s",
                         goal->GetID(), goal->GetDoorUsage(),
                         goal->GetLastPassingTime());

               string statsfile=_argsParser.GetTrajectoriesFile()+"_flow_exit_id_"+to_string(goal->GetID())+".dat";
               Log->Write("More Information in the file: %s",statsfile.c_str());
               auto output= new FileHandler(statsfile.c_str());
               output->Write("#Flow at exit "+goal->GetCaption()+"( ID "+to_string(goal->GetID())+" )");
               output->Write("#Time (s)  cummulative number of agents \n");
               output->Write(goal->GetFlowCurve());
          }
     }
     Log->Write("\n");
}

void Simulation::RunHeader(long nPed)
{
     // writing the header
     if(nPed==-1) nPed=_nPeds;
     _iod->WriteHeader(nPed, _fps, _building.get(), _seed);
     _iod->WriteGeometry(_building.get());

     int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
     writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
     int firstframe=(Pedestrian::GetGlobalTime()/_deltaT)/writeInterval;

     _iod->WriteFrame(firstframe, _building.get());

     //first initialisation needed by the linked-cells
     UpdateRoutesAndLocations();
     ProcessAgentsQueue();
}

int Simulation::RunBody(double maxSimTime)
{
     //needed to control the execution time PART 1
     //in the case you want to run in no faster than realtime
     //time_t starttime, endtime;
     //time(&starttime);

     //take the current time from the pedestrian
     double t=Pedestrian::GetGlobalTime();

     //frame number. This function can be called many times,
     static int frameNr = 1 + t/_deltaT ; // Frame Number

     int writeInterval = (int) ((1. / _fps) / _deltaT + 0.5);
     writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0


     //process the queue for incoming pedestrians
     //important since the number of peds is used
     //to break the main simulation loop
     ProcessAgentsQueue();
     _nPeds = _building->GetAllPedestrians().size();
     int initialnPeds = _nPeds; 
     // main program loop
     while ( (_nPeds || !_agentSrcManager.IsCompleted() ) && t < maxSimTime)
     {
          t = 0 + (frameNr - 1) * _deltaT;

          //process the queue for incoming pedestrians
          ProcessAgentsQueue();

          if(t>Pedestrian::GetMinPremovementTime())
          {
               //update the linked cells
               _building->UpdateGrid();

               // update the positions
               _operationalModel->ComputeNextTimeStep(t, _deltaT, _building.get(), _periodic);

               //update the events
               _em->ProcessEvent();

               //update the routes and locations
               UpdateRoutesAndLocations();

               //other updates
               //someone might have left the building
               _nPeds = _building->GetAllPedestrians().size();
          }

          // update the global time
          Pedestrian::SetGlobalTime(t);

          // write the trajectories
          if (0 == frameNr % writeInterval) {
               _iod->WriteFrame(frameNr / writeInterval, _building.get());
          }
          Log->ProgressBar(initialnPeds,   initialnPeds -  _nPeds , t);

          // needed to control the execution time PART 2
          // time(&endtime);
          // double timeToWait=t-difftime(endtime, starttime);
          // clock_t goal = timeToWait*1000 + clock();
          // while (goal > clock());
          ++frameNr;
     }
     return (int) t;
}

void Simulation::RunFooter()
{
     // writing the footer
     _iod->WriteFooter();
}

void Simulation::ProcessAgentsQueue()
{
     //incoming pedestrians
     vector<Pedestrian*> peds;
     AgentsQueueIn::GetandClear(peds);
     for(auto&& ped: peds)
     {
          _building->AddPedestrian(ped);
     }

#ifdef _USE_PROTOCOL_BUFFER
     //outgoing pedestrians
     if (_hybridSimManager)
     {
          _hybridSimManager->ProcessOutgoingAgent();
     }
#endif
}

void Simulation::UpdateFlowAtDoors(const Pedestrian& ped) const
{
     if(_argsParser.ShowStatistics())
     {
          Transition* trans =_building->GetTransitionByUID(ped.GetExitIndex());
          if(trans)
          {
               //check if the pedestrian left the door correctly
               if(ped.GetExitLine()->DistTo(ped.GetPos())>0.5)
               {
                    Log->Write("WARNING:\t pedestrian [%d] left room/subroom [%d/%d] in an unusual way. Please check",ped.GetID(), ped.GetRoomID(), ped.GetSubRoomID());
                    Log->Write("       :\t distance to last door (%d | %d) is %f. That should be smaller.", ped.GetExitLine()->GetUniqueID(), ped.GetExitIndex(), ped.GetExitLine()->DistTo(ped.GetPos()));
                    Log->Write("       :\t correcting the door statistics");
                    //ped.Dump(ped.GetID());

                    //checking the history and picking the nearest previous destination
                    double biggest=0.3;
                    bool success=false;
                    for(const auto & dest:ped.GetLastDestinations())
                    {
                         if(dest!=-1)
                         {
                              Transition* trans_tmp =_building->GetTransitionByUID(dest);
                              if(trans_tmp&&trans_tmp->DistTo(ped.GetPos())<biggest)
                              {
                                   biggest=trans_tmp->DistTo(ped.GetPos());
                                   trans=trans_tmp;
                                   Log->Write("       :\t Best match found at door %d",dest);
                                   success=true;//at least one door was found
                              }
                         }
                    }

                    if(success==false)
                    {
                         Log->Write("WARNING       :\t correcting the door statistics");
                         return; //todo we need to check if the ped is in a subroom neighboring the target. If so, no problems!
                    }
               }
               trans->IncreaseDoorUsage(1, ped.GetGlobalTime());
          }
     }
}

AgentsSourcesManager& Simulation::GetAgentSrcManager()
{
     return _agentSrcManager;
}

Building* Simulation::GetBuilding()
{
     return _building.get();
}
