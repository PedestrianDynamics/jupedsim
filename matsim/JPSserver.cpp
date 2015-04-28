/*
 * JPSserver.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#include "JPSserver.h"
#include "../IO/OutputHandler.h"
#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/AgentsSource.h"
#include "../pedestrian/AgentsQueue.h"
#include "../pedestrian/Pedestrian.h"


#include <iostream>

// external variables
extern OutputHandler* Log;


using namespace std;


JPSserver::JPSserver(AgentsSourcesManager& src): _agentSrcMng(src)
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
     string agent_id=request->agent().id();
     string leave_node=request->agent().nodes(0);//just the first node
     string enter_node=request->agent().enternode();
     Log->Write("INFO:\tRPC::JPSserver I am taking agent %s going to node %s ",agent_id.c_str(),leave_node.c_str());

     auto srcs=_agentSrcMng.GetSources();
     //cout<<"There are: "<<srcs.size()<<" options"<<endl;

     for(auto&& src:srcs)
     {
          //cout<<"size: "<<src->GetPoolSize()<<endl;
          if(src->GetId()==std::stoi(enter_node))
          {
               std::vector<Pedestrian*> peds;
               src->GenerateAgents(peds,1,_agentSrcMng.GetBuilding());
               //there should be only one agent in this vector
               for(auto&& ped:peds)
               {
                    ped->SetID(std::stoi(agent_id));
                    ped->SetFinalDestination(std::stoi(leave_node));
                    //schedule the agent
                    src->AddToPool(ped);
               }
               _agentSrcMng.ProcessAllSources();
               //AgentsQueue::Add(peds);
          }
     }

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
