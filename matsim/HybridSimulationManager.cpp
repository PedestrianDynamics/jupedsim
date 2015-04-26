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
	_serverName = server;
	_port = port;
}

HybridSimulationManager::~HybridSimulationManager()
{
}

bool HybridSimulationManager::Init(Building* building)
{
	//     GOOGLE_PROTOBUF_VERIFY_VERSION;

	_building = building;
	return true;
}
void HybridSimulationManager::AttachSourceManager(const AgentsSourcesManager& src)
{
//create a source manager and copy

}

bool HybridSimulationManager::Run()
{
	//perform some initialisation stuff
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	grpc_init();

	//create a socket and use it for the serveur and the client
	std::thread t1(&HybridSimulationManager::RunClient, this);
	std::thread t2(&HybridSimulationManager::RunServer, this);
	t1.join();
	t2.join();

	//clean up everything
	grpc_shutdown();
	google::protobuf::ShutdownProtobufLibrary();
	return true;
}
void HybridSimulationManager::operator()()
{
	Run();
}
bool HybridSimulationManager::RunClient()
{

			//check the message queue and send
		JPSclient client(
				grpc::CreateChannel("localhost:9999",
						grpc::InsecureCredentials(), ChannelArguments()));

		//client.SetSourceManager();

	do
	{

		//check if agents enter/left a link and fire event
		client.ProcessAgentQueue(_building);

		//wait some time, before a new attempt
		cout << "waiting for input:" << _shutdown << endl;
		//ProcessOutgoingAgent();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} while (!_shutdown);

	return true;
}

bool HybridSimulationManager::RunServer()
{
	//check the message queue and send
	do
	{
		std::string server_address(_serverName + ":" + std::to_string(_port));
		JPSserver service;

		ServerBuilder builder;
		builder.AddListeningPort(server_address,
				grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		_rpcServer= builder.BuildAndStart();
		Log->Write("INFO:\tJuPedSim Server listening on " + server_address);
		//_rpcServer->Wait();

		//wait for incoming connection
		//receive and parse message
		//process message
		//can insert pedestrian ?
		//accept pedestrian and feed to the queue

		//wait some time, before a new attempt
		cout << "waiting for output:" << _shutdown << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} while (!_shutdown);

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

void HybridSimulationManager::ProcessOutgoingAgent()
{
	std::vector<Pedestrian*> peds;
	AgentsQueueOut::GetandClear(peds);

	for (auto && ped:peds)
	{
		//pick the agent from the queue
		hybrid::Extern2MATSim msg;
		msg.mutable_agent()->set_id(std::to_string(ped->GetID()));
		msg.mutable_agent()->set_leavenode(std::to_string(ped->GetExitIndex()));

		//write to the disk or send to hell
		//std::fstream output("test.buf", ios::out | ios::trunc | ios::binary);
		//if (!msg.SerializeToOstream(&output))
		//{
		//    cerr << "Failed to write address book." << endl;
		//}
		cout<<"deleting:"<<endl;
		_building->DeletePedestrian(ped);
	}
}

std::string HybridSimulationManager::ToString()
{
	return "INFO:\tHybrid Simulation working on [" + _serverName + ":"
			+ std::to_string(_port) + "]\n";
}
