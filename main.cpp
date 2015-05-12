/**
 * \file        main.cpp
 * \date        Jan 15, 2013
 * \version     v0.6
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
#include "matsim/HybridSimulationManager.h"

#include <thread>
#include <functional>
//#include <boost/version.hpp>

int main(int argc, char **argv)
{
     //gathering some statistics about the runtime
     time_t starttime, endtime;

     // Log = new FileHandler("./Logfile.dat");
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
          //Start the thread for managing incoming messages from MatSim
          auto hybrid=args->GetHybridSimManager();
          //process the hybrid simulation
          if(hybrid)
          {
               evacTime=hybrid->Run(sim);
          }
          //process the normal simulation
          else
          {
               //Start the threads for managing the sources of agents if any
               std::thread t1(sim.GetAgentSrcManager());

               //std::thread t1(&AgentsSourcesManager::Run, &sim.GetAgentSrcManager());

               //main thread for the simulation
               Log->Write("INFO: \tStart runSimulation()");
               //evacTime = sim.RunSimulation(args->GetTmax());
               evacTime = sim.RunStandardSimulation(args->GetTmax());
               Log->Write("\nINFO: \tEnd runSimulation()");
               time(&endtime);

               //the execution is finished at this time
               //so join the main thread
               t1.join();

          }

          // some statistics output
          if(args->ShowStatistics())
          {
               sim.PrintStatistics();
          }

          if (sim.GetPedsNumber())
          {
               Log->Write("WARNING: \nPedestrians not evacuated [%d] using [%d] threads",
                         sim.GetPedsNumber(), args->GetMaxOpenMPThreads());
          }

          double execTime = difftime(endtime, starttime);
          Log->Write("\nExec Time [s]   : %.2f", execTime);
          Log->Write("Evac Time [s]     : %d", evacTime);
          Log->Write("Realtime Factor   : %.2f X", evacTime / execTime);
          Log->Write("Number of Threads : %d", args->GetMaxOpenMPThreads());
          Log->Write("Warnings          : %d", Log->GetWarnings());
          Log->Write("Errors            : %d", Log->GetErrors());

          if (nullptr == dynamic_cast<STDIOHandler*>(Log))
          {
               printf("\nExec Time [s]     : %4.2f\n", execTime);
               printf("Evac Time [s]       : %d\n", evacTime);
               printf("Realtime Factor     : %.2f (X)\n", evacTime / execTime);
               printf("Number of Threads   : %d\n", args->GetMaxOpenMPThreads());
               printf("Warnings            : %d\n", Log->GetWarnings());
               printf("Errors              : %d\n", Log->GetErrors());
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
