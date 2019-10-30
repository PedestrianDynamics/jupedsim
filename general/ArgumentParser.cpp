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
#include "ArgumentParser.h"

#include "IO/IniFileParser.h"
#include "IO/OutputHandler.h"
#include "general/Logger.h"
#include "general/OpenMP.h"
#include "pedestrian/AgentsParameters.h"
#include "routing/global_shortest/GlobalRouter.h"
#include "routing/quickest/QuickestPathRouter.h"
#include "routing/smoke_router/SmokeRouter.h"


const fs::path & ArgumentParser::IniFilePath() const
{
    return iniFilePath;
}

Logging::Level ArgumentParser::LogLevel() const
{
    return logLevel;
}

std::tuple<ArgumentParser::Execution, int> ArgumentParser::Parse(int argc, char * argv[])
{
    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError & e) {
        return {Execution::ABORT, app.exit(e)};
    }
    return {Execution::CONTINUE, 0};
}
