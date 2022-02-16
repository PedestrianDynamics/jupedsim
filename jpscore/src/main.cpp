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
#include "IO/EventFileParser.hpp"
#include "IO/IniFileParser.hpp"
#include "IO/TrainFileParser.hpp"
#include "Simulation.hpp"
#include "agent-creation/AgentCreator.hpp"
#include "events/Event.hpp"
#include "events/EventManager.hpp"
#include "events/EventVisitors.hpp"
#include "general/ArgumentParser.hpp"
#include "general/Compiler.hpp"
#include "general/Configuration.hpp"
#include "geometry/Building.hpp"
#include "math/OperationalModel.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"
#include "routing/RoutingEngine.hpp"

#include <Logger.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>

int main(int argc, char ** argv)
{
    ArgumentParser a;
    if(auto [execution, return_code] = a.Parse(argc, argv);
       execution == ArgumentParser::Execution::ABORT) {
        return return_code;
    }
    if(a.PrintVersionAndExit()) {
        std::cout << fmt::format("Version {}\n", JPSCORE_VERSION)
                  << fmt::format("Commit id {}\n", GIT_COMMIT_HASH)
                  << fmt::format("Commit date {}\n", GIT_COMMIT_DATE)
                  << fmt::format("Build from branch {}\n", GIT_BRANCH)
                  << fmt::format("Build with {}({})", compiler_id, compiler_version) << std::endl;
        return 0;
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

    auto building       = std::make_unique<Building>(&config, nullptr);
    auto * building_ptr = building.get();
    auto agents         = CreateAllPedestrians(&config, building.get(), config.tMax);
    Simulation sim(&config, std::move(building));
    EventManager manager;

    size_t frame   = 0;
    int num_agents = agents.size();

    while(!agents.empty()) {
        double now = sim.Clock().dT() * frame;
        auto t     = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(now));
        auto events = CreateEventsFromAgents(extract(agents, frame), t);
        for(auto && evt : events) {
            manager.AddEvent(evt);
        }
        ++frame;
    }

    if(config.eventFile) {
        try {
            const auto door_events = EventFileParser::ParseDoorEvents(*config.eventFile);
            for(auto && evt : door_events) {
                manager.AddEvent(evt);
            }
        } catch(const std::exception & e) {
            LOG_ERROR("Error parsing events: {}", e.what());
            return EXIT_FAILURE;
        }
    }
    if(config.scheduleFile) {
        try {
            const auto train_door_events =
                EventFileParser::ParseSchedule(config.scheduleFile.value());
            for(auto && evt : train_door_events) {
                manager.AddEvent(evt);
            }
            const auto groupMaxAgents =
                EventFileParser::ParseMaxAgents(config.scheduleFile.value());
            for(auto const & [transID, maxAgents] : groupMaxAgents) {
                building_ptr->GetTransition(transID)->SetMaxDoorUsage(maxAgents);
            }
        } catch(const std::exception & e) {
            LOG_ERROR("Error parsing schedule file {}", e.what());
            return EXIT_FAILURE;
        }
    }
    if(!config.trainTimeTableFile.empty() && !config.trainTypeFile.empty()) {
        try {
            //TODO(kkratz) Have another look at the error handling
            auto trainTypes = TrainFileParser::ParseTrainTypes(config.trainTypeFile);
            const auto timeTableContents =
                TrainFileParser::ParseTrainTimeTable(trainTypes, config.trainTimeTableFile);
            for(auto && evt : timeTableContents.events) {
                manager.AddEvent(evt);
            }
            for(auto && [k, v] : timeTableContents.trains) {
                building_ptr->AddTrainType(k, v);
            }
        } catch(const std::exception & e) {
            LOG_ERROR("Error parsing train files: {}", e.what());
            return EXIT_FAILURE;
        }
    }
    if(!sim.InitArgs()) {
        LOG_ERROR("Could not start simulation. Check the log for prior errors");
        return EXIT_FAILURE;
    }
    time_t starttime{};
    time(&starttime);

    double evacTime{0};
    LOG_INFO("Simulation started with {} pedestrians", sim.GetPedsNumber());
    try {
        auto writer = std::make_unique<TrajectoryWriter>(
            config.precision,
            config.optionalOutput,
            std::make_unique<FileHandler>(config.trajectoriesFile),
            config.agentColorMode);
        sim.RunHeader(num_agents, *writer);

        const int writeInterval = static_cast<int>((1. / sim.Fps()) / sim.Clock().dT() + 0.5);

        while((!sim.Agents().empty() || manager.HasEventsAfter(sim.Clock())) &&
              sim.Clock().ElapsedTime() < config.tMax) {
            auto next_events = manager.NextEvents(sim.Clock());
            for(auto const & [_, event] : next_events) {
                // lambda is used to bind additional function paramters to the visitor
                auto visitor = [&sim](auto event) { ProcessEvent(event, sim); };
                std::visit(visitor, event);
            }
            if(sim.Clock().Iteration() == 0) {
                writer->WriteFrame(0, sim.Agents());
            }
            sim.Iterate();
            // write the trajectories
            if(0 == sim.Clock().Iteration() % writeInterval) {
                writer->WriteFrame(sim.Clock().Iteration() / writeInterval, sim.Agents());
            }


            if(sim.Clock().Iteration() % 1000 == 0) {
                if(config.showStatistics) {
                    LOG_INFO("Update door statistics at t={:.2f}", sim.Clock().ElapsedTime());
                    sim.PrintStatistics(sim.Clock().ElapsedTime());
                }
            }
        }
        evacTime = sim.Clock().ElapsedTime();

    } catch(const std::exception & e) {
        LOG_ERROR("Exception in Simulation::RunStandardSimulation thrown, what: {}", e.what());

        return EXIT_FAILURE;
    }
    LOG_INFO("Simulation completed");
    time_t endtime{};
    time(&endtime);

    // some statistics output
    if(config.showStatistics) {
        sim.PrintStatistics(evacTime); // negative means end of simulation
    }

    if(sim.GetPedsNumber()) {
        LOG_WARNING("Pedestrians not evacuated [{}]", sim.GetPedsNumber());
    }

    const double execTime = difftime(endtime, starttime);
    LOG_INFO("Exec Time {:.2f}s", execTime);
    LOG_INFO("Evac Time {:.2f}s", evacTime);
    LOG_INFO("Realtime Factor {:.2f}x", evacTime / execTime);
    return (EXIT_SUCCESS);
}
