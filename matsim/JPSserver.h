/*
 * JPSserver.h
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

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
