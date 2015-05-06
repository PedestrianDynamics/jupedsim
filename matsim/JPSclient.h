/*
 * JPSclient.h
 *
 *  Created on: Apr 21, 2015
 *      Author: piccolo
 */

#ifndef MATSIM_JPSCLIENT_H_
#define MATSIM_JPSCLIENT_H_

//#include <grpc++/status.h>
//client stuff
//#include <grpc++/channel_arguments.h>
//#include <grpc++/create_channel.h>
//#include <grpc++/credentials.h>

//using grpc::Server;
//using grpc::ServerBuilder;
//using grpc::ServerContext;
//using grpc::Status;
//using grpc::ChannelArguments;

//using grpc::ClientContext;
//using grpc::Status;

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/status.h>

#include "MATSimInterface.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ChannelInterface;
using grpc::ClientContext;
using org::matsim::hybrid::MATSim2ExternHasSpace;
using org::matsim::hybrid::MATSim2ExternHasSpaceConfirmed;

using namespace org::matsim::hybrid;

class Building;
class Pedestrian;

class JPSclient
{
public:

     /**
      * Constructor
      * @param channel, the rpc channel for the communication
      */
     JPSclient(std::shared_ptr<ChannelInterface> channel);

     /**
      * Destructor
      */
     virtual ~JPSclient();

     /**
      * Process the finished agents queue
      */
     void ProcessAgentQueue(Building* building);

     /**
      * Shutdown the channel
      */
     void Shutdown()
     {
          _jupedsimChannel.reset();
          _matsimChannel.reset();
     }

     bool NotifyExternalService(const std::string& host, int port);

     bool SendAgentToMatsim(Pedestrian* ped);

     bool NotifyEndOfSimulation();

     /**
      * Request the maximal number of agents
      * @return
      */
     int RequestMaxNumberAgents();

     /**
      * JuPedsim uses integer for the agent ID, MATSim uses string.
      * @param jpsID, the jupedsim internal ID
      * @param matsimID, the matsim string id
      */
     void MapMatsimAgentToJPSagent(int jpsID, std::string matsimID)
     {
          _mapMatsimID2JPSID[jpsID]=matsimID;
     }

private:
     bool HasSpaceOnMatsim(int nodeID);
     bool HasSpaceOnJuPedSim(int nodeID);
     bool SendAgentToJuPedSim(Pedestrian* ped);

private:
     std::unique_ptr<ExternInterfaceService::Stub> _jupedsimChannel;
     std::unique_ptr<MATSimInterfaceService::Stub> _matsimChannel;
     //map the matsim agent id to the jupedsim agent id
     std::map<int,std::string> _mapMatsimID2JPSID;
};

#endif /* MATSIM_JPSCLIENT_H_ */
