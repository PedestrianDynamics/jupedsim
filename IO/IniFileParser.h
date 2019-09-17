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
//
// Created by laemmel on 24.03.16.
//
#pragma once

#include "general/Configuration.h"
#include "general/Filesystem.h"

#include <string>

class OutputHandler;

class TiXmlElement;

class TiXmlNode;

class DirectionManager;

class WaitingStrategy;

extern OutputHandler* Log;

class IniFileParser {

public:
     IniFileParser(Configuration* config);
     ~IniFileParser(){};

     bool Parse(const fs::path& iniFile);

private:
     bool ParseHeader(TiXmlNode* xHeader);

     bool ParseGCFMModel(TiXmlElement* xGCFM, TiXmlElement* xMain);

     bool ParseKrauszModel(TiXmlElement* xKrausz, TiXmlElement* xMain);

     bool ParseGompertzModel(TiXmlElement* xGompertz, TiXmlElement* xMain);

     bool ParseGradientModel(TiXmlElement* xGradient, TiXmlElement* xMain);

     bool ParseVelocityModel(TiXmlElement* xVelocity, TiXmlElement* xMain);

     void ParseAgentParameters(TiXmlElement* operativModel, TiXmlNode* agentDistri);

     bool ParseRoutingStrategies(TiXmlNode* routingNode, TiXmlNode* agentDistri);

     bool ParseFfRouterOps(TiXmlNode* routingNode, RoutingStrategy s);

     bool ParseCogMapOpts(TiXmlNode* routingNode);
#ifdef AIROUTER
     bool ParseAIOpts(TiXmlNode* routingNode);
#endif
     bool ParseLinkedCells(const TiXmlNode& linkedCellNode);

     bool ParseStepSize(TiXmlNode& stepNode);

     bool ParsePeriodic(TiXmlNode& Node);

     bool ParseNodeToSolver(const TiXmlNode& solverNode);

     bool ParseStrategyNodeToObject(const TiXmlNode& strategyNode);

     bool ParseFfOpts(const TiXmlNode& strategyNode);

     Configuration* _config;
     int _model;
     std::shared_ptr<DirectionStrategy> _directionStrategy;
     std::shared_ptr<WaitingStrategy> _waitingStrategy;
     std::shared_ptr<DirectionManager> _directionManager;
    int _exit_strat_number;

};