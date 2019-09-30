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
// Created by laemmel on 26.02.16.
//

#ifndef JPSCORE_HYBRIDSIMULATION_H
#define JPSCORE_HYBRIDSIMULATION_H

#include "../IO/OutputHandler.h"
#include "../general/Configuration.h"
#include "JPSserver.h"
#include "Latches.h"
#include "SimObserver.h"

#include <grpc++/grpc++.h>
#include <string>

using grpc::Server;

class Building;

class Simulation;

class JPSserver;

class Configuration;

extern OutputHandler * Log;

class HybridSimulationManager
{
public:
    HybridSimulationManager(Configuration * config);

    virtual ~HybridSimulationManager();

    bool Run(Simulation & sim);

    std::string ToString();

    void Start();

    void Shutdown();

    void WaitForScenarioLoaded();

    SimObserver * GetSimObserver();

private:
    //    Building* _building=nullptr;
    SimObserver _simObserver;

    Configuration * _config;

    std::shared_ptr<Latches> _latches = nullptr;
    JPSserver * _service;

    int _jpsServerPort              = 9998;
    std::shared_ptr<Server> _server = nullptr;
};

#endif //JPSCORE_HYBRIDSIMULATION_H
