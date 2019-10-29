/**
 * \file        ArgumentParser.h
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
 *
 * \section Description
 * A simple wrapper around CLI11 argument parsing
 *
 **/
#pragma once

#include "general/Filesystem.h"
#include "general/Logger.h"

#include <CLI/CLI.hpp>
#include <tuple>
#include <vector>

class ArgumentParser final
{
private:
    std::vector<std::pair<std::string, Logging::Level>> logLevelMapping{
        {"debug", Logging::Level::Debug},
        {"info", Logging::Level::Info},
        {"warning", Logging::Level::Warning},
        {"error", Logging::Level::Error},
        {"off", Logging::Level::Off}};

    fs::path iniFilePath{"ini.xml"};
    Logging::Level logLevel{Logging::Level::Info};

    CLI::App app{"JuPedSim"};
    CLI::Option * iniFilePathOpt =
        app.add_option("inifile", iniFilePath, "Path to your inifile. Defaults to 'ini.xml'")
            ->check(CLI::ExistingFile);
    CLI::Option * logLevelOpt =
        app.add_option(
               "--log-level",
               logLevel,
               "Minimum level of log messages to show. Deafults to 'info'")
            ->transform(CLI::CheckedTransformer(logLevelMapping, CLI::ignore_case));

public:
    enum class Execution { CONTINUE, ABORT };

    /// @return inifile argument. If none was parsed this defaults to 'ini.xml'
    const fs::path & IniFilePath() const;

    /// @return desired log level. If none was parsed this defauls to 'Info'
    Logging::Level LogLevel() const;

    /// Parses command line arguments
    /// Parsing ends in one of three states:
    ///     1) Everything parsed, all ok -> returns [CONTINUE, 0]
    ///     2) Error during parsing, e.g. inifile does not exist
    ///             -> [ABORT, EXIT_FAILURE]
    ///     3) Parsed -h/--help, not an error but execution should end
    ///             -> [ABORT, EXIT_SUCCESS]
    ///
    /// @param argc argument count
    /// @param argv arguments
    /// @return [Execution, ReturnCode] state after parsing. Excution describes
    ///         if the program shall continue or abort. In case 'Execution' is
    ///         ABORT returncode contains the to-be-used returncode.
    std::tuple<Execution, int> Parse(int argc, char * argv[]);
};
