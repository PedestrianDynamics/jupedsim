/*
 * HybridSimulationManager.h
 *
 *  Created on: Apr 20, 2015
 *      Author: piccolo
 */

#ifndef HYBRIDSIMULATIONMANAGER_H_
#define HYBRIDSIMULATIONMANAGER_H_

#include <string>
#include <atomic>
#include <memory>
#include <grpc++/server.h>

#include "../pedestrian/AgentsSourcesManager.h"

//forward classes
class Building;
class AgentsSourcesManager;
class JPSclient;
class Simulation;

class HybridSimulationManager
{
public:
     /**
      * Constructor
      * @param server, the server name to listen
      * @param port, the server port to listen
      */
     HybridSimulationManager(const std::string& server,int port);

     /**
      * Destructor
      */
     virtual ~HybridSimulationManager();

     bool Init(Building* building);
     bool Run(Simulation& sim);
     bool RunClient();
     bool RunServer();
     void Shutdown();
     std::string ToString();

     /**
      * Make the class "runnable" by overloading the operator
      * @param value
      */
     void operator()();

     /**
      * Create another source manager
      */
     void AttachSourceManager(const AgentsSourcesManager& src);

     void ProcessOutgoingAgent();

private:
     void ProcessIncomingAgent();

     void TestWorkflow();

private:
     //std::atomic<bool> _shutdown=false;
     static bool _shutdown;
     int _externalServerPort=9999;
     int _internalServerPort=9998;
     std::string _externalServerName="localhost";
     std::string _internalServerName="zam763";
     Building* _building=nullptr;
     //TODO: the method should be passed bz reference in the main function
     //std::unique_ptr<grpc::Server> _rpcServer;
     std::unique_ptr<grpc::Server> _rpcServer;
     AgentsSourcesManager _agentSrcMng;
     std::shared_ptr<JPSclient> _rpcClient;
};

#endif /* HYBRIDSIMULATIONMANAGER_H_ */
