/**
 * \file        JPSclient.h
 * \date        Apr 21, 2015
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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

#ifndef MATSIM_JPSCLIENT_H_
#define MATSIM_JPSCLIENT_H_

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
using org::matsim::hybrid::Extern2MATSimTrajectories_Agent;

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
      * Process the finished agents queue and send them to the
      * external server
      */
     void ProcessAgentQueue(Building* building);

     /**
      * Shutdown the channels
      */
     void Shutdown()
     {
          _jupedsimChannel.reset();
          _matsimChannel.reset();
     }

     /**
      * Notify the external service that JuPedsim is up, running
      * and waiting for order.
      * @param host, the hostname where jupedsim can be reached
      * @param port, the port where jupedsim can be reached
      * @return true if everything went fine
      */
     bool NotifyExternalService(const std::string& host, int port);

     /**
      * Send a pedestrian to the external server
      * @return true in the case of success
      */
     bool SendAgentToMatsim(Pedestrian* ped);

     /**
      * Notify the external server about the end of the simulation.
      * This can also be the end of the simulation step as requested by the
      * @return true in case of success
      */
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
          //if()
          _mapMatsimID2JPSID[jpsID]=matsimID;
     }

     /**
      * Send trajectories after a simulation has been completed.
      * return true if everything went fine
      */
     bool SendTrajectories(Building* building);

     //void SetBuilding(Building)

private:
     // for testing the functionalities when there is no matsim server present
     bool HasSpaceOnJuPedSim(int nodeID);
     // for testing the functionalities when there is no matsim server present
     bool SendAgentToJuPedSim(Pedestrian* ped);

private:
     //call the remote methods on Jupedsim
     std::unique_ptr<ExternInterfaceService::Stub> _jupedsimChannel;
     //call the remote methods on matsim
     std::unique_ptr<MATSimInterfaceService::Stub> _matsimChannel;
     //map the matsim agent id to the jupedsim agent id
     std::map<int,std::string> _mapMatsimID2JPSID;
     //map the pedestrian id, to a time which is set
     // if the pedestrian could not be transfered to matsim
     std::map <int,int>_counter;
     //building object
     //Building* _building=nullptr;
};

#endif /* MATSIM_JPSCLIENT_H_ */
