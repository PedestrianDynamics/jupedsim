/**
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
 **/
//
// Created by laemmel on 26.02.16.
//


#include <cstdlib>

#include <hybridsim.pb.h>
#include "JPSserver.h"

#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/AgentsSource.h"
#include "../pedestrian/Pedestrian.h"
#include "../Simulation.h"

#include "../pedestrian/AgentsQueue.h"
#include "../math/GompertzModel.h"
#include "IniFromProtobufLoader.h"

// external variables
extern OutputHandler* Log;

JPSserver::JPSserver(HybridSimulationManager* hsm, std::shared_ptr<Latches> latches, Configuration* configuration)
          :_HybridSimulationManager(hsm), _latches(latches), _configuration(configuration)
{
}

JPSserver::~JPSserver()
{

}

void JPSserver::SetSimulation(Simulation& src)
{
     _SimManager = &src;
}

grpc::Status JPSserver::simulatedTimeInerval(::grpc::ServerContext* context,
          const ::hybridsim::LeftClosedRightOpenTimeInterval* request,
          ::hybridsim::Empty* response)
{

     double from = request->fromtimeincluding();
     double to = request->totimeexcluding();
//    Log->Write("INFO:\tRPC::JPSserver Simulation step from %.2f to %.2f seconds", from, to);

     _SimManager->RunBody(to);

     return Status::OK;
}

::grpc::Status JPSserver::transferAgent(::grpc::ServerContext* context, const ::hybridsim::Agent* request,
          ::hybridsim::Boolean* response)
{

     std::string agent_id = request->id();

     hybridsim::Coordinate enterC = request->enterlocation();
     hybridsim::Coordinate leaveC = request->leavelocation();
     Point leaveP(leaveC.x(), leaveC.y());

     auto& agentSrcMng = _SimManager->GetAgentSrcManager();
     for (const hybridsim::Link& l : request->leg().link()) {
          if (IsExtLinkIDMapped(l.id())) {
               continue;
          }

          bool fnd = false;
          Point p = Point(l.centroid().x(), l.centroid().y());
          Building* bld = agentSrcMng.GetBuilding();
          for (auto&& rm : bld->GetAllRooms()) {//TODO: store rooms in a more adequate data structure to avoid linear search (e.g. quadtree)
               for (auto&& sr : rm.second->GetAllSubRooms()) {
                    if (sr.second->IsInSubRoom(p)) {
                         int rid = rm.second->GetID();
                         MapExtLinkID2JPSRoomID(rid, l.id());
                         fnd = true;
                         break;
                    }
               }
               if (fnd) {
                    break;
               }
          }
          if (!fnd) {
               Log->Write("ERROR:\tcould not find JPS room for link with ID: "+l.id());
               exit(EXIT_FAILURE);
          }

     }



     //TODO: check whether the requested node has space, e.g. w/ voronoi method
     response->set_val(true);//for now JPS has always space for one more agent

     const map<int, Goal*>& goals = _SimManager->GetBuilding()->GetAllGoals();

     auto srcs = agentSrcMng.GetSources();

     for (auto&& src:srcs) //TODO: linear search is inefficient, replace it by a map
     {
          auto dist = src->GetStartDistribution();
          double bounds[4] = {0, 0, 0, 0};
          dist->Getbounds(bounds);
          if (enterC.x()>bounds[0] && enterC.x()<bounds[1] && enterC.y()>bounds[2] && enterC.y()<bounds[3]) {
               std::vector<Pedestrian*> peds;
               src->GenerateAgents(peds, 1, agentSrcMng.GetBuilding());

               bool found = false;
               //there should be only one agent in this vector
               for (auto&& ped:peds) {
                    //TODO: there might be a race condition here if the client is sending agents out at the same time
                    //TODO: map the agents back, not necessary if jupedsim is reset after each iteration
                    // because each incoming agent has an id and an agent will get two
                    MapExtIdToJPSID(ped->GetID(), agent_id);
                    bool foundGoal = false;
                    for (auto&& goal : goals) { //TODO: replace linear search by smthng more efficient
                         if (goal.second->Contains(leaveP)) {
                              ped->SetFinalDestination(goal.second->GetId());
                              foundGoal = true;
                              if (_configuration->GetDumpScenario()) {
                                   _HybridSimulationManager->GetSimObserver()->AddPedestrian(src, goal.second->GetId());
                              }
                              break;
                         }

                    }
                    if (!foundGoal) {
                         Log->Write("ERROR:\tCould not find goal for pedestrian: "+agent_id);
                         exit(-1);
                    }

                    //schedule the agent
                    src->AddToPool(ped);
                    found = true;
               }
               if (found) {
                    break;
               }

          }
     }

     agentSrcMng.ProcessAllSources();
     return ::grpc::Status::OK;
}

::grpc::Status JPSserver::receiveTrajectories(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
          ::hybridsim::Trajectories* response)
{

     Building* building = _SimManager->GetBuilding();

     auto&& allPeds = building->GetAllPedestrians();
     for (const auto& ped:allPeds) {
          std::string extId = GEtExtId(ped->GetID());
          if (extId=="") {
               Log->Write(
                         "WARNING:\tzombi detected"); //this happens if there are agents in the simulation that stem not from
               //an external source (e.g. SUMO). We need to decided whether we want to allow this at all (e.g. as background noise) - gl Mar'16
               continue;
          }

          double angle = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi());
          double phi = angle>M_PI ? angle-2*M_PI : angle;
          auto V = ped->GetV();
          double spd = std::sqrt(V._x*V._x+V._y*V._y);
          auto&& tr = response->add_trajectories();

          tr->set_id(extId);
          tr->set_phi(phi);
          tr->set_x(ped->GetPos()._x);
          tr->set_y(ped->GetPos()._y);
          tr->set_spd(spd);
          tr->set_linkid(GetLinkID(ped->GetRoomID()));
     }

     return ::grpc::Status::OK;
}

::grpc::Status JPSserver::retrieveAgents(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
          ::hybridsim::Agents* response)
{

     Building* building = _SimManager->GetBuilding();
     std::vector<Pedestrian*> peds;
     AgentsQueueOut::GetandClear(peds);
     for (auto&& ped:peds) {
          std::string extId = GEtExtId(ped->GetID());
          if (extId=="") {
               Log->Write(
                         "WARNING:\tzombi detected");//this happens if there are agents in the simulation that stem not from
               //an external source (e.g. SUMO). We need to decided whether we want to allow this at all (e.g. as background noise) - gl Mar'16
               continue;
          }

          Agent* agent = response->add_agents();
          agent->set_id(extId);//TODO: remove sumo<-->jps id mapping after ped has left jps
          building->DeletePedestrian(ped);
#ifdef  _DBG_JPS_AS_A_SERVICE
          Log->Write("INFO:\tagent: "+extId+" is leaving JPS");
#endif
     }

     return ::grpc::Status::OK;
}

::grpc::Status JPSserver::shutdown(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
          ::hybridsim::Empty* response)
{

     std::thread t(&HybridSimulationManager::Shutdown, _HybridSimulationManager);
     t.detach();
     _latches->SimulationFinished();
     return grpc::Status::OK;
}

::grpc::Status JPSserver::initScenario(::grpc::ServerContext* context, const ::hybridsim::Scenario* request,
          ::hybridsim::Empty* response)
{

     //TODO: no error handling thus far!!
     IniFromProtobufLoader loader(_configuration);

     loader.Initialize(request);

     _configuration->SetScenario(request);
     _latches->ScenarioLoaded();
     _latches->WaitForSimulationPrepared();
     return grpc::Status::OK;
}












