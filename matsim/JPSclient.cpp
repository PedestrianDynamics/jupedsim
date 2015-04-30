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

JPSclient::JPSclient(std::shared_ptr<ChannelInterface> channel) :
               _jupedsimChannel(ExternInterfaceService::NewStub(channel))
     {
     //stub for testing and to be removed in the final version
     _matsimChannel=MATSimInterfaceService::NewStub(channel);
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


          if(HasSpaceOnMatsim(ped->GetFinalDestination())==true)
          {
               SendAgentToMatsim(ped);
          }
          //std::cout<<"deleting from the client:"<<std::endl;
          building->DeletePedestrian(ped);
     }
}

bool JPSclient::HasSpaceOnMatsim(int nodeID)
{
     //Status status =_matsimChannel->reqExtern2MATSim(&context, request, &reply);
     //if(status.IsOk())
     //{
//
  //   }
     return true;
}

bool JPSclient::SendAgentToMatsim(Pedestrian* ped)
{
     ClientContext context;
     Extern2MATSim request;
     Extern2MATSimConfirmed reply;

     string leave_node=std::to_string(ped->GetFinalDestination());
     string agent_id=std::to_string(ped->GetID());

     request.mutable_agent()->set_id(agent_id);
     request.mutable_agent()->set_leavenode(leave_node);

     Status status =_matsimChannel->reqExtern2MATSim(&context, request, &reply);

     return status.IsOk();
}

bool JPSclient::HasSpaceOnJuPedSim(int nodeID)
{
     MATSim2ExternHasSpace request;
     request.set_nodeid(std::to_string(nodeID));
     MATSim2ExternHasSpaceConfirmed reply;
     ClientContext context;

     Status status = _jupedsimChannel->reqMATSim2ExternHasSpace(&context, request, &reply);
     return status.IsOk();
}

bool JPSclient::NotifyExternalService()
{
     ClientContext context;
     ExternalConnect request;
     ExternalConnectConfirmed reply;
     request.set_accepted(true);
     //reply.
     Status status =_matsimChannel->reqExternalConnect(&context, request, &reply);
     std::cout<<"Details: "<<status.details()<<endl;
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
