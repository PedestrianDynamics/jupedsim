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
 * \section Description
 * Class handling the different output formats.
 *
 * TrajectoriesXML: xml output
 *
 * TrajectoriesTXT: txt output
 **/
#pragma once

#include "general/Macros.h"
#include "geometry/Building.h"
#include "pedestrian/AgentsSource.h"

#include <functional>
#include <vector>

class Trajectories
{
protected:
    std::shared_ptr<OutputHandler> _outputHandler;
    std::set<OptionalOutput> _optionalOutputOptions;
    std::map<OptionalOutput, std::function<std::string(Pedestrian *)>> _optionalOutput;
    std::map<OptionalOutput, std::string> _optionalOutputHeader;
    std::map<OptionalOutput, std::string> _optionalOutputInfo;

public:
    Trajectories()          = default;
    virtual ~Trajectories() = default;
    virtual void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) = 0;
    virtual void WriteGeometry(Building * building)                                            = 0;
    virtual void WriteFrame(int frameNr, Building * building)                                  = 0;
    virtual void WriteFooter()                                                                 = 0;
    virtual void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &)              = 0;
    virtual void AddOptionalOutput(OptionalOutput option) { _optionalOutputOptions.insert(option); }
    virtual void SetOptionalOutput(std::set<OptionalOutput> options)
    {
        _optionalOutputOptions = options;
    }

    void Write(const std::string & str) { _outputHandler->Write(str); }

    void SetOutputHandler(std::shared_ptr<OutputHandler> outputHandler)
    {
        _outputHandler = std::move(outputHandler);
    }
};

class TrajectoriesXML : public Trajectories
{
public:
    TrajectoriesXML() = default;

    void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) override;
    void WriteGeometry(Building * building) override;
    void WriteFrame(int frameNr, Building * building) override;
    void WriteFooter() override;
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &) override;
};


class TrajectoriesTXT : public Trajectories
{
public:
    TrajectoriesTXT();

    void WriteHeader(long nPeds, double fps, Building * building, int seed, int count) override;
    void WriteGeometry(Building * building) override;
    void WriteFrame(int frameNr, Building * building) override;
    void WriteFooter() override{};
    void WriteSources(const std::vector<std::shared_ptr<AgentsSource>> &) override{};
};