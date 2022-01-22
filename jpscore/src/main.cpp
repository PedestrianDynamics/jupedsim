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
#include "GCFMModel.hpp"
#include "GeometryBuilder.hpp"
#include "IO/GeoFileParser.hpp"
#include "IO/IniFileParser.hpp"
#include "IO/Trajectories.hpp"
#include "IteratorPair.hpp"
#include "OperationalModel.hpp"
#include "ResultHandling.hpp"
#include "RoutingEngine.hpp"
#include "Simulation.hpp"
#include "VelocityModel.hpp"
#include "agent-creation/AgentCreator.hpp"
#include "events/Event.hpp"
#include "events/EventManager.hpp"
#include "events/EventVisitors.hpp"
#include "general/ArgumentParser.hpp"
#include "general/Compiler.hpp"
#include "general/Configuration.hpp"
#include "geometry/Building.hpp"
#include "geometry/SubRoom.hpp"
#include "pedestrian/AgentsSourcesManager.hpp"

#include <Logger.hpp>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <fmt/format.h>
#include <iostream>
#include <iterator>
#include <memory>

std::unique_ptr<OperationalModel>
CreateFromType(OperationalModelType type, const Configuration& config)
{
    switch(type) {
        case OperationalModelType::GCFM:
            return std::make_unique<GCFMModel>(
                config.nuPed,
                config.nuWall,
                config.distEffMaxPed,
                config.distEffMaxWall,
                config.intPWidthPed,
                config.intPWidthWall,
                config.maxFPed,
                config.maxFWall);
        case OperationalModelType::VELOCITY:
            return std::make_unique<VelocityModel>(
                config.aPed, config.dPed, config.aWall, config.dWall);
    }
}

int main(int argc, char** argv)
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
    try {
        Logging::SetLogLevel(a.LogLevel());
        LOG_INFO("Starting JuPedSim - JPScore");
        LOG_INFO("Version {}", JPSCORE_VERSION);
        LOG_INFO("Commit id {}", GIT_COMMIT_HASH);
        LOG_INFO("Commit date {}", GIT_COMMIT_DATE);
        LOG_INFO("Build from branch {}", GIT_BRANCH);
        LOG_INFO("Build with {}({})", compiler_id, compiler_version);

        auto config = ParseIniFile(a.IniFilePath());
        auto building = std::make_unique<Building>(&config);
        auto agents = CreateAllPedestrians(&config, building.get(), config.tMax);

        GeometryBuilder geometryBuilder{};
        for(const auto& [_, room] : building->GetAllRooms()) {
            for(const auto& [_, sub_room] : room->GetAllSubRooms()) {
                geometryBuilder.AddAccessibleArea(sub_room->GetPolygon());
                for(const auto& o : sub_room->GetAllObstacles()) {
                    geometryBuilder.ExcludeFromAccessibleArea(o->GetPolygon());
                }
            }
        }
        auto geometry = geometryBuilder.Build();

        Simulation sim(
            CreateFromType(config.operationalModel, config),
            std::move(geometry.collisionGeometry),
            std::move(geometry.routingEngine),
            std::make_unique<Areas>(std::move(config.areas)),
            config.dT);
        EventManager manager;

        size_t frame = 0;
        const auto num_agents_in_simulation = agents.size();

        while(!agents.empty()) {
            double now = sim.Clock().dT() * frame;
            auto t = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(now));
            auto events = CreateEventsFromAgents(extract(agents, frame), t);
            for(auto&& evt : events) {
                manager.AddEvent(evt);
            }
            ++frame;
        }

        collectInputFilesIn(config.iniFile, "results");
        time_t starttime{};
        time(&starttime);

        double evacTime{0};
        auto writer = std::make_unique<TrajectoryWriter>(
            config.precision,
            config.optionalOutput,
            std::make_unique<FileHandler>(config.trajectoriesFile),
            config.agentColorMode);
        writer->WriteHeader(num_agents_in_simulation, config.fps, config, 0);

        const int writeInterval = static_cast<int>((1. / config.fps) / sim.Clock().dT() + 0.5);
        LOG_INFO("Starting simulation");
        while((!sim.Agents().empty() || manager.HasEventsAfter(sim.Clock())) &&
              sim.Clock().ElapsedTime() < config.tMax) {
            auto next_events = manager.NextEvents(sim.Clock());
            for(auto const& [_, event] : next_events) {
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
        }
        evacTime = sim.Clock().ElapsedTime();

        LOG_INFO("Simulation completed");
        time_t endtime{};
        time(&endtime);

        if(sim.AgentCount()) {
            LOG_WARNING("Pedestrians not evacuated [{}]", sim.AgentCount());
        }

        const double execTime = difftime(endtime, starttime);
        LOG_INFO("Exec Time {:.2f}s", execTime);
        LOG_INFO("Evac Time {:.2f}s", evacTime);
        LOG_INFO("Realtime Factor {:.2f}x", evacTime / execTime);
        return (EXIT_SUCCESS);
    } catch(const std::exception& ex) {
        LOG_ERROR("{:s}", ex.what());
    } catch(...) {
        LOG_ERROR("Unknown exception encountered!");
    }
    return EXIT_FAILURE;
}
