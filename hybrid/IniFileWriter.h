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
// Created by laemmel on 11.04.16.
//

#ifndef JPSCORE_INIFILEWRITER_H
#define JPSCORE_INIFILEWRITER_H

#include "../general/Configuration.h"
#include "SimObserver.h"

class IniFileWriter
{
public:
    IniFileWriter(
        const Configuration * configuration,
        Simulation * simulation,
        SimObserver * simObserver);

    void WriteToFile(std::string file);

private:
    SimObserver * _simObserver;

    const Configuration * _configuration;
    Simulation * _simulation;

    std::unique_ptr<FileHandler> _fileHandler;

    void WriteHeader();

    void WriteBody();

    void WriteRndSeed();

    void WriteMxSimTm();

    void WriteGeoFileName();

    void WriteTrajectoryOutputDefinition();

    void WriteLogFileLocation();

    void WriteTrafficInformation();

    void WriteRouting();

    void WriteAgents();

    void WriteOpModels();

    void WriteRouteChoiceModels();

    void WriteFooter();

    void WriteGoals(std::stringstream & str);

    void WriteGompertzModel(std::stringstream & str);

    void WriteAgentParameters(std::stringstream & str);
};

#endif //JPSCORE_INIFILEWRITER_H
