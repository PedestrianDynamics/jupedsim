/*
 * HybridSimulationManager.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: piccolo
 */

#include "HybridSimulationManager.h"
#include "MATSimInterface.pb.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/Building.h"
#include "../pedestrian/AgentsQueue.h"
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
     //char hostname[1024];
     //gethostname(hostname, 1024);
     //_internalServerName=std::string(hostname);

     //GOOGLE_PROTOBUF_VERIFY_VERSION;
     //grpc_init();

     //_rpcClient = std::unique_ptr<JPSclient>(new JPSclient( grpc::CreateChannel("localhost:9999",
     //          grpc::InsecureCredentials(), ChannelArguments())));


     //     std::string server_address(_serverName + ":" + std::to_string(_port));
     //     JPSserver service(_agentSrcMng);
     //
     //     ServerBuilder builder;
     //     builder.AddListeningPort(server_address,
     //               grpc::InsecureServerCredentials());
     //     builder.RegisterService(&service);
     //     _rpcServer= builder.BuildAndStart();
     //     Log->Write("INFO:\tJuPedSim Server listening on " + server_address);


}

HybridSimulationManager::~HybridSimulationManager()
{
}

bool HybridSimulationManager::Init(Building* building)
{
     _building = building;
     return true;
}

bool HybridSimulationManager::Run(Simulation& sim)
{
     //perform some initialisation stuff
     GOOGLE_PROTOBUF_VERIFY_VERSION;

     grpc_init();

     //string extern_service_address("zam597:9999");
     string extern_service_address(_externalServerName + ":" + std::to_string(_externalServerPort));

     string jupedsim_service_address(_internalServerName + ":" + std::to_string(_internalServerPort));
     //string jupedsim_service_address("0.0.0.0:9999")/*_serverName + ":" + std::to_string(_port)*/;


     //create the client that will be running on its own thread
     _rpcClient = std::shared_ptr<JPSclient>(new JPSclient( grpc::CreateChannel(extern_service_address,
               grpc::InsecureCredentials(), ChannelArguments())));

     //create the server
     std::string server_address(_externalServerName + ":" + std::to_string(_externalServerPort));

     JPSserver jupedsimService(sim);
     jupedsimService.SetDuplexClient(_rpcClient);
     //MATSIMserver jupedsimService;

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


     //starting the simulation thread and waiting
     std::thread t2(&JPSserver::RunSimulation, &jupedsimService);

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

//bool HybridSimulationManager::Run(Simulation& sim)
//{
//     //perform some initialisation stuff
//     GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//     grpc_init();
//
//     //create the client that will be running on its own thread
//     _rpcClient = std::unique_ptr<JPSclient>(new JPSclient( grpc::CreateChannel("localhost:9999",
//                         grpc::InsecureCredentials(), ChannelArguments())));
//
//     //create the server
//     std::string server_address(_serverName + ":" + std::to_string(_port));
//               JPSserver service(_agentSrcMng);
//
//               ServerBuilder builder;
//               builder.AddListeningPort(server_address,
//                         grpc::InsecureServerCredentials());
//               builder.RegisterService(&service);
//
//               _rpcServer= builder.BuildAndStart();
//               Log->Write("INFO:\tJuPedSim Server listening on " + server_address);
//
//               _rpcServer->Wait();
//     //create a socket and use it for the serveur and the client
//     //std::thread t1(&HybridSimulationManager::RunClient, this);
//     std::thread t2(&HybridSimulationManager::RunServer, this);
//     //t1.join();
//     t2.join();
//
//     //clean up everything
//     grpc_shutdown();
//     google::protobuf::ShutdownProtobufLibrary();
//     return true;
//}
void HybridSimulationManager::operator()()
{
     //Run();
}
bool HybridSimulationManager::RunClient()
{
     //check the message queue and send
     JPSclient client(
               grpc::CreateChannel("localhost:9999",
                         grpc::InsecureCredentials(), ChannelArguments()));

     do
     {

          //check if agents enter/left a link and fire event
          client.ProcessAgentQueue(_building);

          //wait some time, before a new attempt
          cout << "processing Requests for input:" << _shutdown << endl;
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
     if (_rpcServer) _rpcServer->Shutdown();
     //cout<<"shutting down: "<<_shutdown<<endl;
}

void HybridSimulationManager::ProcessIncomingAgent()
{

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

//void HybridSimulationManager::AttachSourceManager(const AgentsSourcesManager& src)
//{
//     //TODO create a deep copy of the manager
//     // and copy the sources without copying the generated pedestrian
//     _agentSrcMng=src;
//}
