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

     /**
      * Start the hybrid simulation
      * @param sim, a reference to the simulation object
      * @return true if everything went fine
      */
     bool Run(Simulation& sim);
     bool RunClient();
     bool RunServer();

     /**
      * Shutdown the rpc server and client
      * this is not actually used.
      */
     void Shutdown();

     /**
      * @return a string description of the hybrid simulator
      */
     std::string ToString();

     /**
      * Make the class "runnable" by overloading the operator
      * @param value
      */
     //void operator()();

     /**
      * send outgoing agents to the external server
      */
     void ProcessOutgoingAgent();

private:
     /**
      * Testing the system
      */
     void TestWorkflow();

private:
     //std::atomic<bool> _shutdown=false;
     static bool _shutdown;
     int _externalServerPort=9999;
     int _internalServerPort=9998;
     std::string _externalServerName="localhost";
     std::string _internalServerName="localhost";
     Building* _building=nullptr;
     std::unique_ptr<grpc::Server> _rpcServer;
     AgentsSourcesManager _agentSrcMng;
     std::shared_ptr<JPSclient> _rpcClient;
};

#endif /* HYBRIDSIMULATIONMANAGER_H_ */
