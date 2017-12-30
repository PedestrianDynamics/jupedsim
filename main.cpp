/**
 * \file        main.cpp
 * \date        Jan 15, 2013
 * \version     v0.8.1
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

#ifdef _JPS_AS_A_SERVICE

#include "hybrid/HybridSimulationManager.h"

#endif

#include <thread>

int main(int argc, char** argv)
{
    //gathering some statistics about the runtime
    time_t starttime, endtime;

    // default logger
    Log = new STDIOHandler();

    Configuration* configuration = new Configuration();
    // Parsing the arguments
    bool status = false;
    {    
          //ArgumentParser* p = new ArgumentParser(configuration); //Memory Leak
          std::unique_ptr<ArgumentParser> p(new ArgumentParser(configuration));
          status = p->ParseArgs(argc, argv);
    }
#ifdef _JPS_AS_A_SERVICE
    if (configuration->GetRunAsService()) {
          std::shared_ptr<HybridSimulationManager> hybridSimulationManager = std::shared_ptr<HybridSimulationManager>(
                    new HybridSimulationManager(configuration));
          configuration->SetHybridSimulationManager(hybridSimulationManager);
          std::thread t = std::thread(&HybridSimulationManager::Start, hybridSimulationManager);
          hybridSimulationManager->WaitForScenarioLoaded();
          t.detach();
     }
#endif
    // create and initialize the simulation engine
    // Simulation
    time(&starttime);

    Simulation sim(configuration);

    if (status && sim.InitArgs()) {
        //evacuation time
        double evacTime = 0;
        Log->Write("INFO: \tStart runSimulation() with %d pedestrians", sim.GetPedsNumber());

#ifdef _JPS_AS_A_SERVICE

        if (configuration->GetRunAsService()) {
              configuration->GetHybridSimulationManager()->Run(sim);
         }
         else
#endif
        if (sim.GetAgentSrcManager().GetMaxAgentNumber()) {
            //Start the thread for managing the sources of agents if any
            //std::thread t1(sim.GetAgentSrcManager());
            double simMaxTime = configuration->GetTmax();
            std::thread t1(&AgentsSourcesManager::Run, &sim.GetAgentSrcManager());//@todo pass simMaxTime to Run
            //main thread for the simulation
            evacTime = sim.RunStandardSimulation(simMaxTime);
            //Join the main thread
            t1.join();
        }
        else {
            //main thread for the simulation
            evacTime = sim.RunStandardSimulation(configuration->GetTmax());
        }

        Log->Write("\nINFO: \tEnd runSimulation()");
        time(&endtime);

        // some statistics output
        if (configuration->ShowStatistics()) {
            sim.PrintStatistics();
        }

        if (sim.GetPedsNumber()) {
            Log->Write("WARNING: Pedestrians not evacuated [%d] using [%d] threads",
                    sim.GetPedsNumber(), configuration->GetMaxOpenMPThreads());
        }

        double execTime = difftime(endtime, starttime);

        std::stringstream summary;
        summary << std::setprecision(2) << std::fixed;
        summary << "\nExec Time [s]     : " << execTime << std::endl;
        summary << "Evac Time [s]     : " << evacTime << std::endl;
        summary << "Realtime Factor   : " << evacTime/execTime << " X " << std::endl;
        summary << "Number of Threads : " << configuration->GetMaxOpenMPThreads() << std::endl;
        summary << "Warnings          : " << Log->GetWarnings() << std::endl;
        summary << "Errors            : " << Log->GetErrors() << std::endl;
        Log->Write(summary.str().c_str());

        //force an output to the screen if the log is not the standard output
        if (nullptr==dynamic_cast<STDIOHandler*>(Log)) {
            printf("%s\n", summary.str().c_str());
        }
    }
    else {
        Log->Write("INFO:\tFinishing...");
    }
    // do the last cleaning
    delete configuration;
    delete Log;

    return (EXIT_SUCCESS);
}
