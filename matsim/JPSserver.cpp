/*
 * JPSserver.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#include "JPSserver.h"
#include "../IO/OutputHandler.h"


#include <iostream>

// external variables
extern OutputHandler* Log;


using namespace std;


JPSserver::JPSserver()
{
}

JPSserver::~JPSserver()
{
}

Status JPSserver::reqMATSim2ExternHasSpace(ServerContext* context,
          const MATSim2ExternHasSpace* request,
          MATSim2ExternHasSpaceConfirmed* response)
{
     string nodeID=request->nodeid();
     Log->Write("INFO:\tRPC::JPSserver I have space on node " + nodeID);

     response->set_hasspace(true);
     return Status::OK;
}

Status JPSserver::reqMATSim2ExternPutAgent(ServerContext* context,
          const MATSim2ExternPutAgent* request,
          MATSim2ExternPutAgentConfirmed* response)
{
     //Log->Write("INFO:\tRPC::JPSserver I have space on node " + nodeID);
     std::cout << "I am taking the agent" << std::endl;

     //take the nodeID and the destination ID
     //find the corresponding source
     //generate a new agent and add to the source
     //call processAllSource on the AgentsoursceManager
     return Status::OK;
}

Status JPSserver::reqExternDoSimStep(ServerContext* context,
          const ExternDoSimStep* request, ExternDoSimStepReceived* response)
{
     std::cout << "Performing simulation step" << std::endl;
     return Status::OK;
}

Status JPSserver::reqExternOnPrepareSim(ServerContext* context,
          const ExternOnPrepareSim* request,
          ExternOnPrepareSimConfirmed* response)
{
     std::cout << "I am preparing the simulation" << std::endl;
     return Status::OK;
}

Status JPSserver::reqExternAfterSim(ServerContext* context,
          const ExternAfterSim* request, ExternAfterSimConfirmed* response)
{
     std::cout << "Simulation step completed" << std::endl;
     return Status::OK;
}
