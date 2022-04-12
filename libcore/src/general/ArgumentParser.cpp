/**
 * \file        ArgumentParser.cpp
 * \copyright   <2009-2019> Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/
#include "ArgumentParser.hpp"

#include "IO/IniFileParser.hpp"
#include "IO/OutputHandler.hpp"
#include "pedestrian/AgentsParameters.hpp"

#include <Logger.hpp>

const fs::path& ArgumentParser::IniFilePath() const
{
    return iniFilePath;
}

Logging::Level ArgumentParser::LogLevel() const
{
    return logLevel;
}

bool ArgumentParser::PrintVersionAndExit() const
{
    return printVersionAndExit;
}

std::tuple<ArgumentParser::Execution, int> ArgumentParser::Parse(int argc, char* argv[])
{
    // Silence warnigns about unused member. Opts are keept as class members
    // so that declaration of all comand line options can be done in the header
    // file
    (void) iniFilePathOpt;
    (void) logLevelOpt;
    (void) versionFlag;
    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError& e) {
        return {Execution::ABORT, app.exit(e)};
    }
    return {Execution::CONTINUE, 0};
}
