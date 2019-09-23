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
#ifndef HYBRID_JPSSERVER_H_
#define HYBRID_JPSSERVER_H_

#include <thread>
#include <iostream>
#include <atomic>

#include <grpc++/grpc++.h>
#include <set>
#include "HybridSimulationManager.h"
#include "hybridsim.grpc.pb.h"
#include "Latches.h"
#include "../general/Configuration.h"
#include "../routing/direction/walking/DirectionStrategy.h"

using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using hybridsim::HybridSimulation;
using hybridsim::Agent;
using hybridsim::Agents;
using hybridsim::Boolean;
using hybridsim::Empty;
using hybridsim::LeftClosedRightOpenTimeInterval;
using hybridsim::Scenario;

class Simulation;

class HybridSimulationManager;

class Configuration;

class JPSserver final : public HybridSimulation::Service {

public:
     /**
     * constructor with an agent source manager, which will be
     * responsible for positioning the agents.
     */
     JPSserver(HybridSimulationManager* hybridSimulationManager, std::shared_ptr<Latches> latches,
               Configuration* configuration);

     /**
      * Destructor
      */
     virtual ~JPSserver();

     void SetSimulation(Simulation& src);

     virtual ::grpc::Status simulatedTimeInerval(::grpc::ServerContext* context,
               const ::hybridsim::LeftClosedRightOpenTimeInterval* request,
               ::hybridsim::Empty* response) override;

     virtual ::grpc::Status transferAgent(::grpc::ServerContext* context, const ::hybridsim::Agent* request,
               ::hybridsim::Boolean* response) override;

     virtual ::grpc::Status receiveTrajectories(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
               ::hybridsim::Trajectories* response) override;

     virtual ::grpc::Status retrieveAgents(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
               ::hybridsim::Agents* response) override;

     virtual ::grpc::Status shutdown(::grpc::ServerContext* context, const ::hybridsim::Empty* request,
               ::hybridsim::Empty* response) override;

     virtual ::grpc::Status initScenario(::grpc::ServerContext* context, const ::hybridsim::Scenario* request,
               ::hybridsim::Empty* response) override;



     //void SetAgentsSourcesManager(const AgentsSourcesManager& src) const;

private:
     std::shared_ptr<Latches> _latches;
     Simulation* _SimManager;
     HybridSimulationManager* _HybridSimulationManager;
     Configuration* _configuration;

     std::map<int, std::string> _mapExtID2JPSID;
     std::map<int, std::string> _mapExtLinkID2JPSRoomID;
     std::set<std::string> _mappedExtLinkIDs;

     inline void AddMappedExtLinkID(std::string linkID)
     {
          _mappedExtLinkIDs.insert(linkID);
     }

     inline bool IsExtLinkIDMapped(std::string linkID)
     {
          return _mappedExtLinkIDs.find(linkID)!=_mappedExtLinkIDs.end();
     }

     inline void MapExtLinkID2JPSRoomID(int jpsRoomID, std::string linkID)
     {
          _mapExtLinkID2JPSRoomID[jpsRoomID] = linkID;
     }

     inline std::string GetLinkID(int jpsRoomID)
     {
          return _mapExtLinkID2JPSRoomID[jpsRoomID];
     }

     inline void MapExtIdToJPSID(int jpsID, std::string matsimID)
     {
          _mapExtID2JPSID[jpsID] = matsimID;
     }

     inline std::string GEtExtId(int jpsID)
     {
          return _mapExtID2JPSID[jpsID];
     }

     std::thread _t;


};

#endif /* HYBRID_JPSSERVER_H_ */
