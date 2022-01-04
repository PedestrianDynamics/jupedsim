/**
 * \copyright   <2009-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/
#pragma once

#include "OutputHandler.h"
#include "general/Configuration.h"
#include "general/Macros.h"
#include "pedestrian/Pedestrian.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class TrajectoryWriter
{
    unsigned int _precision;
    std::set<OptionalOutput> _options;
    std::unique_ptr<OutputHandler> _outputHandler;
    std::map<OptionalOutput, std::function<std::string(Pedestrian *)>> _optionalOutput{};
    std::map<OptionalOutput, std::string> _optionalOutputHeader{};
    std::map<OptionalOutput, std::string> _optionalOutputInfo{};
    AgentColorMode _colorMode{AgentColorMode::BY_VELOCITY};

public:
    TrajectoryWriter(
        unsigned int precision,
        std::set<OptionalOutput> options,
        std::unique_ptr<OutputHandler> outputHandler,
        AgentColorMode colorMode);

    ~TrajectoryWriter() = default;

    TrajectoryWriter(const TrajectoryWriter & other) = delete;

    TrajectoryWriter & operator=(const TrajectoryWriter & other) = delete;

    TrajectoryWriter(TrajectoryWriter && other) = delete;

    TrajectoryWriter & operator=(TrajectoryWriter && other) = delete;

    void WriteHeader(size_t nPeds, double fps, const Configuration & cfg, int count);

    void WriteFrame(int frameNr, const std::vector<std::unique_ptr<Pedestrian>> & pedestrians);

private:
    int computeColor(const Pedestrian & ped) const;
};
