/*
 * JPSclient.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#include "JPSclient.h"
#include "../pedestrian/Pedestrian.h"
#include "../pedestrian/AgentsQueue.h"
#include "../geometry/Building.h"

#include "MATSimInterface.pb.h"

using namespace std;

JPSclient::JPSclient(std::shared_ptr<ChannelInterface> channel)
          //:_jupedsimChannel(ExternInterfaceService::NewStub(channel))
{
     //communication channel to matsim
     _matsimChannel = MATSimInterfaceService::NewStub(channel);

     //communication channel to JuPedsim
     // stub for testing and to be removed in the final version
     _jupedsimChannel= ExternInterfaceService::NewStub(channel);
}

JPSclient::~JPSclient()
{
}

void JPSclient::ProcessAgentQueue(Building* building)
{
     std::vector<Pedestrian*> peds;
     AgentsQueueOut::GetandClear(peds);

     for (auto && ped:peds)
     {

          /* for testing only, the agent is send back to jupedsim
          if(HasSpaceOnJuPedSim(ped->GetFinalDestination())==true)
          {
               if(SendAgentToJuPedSim(ped)==false)
               {
                    Log->Write("ERROR:\t RPC:JPSclient request failed (send agent to jupedsim)");
               }
          }
          else
          {
               Log->Write("ERROR:\t RPC:JPSclient request failed (space on jupedsim)");
          }
           */

          //remove the pedestrian only if successfully sent
          if(SendAgentToMatsim(ped)==true)
          {
               building->DeletePedestrian(ped);
          }
          else
          {
               AgentsQueueOut::Add(ped);
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
     //std::cout<<"Details: "<<status.details()<<endl;
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
