/**
 * \file        main.cpp
 * \date        Jan 15, 2013
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * main function
 *
 *
 **/

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "./Simulation.h"
#include "pedestrian/AgentsSourcesManager.h"

#ifdef _USE_PROTOCOL_BUFFER
#include "matsim/HybridSimulationManager.h"
#endif

#include <thread>
#include <functional>
#include <iomanip>

int main(int argc, char **argv)
{
     //gathering some statistics about the runtime
     time_t starttime, endtime;

     // default logger
     Log = new STDIOHandler();

     // Parsing the arguments
     ArgumentParser* args = new ArgumentParser();
     bool status=args->ParseArgs(argc, argv);

     // create and initialize the simulation engine
     // Simulation
     time(&starttime);

     Simulation sim(*args);

     if(status&&sim.InitArgs(*args))
     {
          //evacuation time
          int evacTime = 0;
          Log->Write("INFO: \tStart runSimulation()");

#ifdef _USE_PROTOCOL_BUFFER
          //Start the thread for managing incoming messages from MatSim
          auto hybrid=args->GetHybridSimManager();
          //process the hybrid simulation
          if(hybrid)
          {
               evacTime=hybrid->Run(sim);
          }
          //process the normal simulation
          else
#endif
          if(sim.GetAgentSrcManager().GetMaxAgentNumber())
          {
               //Start the thread for managing the sources of agents if any
               //std::thread t1(sim.GetAgentSrcManager());
               std::thread t1(&AgentsSourcesManager::Run, &sim.GetAgentSrcManager());
               //main thread for the simulation
               evacTime = sim.RunStandardSimulation(args->GetTmax());
               //Join the main thread
               t1.join();
          }
          else
          {
               //main thread for the simulation
               evacTime = sim.RunStandardSimulation(args->GetTmax());
          }

          Log->Write("\nINFO: \tEnd runSimulation()");
          time(&endtime);

          // some statistics output
          if(args->ShowStatistics())
          {
               sim.PrintStatistics();
          }

          if (sim.GetPedsNumber())
          {
               Log->Write("WARNING: Pedestrians not evacuated [%d] using [%d] threads",
                         sim.GetPedsNumber(), args->GetMaxOpenMPThreads());
          }

          double execTime = difftime(endtime, starttime);

          std::stringstream summary;
          summary << std::setprecision(2)<<std::fixed;
          summary<<"\nExec Time [s]     : "<< execTime<<std::endl;
          summary<<"Evac Time [s]     : "<< evacTime<<std::endl;
          summary<<"Realtime Factor   : "<< evacTime / execTime<<" X " <<std::endl;
          summary<<"Number of Threads : "<< args->GetMaxOpenMPThreads()<<std::endl;
          summary<<"Warnings          : "<< Log->GetWarnings()<<std::endl;
          summary<<"Errors            : "<< Log->GetErrors()<<std::endl;
          Log->Write(summary.str().c_str());

          //force an output to the screen if the log is not the standard output
          if (nullptr == dynamic_cast<STDIOHandler*>(Log))
          {
               printf("%s\n", summary.str().c_str());
          }
     }
     else
     {
          Log->Write("INFO:\tFinishing...");
     }
     // do the last cleaning
     delete args;
     delete Log;

     return (EXIT_SUCCESS);
}
