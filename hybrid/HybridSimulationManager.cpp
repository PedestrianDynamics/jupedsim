/**
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
 **/
//
// Created by laemmel on 26.02.16.
//

#include "HybridSimulationManager.h"
#include "../Simulation.h"
#include "IniFileWriter.h"

#include <google/protobuf/stubs/common.h>

HybridSimulationManager::~HybridSimulationManager()
{

}

HybridSimulationManager::HybridSimulationManager(Configuration* config)
          :_config(config), _latches(new Latches())
{

}

std::string HybridSimulationManager::ToString()
{
     return "TODO\n";
}

void HybridSimulationManager::Start()
{
     GOOGLE_PROTOBUF_VERIFY_VERSION;
     grpc_init();

     ///0.0.0.0 means to listen on all devices
     string jupedsim_service_address("0.0.0.0:"+std::to_string(_config->GetServicePort()));

     JPSserver service(this, _latches, _config);
     _service = &service;
     ServerBuilder builder;
     builder.AddListeningPort(jupedsim_service_address, grpc::InsecureServerCredentials());
     builder.RegisterService(&service);
     _server = builder.BuildAndStart();

     Log->Write("INFO:\tJPS server at port: "+std::to_string(_config->GetServicePort())+" is up and running.");

     _server->Wait();
}

bool HybridSimulationManager::Run(Simulation& sim)
{

     Log->Write("WARNING:\tJuPedSim requires the maximum number of pedestrians to be known at startup. However,\n"
               "\t\tin a hybrid approach it is not possible to know this number beforehand.\n"
               "\t\tThis needs to be fixed in future. For the time being the number is arbitrarily set to 10,000.\n"
               "\t\tThe simulation is likely to crash if the actual number of pedestrians exceeds 10,000.");
     sim.RunHeader(10000);



     _service->SetSimulation(sim);

     _latches->SimulationPrepared();
     _latches->WaitForSimulationFinished();
     //simReady

     if (_config->GetDumpScenario()) {
          Log->Write("INFO:\tDumping scenario.");
          _config->SetTmax(Pedestrian::GetGlobalTime());
          sim.GetBuilding()->SaveGeometry(_config->GetProjectRootDir() / _config->GetGeometryFile());
          IniFileWriter* w = new IniFileWriter(_config, &sim, GetSimObserver());
          w->WriteToFile(_config->GetProjectRootDir() / "ini.xml");
     }

     grpc_shutdown();
     google::protobuf::ShutdownProtobufLibrary();
     Log->Write("INFO:\tJPS server shutdown.");

//     if (_config->GetDumpScenario()) {
//          Log->Write("INFO:\tDumping scenario.");
//          _config->SetTmax(Pedestrian::GetGlobalTime());
//          sim.GetBuilding()->SaveGeometry(_config->GetProjectRootDir()+_config->GetGeometryFile());
//          IniFileWriter* w = new IniFileWriter(_config, &sim, GetSimObserver());
//          w->WriteToFile(_config->GetProjectRootDir()+"ini.xml");
//     }
     return true;
}

void HybridSimulationManager::Shutdown()
{
     _server->Shutdown();

}

void HybridSimulationManager::WaitForScenarioLoaded()
{
     _latches->WaitForScenarioLoaded();
}

SimObserver* HybridSimulationManager::GetSimObserver()
{
     return &_simObserver;
}







