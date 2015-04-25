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


JPSclient::JPSclient(std::shared_ptr<ChannelInterface> channel) :
               _stub(ExternInterfaceService::NewStub(channel))
     {
     //stub for testing and to be removed in the final version
     _stub_matsim=MATSimInterfaceService::NewStub(channel);
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

          if(HasSpaceOnJuPedSim(ped->GetExitIndex())==true)
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


          if(HasSpaceOnMatsim(ped->GetExitIndex())==true)
          {
               SendAgentToMatsim(ped);
          }
          std::cout<<"deleting from the client:"<<std::endl;
          building->DeletePedestrian(ped);
     }
}

bool JPSclient::HasSpaceOnMatsim(int nodeID)
{
}

bool JPSclient::SendAgentToMatsim(Pedestrian* ped)
{
     //pick the agent from the queue
     //hybrid::Extern2MATSim msg;
     //msg.mutable_agent()->set_id(std::to_string(ped->GetID()));
     //msg.mutable_agent()->set_leavenode(std::to_string(ped->GetExitIndex()));

}

bool JPSclient::HasSpaceOnJuPedSim(int nodeID)
{
     MATSim2ExternHasSpace request;
     request.set_nodeid(std::to_string(nodeID));
     MATSim2ExternHasSpaceConfirmed reply;
     ClientContext context;

     Status status = _stub->reqMATSim2ExternHasSpace(&context, request, &reply);
     return status.IsOk();

//     if (status.IsOk()) {
//          return true;
//     } else {
//          return "Rpc failed";
//     }
}

bool JPSclient::SendAgentToJuPedSim(Pedestrian* ped)
{
     ClientContext context;
     MATSim2ExternPutAgent request;
     MATSim2ExternPutAgentConfirmed reply;

     request.mutable_agent()->set_id(std::to_string(ped->GetID()));
     request.mutable_agent()->set_enternode(std::to_string(ped->GetExitIndex()));
     //only one final destination is supported
     request.mutable_agent()->add_nodes("-1");
     request.mutable_agent()->add_nodes("0");

     Status status =_stub->reqMATSim2ExternPutAgent(&context, request, &reply);
     return status.IsOk();

}
