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
#include "geometry/Building.h"
#include "pedestrian/AgentsSourcesManager.h"

#include <Logger.h>
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
    ArgumentParser a;
    if(auto [execution, return_code] = a.Parse(argc, argv);
       execution == ArgumentParser::Execution::ABORT) {
        return return_code;
    }
    Logging::Guard guard;
    Logging::SetLogLevel(a.LogLevel());
    LOG_INFO("Starting JuPedSim - JPScore");
    LOG_INFO("Version {}", JPSCORE_VERSION);
    LOG_INFO("Commit id {}", GIT_COMMIT_HASH);
    LOG_INFO("Commit date {}", GIT_COMMIT_DATE);
    LOG_INFO("Build from branch {}", GIT_BRANCH);
    LOG_INFO("Build with {}({})", compiler_id, compiler_version);

    Configuration config;
    // TODO remove me in refactoring
    IniFileParser iniFileParser(&config);

    try {
        iniFileParser.Parse(a.IniFilePath());
    } catch(const std::exception & e) {
        LOG_ERROR("Exception in IniFileParser::Parse thrown, what: {}", e.what());

        return EXIT_FAILURE;
    }

    Simulation sim(&config);

    if(!sim.InitArgs()) {
        LOG_ERROR("Could not start simulation. Check the log for prior errors");
        return EXIT_FAILURE;
    }
    time_t starttime{};
    time(&starttime);

    double evacTime;
    LOG_INFO("Simulation started with {} pedestrians", sim.GetPedsNumber());
    try {
        evacTime = sim.RunStandardSimulation(config.GetTmax());
    } catch(const std::exception & e) {
        LOG_ERROR("Exception in Simulation::RunStandardSimulation thrown, what: {}", e.what());

        return EXIT_FAILURE;
    }
    LOG_INFO("Simulation completed");
    time_t endtime{};
    time(&endtime);

    // some statistics output
    if(config.ShowStatistics()) {
        sim.PrintStatistics(evacTime); // negative means end of simulation
    }

    if(sim.GetPedsNumber()) {
        LOG_WARNING(
            "Pedestrians not evacuated [{}] using [{}] threads",
            sim.GetPedsNumber(),
            config.GetMaxOpenMPThreads());
    }

    const double execTime = difftime(endtime, starttime);
    LOG_INFO("Exec Time {:.2f}s", execTime);
    LOG_INFO("Evac Time {:.2f}s", evacTime);
    LOG_INFO("Realtime Factor {:.2f}x", evacTime / execTime);
    LOG_INFO("Number of Threads {}", config.GetMaxOpenMPThreads());
    return (EXIT_SUCCESS);
}
