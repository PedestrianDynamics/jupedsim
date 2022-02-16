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

#include "direction/walking/DirectionStrategy.hpp"
#include "general/Configuration.hpp"
#include "general/Filesystem.hpp"

#include <string>

class OutputHandler;

class TiXmlElement;

class TiXmlNode;

class IniFileParser
{
public:
    IniFileParser(Configuration * config);
    ~IniFileParser(){};

    void Parse(const fs::path & iniFile);

private:
    bool ParseHeader(TiXmlNode * xHeader);

    bool ParseGCFMModel(TiXmlElement * xGCFM, TiXmlElement * xMain);

    bool ParseVelocityModel(TiXmlElement * xVelocity, TiXmlElement * xMain);

    void ParseAgentParameters(TiXmlElement * operativModel, TiXmlNode * agentDistri);

    bool ParseRoutingStrategies(TiXmlNode * routingNode, TiXmlNode * agentDistri);

    bool ParseLinkedCells(const TiXmlNode & linkedCellNode);

    bool ParseStepSize(const TiXmlNode & stepNode);

    bool ParseStrategyNodeToObject(const TiXmlNode & strategyNode);

    bool ParseFfOpts(const TiXmlNode & strategyNode);

    bool ParseExternalFiles(const TiXmlNode & xMain);

    std::optional<GlobalRouterParameters> ParseGlobalRouterParmeters(const TiXmlElement * e);

    Configuration * _config;
    int _model;
};
