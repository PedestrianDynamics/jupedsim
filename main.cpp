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
#include "IO/IniFileParser.h"
#include "Simulation.h"
#include "general/ArgumentParser.h"
#include "general/Compiler.h"
#include "general/Configuration.h"
#include "general/Format.h"
#include "general/Logger.h"
#include "geometry/Building.h"
#include "pedestrian/AgentsSourcesManager.h"

#include <exception>
#include <iomanip>
#include <memory>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>

int main(int argc, char ** argv)
{
    // default logger
    Log = new STDIOHandler();

    ArgumentParser a;
    if(auto [execution, return_code] = a.Parse(argc, argv);
       execution == ArgumentParser::Execution::ABORT) {
        return return_code;
    }
    Logging::Guard guard;
    Logging::SetLogLevel(a.LogLevel());
    Logging::Info("Starting JuPedSim - JPScore");
    Logging::Info(fmt::format(check_fmt("Version {}"), JPSCORE_VERSION));
    Logging::Info(fmt::format(check_fmt("Commit id {}"), GIT_COMMIT_HASH));
    Logging::Info(fmt::format(check_fmt("Commit date {}"), GIT_COMMIT_DATE));
    Logging::Info(fmt::format(check_fmt("Build from branch {}"), GIT_BRANCH));
    Logging::Info(fmt::format(check_fmt("Build with {}({})"), compiler_id, compiler_version));

    Configuration config;
    // TODO remove me in refactoring
    IniFileParser iniFileParser(&config);
    try {
        if(!iniFileParser.Parse(a.IniFilePath())) {
            return EXIT_FAILURE;
        }
    } catch(const std::exception & e) {
        Logging::Error(
            fmt::format(check_fmt("Exception in IniFileParser::Parse thrown, what: {}"), e.what()));
        return EXIT_FAILURE;
    }


    // create and initialize the simulation engine
    // Simulation
    time_t starttime, endtime;
    time(&starttime);

    Simulation sim(&config);

    if(sim.InitArgs()) {
        // evacuation time
        double evacTime = 0;
        Logging::Info(
            fmt::format(check_fmt("Simulation started with {} pedestrians"), sim.GetPedsNumber()));
        if(sim.GetAgentSrcManager().GetMaxAgentNumber()) {
            // Start the thread for managing the sources of agents if any
            // std::thread t1(sim.GetAgentSrcManager());
            double simMaxTime = config.GetTmax();
            std::thread t1(&AgentsSourcesManager::Run, &sim.GetAgentSrcManager());
            while(!sim.GetAgentSrcManager().IsRunning()) {
                // std::cout << "waiting...\n";
            }
            // main thread for the simulation
            evacTime = sim.RunStandardSimulation(simMaxTime);
            // Join the main thread
            t1.join();
        } else {
            // main thread for the simulation
            evacTime = sim.RunStandardSimulation(config.GetTmax());
        }

        Logging::Info(fmt::format(check_fmt("\n\nSimulation completed"), sim.GetPedsNumber()));
        time(&endtime);

        // some statistics output
        if(config.ShowStatistics()) {
            sim.PrintStatistics(evacTime); // negative means end of simulation
        }

        if(sim.GetPedsNumber()) {
            Logging::Warning(fmt::format(
                check_fmt("Pedestrians not evacuated [{}] using [{}] threads"),
                sim.GetPedsNumber(),
                config.GetMaxOpenMPThreads()));
        }

        const double execTime = difftime(endtime, starttime);
        Logging::Info(fmt::format(check_fmt("Exec Time {:.2f}s"), execTime));
        Logging::Info(fmt::format(check_fmt("Evac Time {:.2f}s"), evacTime));
        Logging::Info(fmt::format(check_fmt("Realtime Factor {:.2f}x"), evacTime / execTime));
        Logging::Info(fmt::format(check_fmt("Number of Threads {}"), config.GetMaxOpenMPThreads()));
        Logging::Info(fmt::format(check_fmt("Warnings {}"), Log->GetWarnings()));
        Logging::Info(fmt::format(check_fmt("Errors {}"), Log->GetErrors()));
        Logging::Info(fmt::format(check_fmt("Deleted Agents {}"), Log->GetDeletedAgents()));
    } else {
        Logging::Error("Could not start simulation."
                       " Check the log for prior errors");
    }
    // do the last cleaning
    delete Log;

    return (EXIT_SUCCESS);
}
