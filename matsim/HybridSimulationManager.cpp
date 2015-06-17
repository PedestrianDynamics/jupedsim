/**
 * \file        HybridSimulationManager.cpp
 * \date        Apr 20, 2015
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
 * This class manages the hybrid simulation by connecting to a server, receiving information and launching a simulation.
 * It makes use of the google protocol buffer (https://github.com/google/protobuf)
 * as serializing technique and grpc (https://github.com/grpc/) for the communication across the network.
 *
 **/

#include "HybridSimulationManager.h"
#include "MATSimInterface.pb.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Building.h"
#include "../pedestrian/AgentsSourcesManager.h"
#include "../Simulation.h"
#include "JPSserver.h"
#include "JPSclient.h"

// std stuffs
#include <thread>
#include <functional>

//google stuff
//server stuff
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>
//client stuff
#include <grpc++/channel_arguments.h>
#include <grpc++/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/credentials.h>
#include "../pedestrian/AgentsQueue.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using namespace std;

bool HybridSimulationManager::_shutdown = false;

HybridSimulationManager::HybridSimulationManager(const std::string& server,
          int port)
{
     _externalServerName = server;
     _externalServerPort = port;

     //get the canonical hostname
     char hostname[1024];
     gethostname(hostname, 1024);
     _internalServerName=std::string(hostname);
}

HybridSimulationManager::~HybridSimulationManager()
{
}

bool HybridSimulationManager::Run(Simulation& sim)
{
     //perform some initialisation stuff
     GOOGLE_PROTOBUF_VERIFY_VERSION;
     grpc_init();

     //copy the building object
     _building=sim.GetBuilding();

     //string extern_service_address("zam597:9999");
     string extern_service_address(_externalServerName + ":" + std::to_string(_externalServerPort));

     ///0.0.0.0 means to listen on all devices
     string jupedsim_service_address("0.0.0.0:" + std::to_string(_internalServerPort));

     //create the client that will be running on its own thread
     _rpcClient = std::shared_ptr<JPSclient>(new JPSclient( grpc::CreateChannel(extern_service_address,
               grpc::InsecureCredentials(), ChannelArguments())));

     //create the server
     std::string server_address(_externalServerName + ":" + std::to_string(_externalServerPort));

     JPSserver jupedsimService(sim);
     jupedsimService.SetDuplexClient(_rpcClient);


     ServerBuilder builder;
     builder.AddListeningPort(jupedsim_service_address,
               grpc::InsecureServerCredentials());
     builder.RegisterService(&jupedsimService);

     _rpcServer= builder.BuildAndStart();

     Log->Write("INFO:\tJuPedSim is up and running on " + jupedsim_service_address);
     Log->Write("INFO:\tNotifying Matsim at " + extern_service_address);

     if(false==_rpcClient->NotifyExternalService(_internalServerName,_internalServerPort))
     {
          Log->Write("ERROR:\tNotification failed");
     }

     // Starting the simulation thread and waiting
     // The simulation runs in a loop until the shutdown request
     // is received from the external serveur
     std::thread t2(&JPSserver::RunSimulation, &jupedsimService);

     // This is not needed since t2 will not exit until shutdown is received.
     //_rpcServer->Wait();

     //TestWorkflow();
     t2.join();


     //create a socket and use it for the serveur and the client
     //std::thread t1(&HybridSimulationManager::RunClient, this);
     //std::thread t2(&HybridSimulationManager::RunServer, this);
     //t1.join();
     //t2.join();

     //clean up everything
     grpc_shutdown();
     google::protobuf::ShutdownProtobufLibrary();
     return true;
}

bool HybridSimulationManager::RunClient()
{
     //check the message queue and send
     string extern_service_address(
               _externalServerName + ":" + std::to_string(_externalServerPort));
     JPSclient client(
               grpc::CreateChannel(extern_service_address,
                         grpc::InsecureCredentials(), ChannelArguments()));
     do
     {
          //check if agents enter/left a link and fire event
          client.ProcessAgentQueue(_building);
          //wait some time, before a new attempt
          Log->Write("INFO:\t processing Requests for input: %d", _shutdown);
          //ProcessOutgoingAgent();
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
     } while (!_shutdown);

     return true;
}

bool HybridSimulationManager::RunServer()
{
     //
     //     do
     //     {
     //          std::string server_address(_serverName + ":" + std::to_string(_port));
     //          JPSserver service(_agentSrcMng);
     //
     //          ServerBuilder builder;
     //          builder.AddListeningPort(server_address,
     //                    grpc::InsecureServerCredentials());
     //          builder.RegisterService(&service);
     //
     //          _rpcServer= builder.BuildAndStart();
     //          Log->Write("INFO:\tJuPedSim Server listening on " + server_address);
     //
     //          _rpcServer->Wait();
     //
     //          //wait for incoming connection
     //          //receive and parse message
     //          //process message
     //          //can insert pedestrian ?
     //          //accept pedestrian and feed to the queue
     //
     //          //wait some time, before a new attempt
     //          cout << "waiting for output:" << _shutdown << endl;
     //          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
     //     } while (!_shutdown);
     //
     return true;
}

void HybridSimulationManager::Shutdown()
{
     _shutdown = true;
     if (_rpcServer)
     {
          _rpcServer->Shutdown();
     }
}

void HybridSimulationManager::TestWorkflow()
{
     // notify the external service that I am awake
     //_rpcClient->NotifyExternalService();
}


//call after each simulation step
void HybridSimulationManager::ProcessOutgoingAgent()
{
     _rpcClient->ProcessAgentQueue(_building);
}

std::string HybridSimulationManager::ToString()
{
     return "INFO:\tHybrid Simulation working on [" + _externalServerName + ":"
               + std::to_string(_externalServerPort) + "]\n";
}
