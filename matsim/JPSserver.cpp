/**
 * \file        JPSserver.cpp
 * \date        Apr 21, 2015
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
 * This class implements the remote procedure calls methods defined in the matsim.proto interface.
 * These methods are then called by the remote program via the channel created.
 *
 **/

#include "JPSserver.h"
#include "JPSclient.h"
#include "../IO/OutputHandler.h"
#include "../pedestrian/AgentsSourcesManager.h"
#include "../pedestrian/AgentsSource.h"
#include "../pedestrian/Pedestrian.h"
#include "../Simulation.h"

#include <iostream>
#include <thread>

//client stuff
#include <grpc++/channel_arguments.h>
#include <grpc++/create_channel.h>

#include "../pedestrian/AgentsQueue.h"

// external variables
extern OutputHandler* Log;

using namespace std;

JPSserver::JPSserver(Simulation& src): _SimManager(src)
{
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
               _SimManager.RunBody(_maxSimTime);
               //important: _doSimulation should be set to false before doing any other things
               _doSimulation=false;
               _jpsClient->SendTrajectories(_SimManager.GetBuilding());
               _jpsClient->NotifyEndOfSimulation();
          }


     }while(!_shutdown);

     _SimManager.RunFooter();
}

Status JPSserver::reqMATSim2ExternHasSpace(ServerContext* context __attribute__((unused)),
          const MATSim2ExternHasSpace* request,
          MATSim2ExternHasSpaceConfirmed* response)
{
     string nodeID=request->nodeid();
     //Log->Write("INFO:\tRPC::JPSserver I have space on node " + nodeID);
     //TODO: call the voronoi implementation and check if there is a spot
     response->set_hasspace(true);
     return Status::OK;
}

Status JPSserver::reqMATSim2ExternPutAgent(ServerContext* context  __attribute__((unused)),
          const MATSim2ExternPutAgent* request,
          MATSim2ExternPutAgentConfirmed* response __attribute__((unused)))
{
     //Log->Write("INFO:\tRPC::JPSserver I have space on node " + nodeID);
     string agent_id=request->agent().id();
     string leave_node=request->agent().nodes(0);//just the first node
     string enter_node=request->agent().enternode();
     //Log->Write("INFO:\tRPC::JPSserver I am taking agent %s going to node %s ",agent_id.c_str(),leave_node.c_str());

     auto& agentSrcMng=_SimManager.GetAgentSrcManager();
     auto srcs=agentSrcMng.GetSources();

     for(auto&& src:srcs)
     {
          if(src->GetId()==std::stoi(enter_node))
          {
               std::vector<Pedestrian*> peds;
               src->GenerateAgents(peds,1,agentSrcMng.GetBuilding());
               //there should be only one agent in this vector
               for(auto&& ped:peds)
               {
                    //TODO: there might be a race condition here if the client is sending agents out at the same time
                    //TODO: map the agents back, not necessary if jupedsim is reset after each iteration
                    // because each incoming agent has an id and an agent will get two
                    _jpsClient->MapMatsimAgentToJPSagent(ped->GetID(),agent_id);
                    ped->SetFinalDestination(std::stoi(leave_node));
                    //schedule the agent
                    src->AddToPool(ped);
               }
               agentSrcMng.ProcessAllSources();
          }
     }

     return Status::OK;
}

Status JPSserver::reqExternDoSimStep(ServerContext* context __attribute__((unused)),
          const ExternDoSimStep* request, ExternDoSimStepReceived* response __attribute__((unused)))
{
     double from =request->fromtime();
     double to=request->totime();
     Log->Write("INFO:\tRPC::JPSserver Simulation step from %.2f to %.2f seconds",from,to);
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

Status JPSserver::reqExternAfterSim(
          ServerContext* context __attribute__((unused)),
          const ExternAfterSim* request __attribute__((unused)),
          ExternAfterSimConfirmed* response __attribute__((unused)))
{
     Log->Write("INFO:\tRPC::JPSserver I received shutdown order. Good bye");
     _shutdown=true;
     return Status::OK;
}

void JPSserver::SetDuplexClient(std::shared_ptr<JPSclient>& client)
{
     _jpsClient=client;
}
