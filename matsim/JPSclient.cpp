/**
 * \file        JPSclient.cpp
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
 * This class uses the channel created by the grpc to send information to the client.
 *
 **/

#include "JPSclient.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Building.h"
#include "../pedestrian/AgentsQueue.h"

#include "MATSimInterface.pb.h"

using namespace std;

JPSclient::JPSclient(std::shared_ptr<ChannelInterface> channel)
{
     //communication channel to matsim
     _matsimChannel = MATSimInterfaceService::NewStub(channel);

     //communication channel to JuPedsim
     // stub for testing and to be removed in the final version
     _jupedsimChannel = ExternInterfaceService::NewStub(channel);
}

JPSclient::~JPSclient()
{
}

void JPSclient::ProcessAgentQueue(Building* building)
{
     std::vector<Pedestrian*> peds;
     //cout<<"size:"<<AgentsQueueOut::Size()<<endl;
     AgentsQueueOut::GetandClear(peds);
     for (auto && ped:peds)
     {
          //if the agent has waited enough
          if (_counter[ped->GetID()]<=0)
          {
               //remove the pedestrian only if successfully sent
               if(SendAgentToMatsim(ped)==true)
               {
                    building->DeletePedestrian(ped);
               }
               else
               {
                    AgentsQueueOut::Add(ped);
                    //reschedule after 100 timesteps
                    _counter[ped->GetID()]=100;
               }
          }
          else
          {
               AgentsQueueOut::Add(ped);
               _counter[ped->GetID()]--;
          }
     }
}

bool JPSclient::SendAgentToMatsim(Pedestrian* ped)
{
     ClientContext context;
     Extern2MATSim request;
     Extern2MATSimConfirmed reply;

     string leave_node=std::to_string(ped->GetFinalDestination());
     //string agent_id=std::to_string(ped->GetID());
     string agent_id=_mapMatsimID2JPSID[ped->GetID()];

     request.mutable_agent()->set_id(agent_id);
     request.mutable_agent()->set_leavenode(leave_node);

     Status status =_matsimChannel->reqExtern2MATSim(&context, request, &reply);

     if(status.IsOk())
     {
          return reply.accepted();
     }
     return false;
}

bool JPSclient::HasSpaceOnJuPedSim(int nodeID)
{
     MATSim2ExternHasSpace request;
     request.set_nodeid(std::to_string(nodeID));
     MATSim2ExternHasSpaceConfirmed reply;
     ClientContext context;

     Status status = _jupedsimChannel->reqMATSim2ExternHasSpace(&context, request, &reply);

     if(status.IsOk())
     {
          return reply.hasspace();
     }
     else
     {
          Log->Write("ERROR:\t Rpc call failed");
     }

     return false;
}

bool JPSclient::NotifyExternalService(const std::string& host, int port)
{
     ClientContext context;
     ExternalConnect request;
     ExternalConnectConfirmed reply;
     request.set_host(host);
     request.set_port(port);
     Status status =_matsimChannel->reqExternalConnect(&context, request, &reply);
     return status.IsOk();
}

bool JPSclient::SendAgentToJuPedSim(Pedestrian* ped)
{
     ClientContext context;
     MATSim2ExternPutAgent request;
     MATSim2ExternPutAgentConfirmed reply;
     string leave_node_id=std::to_string(ped->GetFinalDestination());
     string enter_node_id="-1";
     if(leave_node_id=="3") enter_node_id="1";
     if(leave_node_id=="4") enter_node_id="2";


     request.mutable_agent()->set_id(std::to_string(ped->GetID()));
     request.mutable_agent()->set_enternode(enter_node_id);
     //only one final destination is supported
     request.mutable_agent()->add_nodes(leave_node_id);
     //request.mutable_agent()->add_nodes("0");

     Status status =_jupedsimChannel->reqMATSim2ExternPutAgent(&context, request, &reply);
     return status.IsOk();

}

bool JPSclient::NotifyEndOfSimulation()
{
     ClientContext context;
     ExternSimStepFinished request;
     ExternSimStepFinishedReceived reply;
     Status status =_matsimChannel->reqExternSimStepFinished(&context, request, &reply);
     //Log->Write("INFO:\tRPC::JPSserver simulation step finished");
     //std::cout<<"Details: "<<status.details()<<endl;
     return status.IsOk();
}

int JPSclient::RequestMaxNumberAgents()
{
     ClientContext context;
     MaximumNumberOfAgentsConfirmed request;
     MaximumNumberOfAgents reply;
     Status status =_matsimChannel->reqMaximumNumberOfAgents(&context, request, &reply);

     if(status.IsOk())
     {
          return reply.number();
     }
     Log->Write("ERROR:\t RPC JPSClient call failed <RequestMaxNumberAgents>");
     return -1;
}

bool JPSclient::SendTrajectories(Building* building)
{
     ClientContext context;
     Extern2MATSimTrajectories request;
     MATSim2ExternTrajectoriesReceived reply;

     //set the time
     request.set_time(Pedestrian::GetGlobalTime());

     auto&& allPeds = building->GetAllPedestrians();
     for(const auto& ped:allPeds)
     {
          auto&& agent = request.add_agent();
          double phi = atan2(ped->GetEllipse().GetSinPhi(), ped->GetEllipse().GetCosPhi())*180/M_PI;
          agent->set_angle(phi);
          agent->set_color(ped->GetColor());
          agent->set_id(ped->GetID());
          agent->set_x(ped->GetPos()._x);
          agent->set_y(ped->GetPos()._y);
          agent->set_z(ped->GetElevation());
     }

     Status status =_matsimChannel->reqSendTrajectories(&context, request, &reply);

     // if(!status.IsOk())
     // {
     //    Log->Write("ERROR:\t RPC JPSClient call failed <reqSendTrajectories>");
     // }
     return status.IsOk();
     //Log->Write("ERROR:\t RPC JPSClient call failed <reqSendTrajectories>");
}
