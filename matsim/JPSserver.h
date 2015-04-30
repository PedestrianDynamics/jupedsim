/*
 * JPSserver.h
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#ifndef MATSIM_JPSSERVER_H_
#define MATSIM_JPSSERVER_H_

#include <iostream>

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
using namespace hybrid;

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
     JPSserver(Simulation& src, const std::string& connection);

     /**
      * Destructor
      */
     virtual ~JPSserver();

     virtual Status reqMATSim2ExternHasSpace(ServerContext* context, const MATSim2ExternHasSpace* request, MATSim2ExternHasSpaceConfirmed* response);
     virtual Status reqMATSim2ExternPutAgent(ServerContext* context, const MATSim2ExternPutAgent* request, MATSim2ExternPutAgentConfirmed* response);
     virtual Status reqExternDoSimStep(ServerContext* context, const ExternDoSimStep* request, ExternDoSimStepReceived* response);
     virtual Status reqExternOnPrepareSim(ServerContext* context, const ExternOnPrepareSim* request, ExternOnPrepareSimConfirmed* response);
     virtual Status reqExternAfterSim(ServerContext* context, const ExternAfterSim* request, ExternAfterSimConfirmed* response);


     bool NotifyExternalService();

     void RunSimulation();


     //void SetDuplexClient(std::unique_ptr<JPSclient>& client);
     //void SetAgentsSourcesManager(const AgentsSourcesManager& src) const;

private:
    Simulation& _SimManager;
    bool _doSimulation=false;
    std::unique_ptr<JPSclient> _rpcClient;
};


class MATSIMserver final : public MATSimInterfaceService::Service
{
    virtual Status reqExternalConnect(::grpc::ClientContext* context, const ::hybrid::ExternalConnect& request, ::hybrid::ExternalConnectConfirmed* response)
    {
         std::cout<<"INFO:\tRPC::JPSserver I have space on node "<<std::endl;

         return Status::OK;
    }

};

#endif /* MATSIM_JPSSERVER_H_ */
