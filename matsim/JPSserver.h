/**
 * \file        JPSserver.h
 * \date        Apr 21, 2015
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
 * This class implements the remote procedure calls methods defined in the matsim.proto interface.
 * These methods are then called by the remote program via the channel created.
 *
 **/

#ifndef MATSIM_JPSSERVER_H_
#define MATSIM_JPSSERVER_H_

#include <iostream>
#include <atomic>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>
#include "MATSimInterface.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using namespace org::matsim::hybrid;

//forward declarations
class Simulation;
class JPSclient;

class JPSserver final : public ExternInterfaceService::Service
{

public:
     /**
      * constructor with an agent source manager, which will be
      * responsible for positioning the agents.
      */
     JPSserver(Simulation& src);

     /**
      * Destructor
      */
     virtual ~JPSserver();

     /// The stub for these functions are automatically generated from the grpc module
     virtual Status reqMATSim2ExternHasSpace(ServerContext* context, const MATSim2ExternHasSpace* request, MATSim2ExternHasSpaceConfirmed* response);
     virtual Status reqMATSim2ExternPutAgent(ServerContext* context, const MATSim2ExternPutAgent* request, MATSim2ExternPutAgentConfirmed* response);
     virtual Status reqExternDoSimStep(ServerContext* context, const ExternDoSimStep* request, ExternDoSimStepReceived* response);
     virtual Status reqExternOnPrepareSim(ServerContext* context, const ExternOnPrepareSim* request, ExternOnPrepareSimConfirmed* response);
     virtual Status reqExternAfterSim(ServerContext* context, const ExternAfterSim* request, ExternAfterSimConfirmed* response);

     void RunSimulation();

     void SetDuplexClient(std::shared_ptr<JPSclient>& client);
     //void SetAgentsSourcesManager(const AgentsSourcesManager& src) const;

private:
    Simulation& _SimManager;
    bool _shutdown=false;
    std::atomic<bool> _doSimulation;
    std::atomic<double> _maxSimTime;
    std::shared_ptr<JPSclient> _jpsClient;
    int _maxAgents=0;
    //std::map<std::string, int> _mapMatsimID2JPSID;
};

#endif /* MATSIM_JPSSERVER_H_ */
