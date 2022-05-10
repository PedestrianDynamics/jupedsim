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
#pragma once

#include "Area.hpp"
#include "Macros.hpp"
#include "OperationalModelType.hpp"
#include "pedestrian/AgentsParameters.hpp"

#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

// This class provides a data container for all configuration parameters.
struct Configuration {
    unsigned int seed{0};
    double fps{8};
    unsigned int precision{2};
    double linkedCellSize{2.2};
    OperationalModelType operationalModel{OperationalModelType::GCFM};
    double tMax{500};
    double dT{0.01};
    double nuPed{0.4};
    double nuWall{0.2};
    double aPed{1.0};
    double aWall{1.0};
    double dWall{0.1};
    double dPed{0.1};
    double intPWidthPed{0.1};
    double intPWidthWall{0.1};
    double maxFPed{3.0};
    double maxFWall{3.0};
    double distEffMaxPed{2};
    double distEffMaxWall{2};
    double deltaH{0.0625};
    double wallAvoidDistance{0.4};
    bool useWallAvoidance{true};
    bool hasDirectionalEscalators{false};
    std::filesystem::path trajectoriesFile{"trajectories.txt"};
    std::filesystem::path originalTrajectoriesFile{"trajectories.txt"};
    std::filesystem::path logFile{"log.txt"};
    std::filesystem::path iniFile{};
    std::filesystem::path geometryFile{};
    std::filesystem::path transitionFile{};
    std::filesystem::path goalFile{};
    std::filesystem::path sourceFile{};
    std::filesystem::path trafficContraintFile{};
    std::optional<std::filesystem::path> eventFile{};
    std::optional<std::filesystem::path> scheduleFile{};
    std::filesystem::path trainTypeFile{};
    std::filesystem::path trainTimeTableFile{};
    std::filesystem::path projectRootDir{"."};
    std::filesystem::path outputPath{std::filesystem::absolute("results")};
    bool showStatistics{false};
    std::map<int, std::shared_ptr<AgentsParameters>> agentsParameters{};
    std::set<OptionalOutput> optionalOutput{};
    AgentColorMode agentColorMode{AgentColorMode::BY_VELOCITY};
    std::map<Area::Id, Area> areas{};
};
