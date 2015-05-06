/*
 * JPSserver.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#include "JPSserver.h"
#include "JPSclient.h"
#include "../IO/OutputHandler.h"
#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/AgentsSource.h"
#include "../pedestrian/AgentsQueue.h"
#include "../pedestrian/Pedestrian.h"
#include "../Simulation.h"

#include <iostream>
#include <thread>


//client stuff
#include <grpc++/channel_arguments.h>
//#include <grpc++/channel_interface.h>
//#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
//#include <grpc++/credentials.h>

// external variables
extern OutputHandler* Log;

using namespace std;

JPSserver::JPSserver(Simulation& src, const std::string& connection): _SimManager(src)
{
     //_rpcClient = std::unique_ptr<JPSclient>(new JPSclient( grpc::CreateChannel(connection,
     //              grpc::InsecureCredentials(), grpc::ChannelArguments())));

     _doSimulation=false;
}

JPSserver::~JPSserver()
{
}

void JPSserver::RunSimulation()
{
     //get the maximum number of agents
     _SimManager.RunHeader(_jpsClient->RequestMaxNumberAgents());
     do
     {
          if(_doSimulation)
          {
               Log->Write("INFO:\tRPC::JPSserver starting a new simulation");
               _SimManager.RunBody(_maxSimTime);
               _doSimulation=false;
               _jpsClient->NotifyEndOfSimulation();
               //exit(0);
          }

          //Log->Write("INFO:\tRPC::JPSserver idle for 3 seconds");
          //std::this_thread::sleep_for(std::chrono::milliseconds(500));
     }while(!_shutdown);

     _SimManager.RunFooter();
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

     auto agentSrcMng=_SimManager.GetAgentSrcManager();
     auto srcs=agentSrcMng.GetSources();
     //cout<<"There are: "<<srcs.size()<<" options"<<endl;

     for(auto&& src:srcs)
     {
          //cout<<"size: "<<src->GetPoolSize()<<endl;
          if(src->GetId()==std::stoi(enter_node))
          {
               std::vector<Pedestrian*> peds;
               src->GenerateAgents(peds,1,agentSrcMng.GetBuilding());
               //there should be only one agent in this vector
               for(auto&& ped:peds)
               {
                    //ped->SetID(std::stoi(agent_id));
                    //_mapMatsimID2JPSID[agent_id]=ped->GetID();
                    //TODO: there might be a race condition here if the client is sending agents out
                    _jpsClient->MapMatsimAgentToJPSagent(ped->GetID(),agent_id);
                    ped->SetFinalDestination(std::stoi(leave_node));
                    //schedule the agent
                    src->AddToPool(ped);
               }
               agentSrcMng.ProcessAllSources();
               //AgentsQueue::Add(peds);
          }
     }

     return Status::OK;
}

Status JPSserver::reqExternDoSimStep(ServerContext* context __attribute__((unused)),
          const ExternDoSimStep* request, ExternDoSimStepReceived* response __attribute__((unused)))
{
     double from =request->fromtime();
     double to=request->totime();
     Log->Write("INFO:\tRPC::JPSserver I will perform a simulation step from %f to %f seconds",from,to);
     _doSimulation=true;
     _maxSimTime=to;
     return Status::OK;
}

Status JPSserver::reqExternOnPrepareSim(ServerContext* context __attribute__((unused)),
          const ExternOnPrepareSim* request __attribute__((unused)),
          ExternOnPrepareSimConfirmed* response __attribute__((unused)))
{
     Log->Write("INFO:\tRPC::JPSserver I am ready for doing the simulation");

     return Status::OK;
}

Status JPSserver::reqExternAfterSim(ServerContext* context,
          const ExternAfterSim* request, ExternAfterSimConfirmed* response)
{
     Log->Write("INFO:\tRPC::JPSserver I received shutdown order. But can I do that ?");
     _shutdown=true;
     return Status::OK;
}

void JPSserver::SetDuplexClient(std::shared_ptr<JPSclient>& client)
{
     _jpsClient=client;
}
