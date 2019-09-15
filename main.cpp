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
#include "Simulation.h"
#include "IO/OutputHandler.h"
#include "events/EventManager.h"
#include "general/Configuration.h"
#include "general/ArgumentParser.h"
#include "general/Compiler.h"
#include "general/Logger.h"
#include "pedestrian/AgentsSourcesManager.h"

#include <fmt/format.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iomanip>
#include <memory>
#include <ostream>
#include <string>
#include <thread>


#ifdef _JPS_AS_A_SERVICE
#include "hybrid/HybridSimulationManager.h"
#endif

int main(int argc, char** argv)
{
    Logging::Guard guard;
    Logging::Info("Starting JuPedSim - JPScore");
    Logging::Info(fmt::format("Version {}", JPSCORE_VERSION));
    Logging::Info(fmt::format("Commit id {}", GIT_COMMIT_HASH));
    Logging::Info(fmt::format("Commit date {}", GIT_COMMIT_DATE));
    Logging::Info(fmt::format("Build from branch {}", GIT_BRANCH));
    Logging::Info(fmt::format("Build with {}({})", compiler_id, compiler_version));

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
        Logging::Info(fmt::format("Simulation started with {} pedestrians", sim.GetPedsNumber()));

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
            std::thread t1(&AgentsSourcesManager::Run, &sim.GetAgentSrcManager());
            while(!sim.GetAgentSrcManager().IsRunning())
            {
                 //std::cout << "waiting...\n";
            }
           //main thread for the simulation
            evacTime = sim.RunStandardSimulation(simMaxTime);
            //Join the main thread
            t1.join();
        }
        else {
            //main thread for the simulation
            evacTime = sim.RunStandardSimulation(configuration->GetTmax());
        }

        Logging::Info(fmt::format("Simulation completed", sim.GetPedsNumber()));
        time(&endtime);

        // some statistics output
        if (configuration->ShowStatistics()) {
             sim.PrintStatistics(evacTime);// negative means end of simulation
        }

        if (sim.GetPedsNumber()) {
            Logging::Warning(fmt::format("Pedestrians not evacuated [{}] using [{}] threads",
                sim.GetPedsNumber(), configuration->GetMaxOpenMPThreads()));
        }

        const double execTime = difftime(endtime, starttime);
        Logging::Info(fmt::format("Exec Time {:.2f}s", execTime));
        Logging::Info(fmt::format("Evac Time {:.2f}s", evacTime));
        Logging::Info(fmt::format("Realtime Factor {:.2f}x", evacTime/execTime));
        Logging::Info(fmt::format("Number of Threads {}", configuration->GetMaxOpenMPThreads()));
        Logging::Info(fmt::format("Warnings {}", Log->GetWarnings()));
        Logging::Info(fmt::format("Errors {}", Log->GetErrors()));
        Logging::Info(fmt::format("Deleted Agents {}", Log->GetDeletedAgents()));
    }
    else {
        Logging::Error("Could not start simulation."
            " Check the log for prior errors");
    }
    // do the last cleaning
    delete configuration;
    delete Log;

    return (EXIT_SUCCESS);
}
