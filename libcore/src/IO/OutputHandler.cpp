/**
 * \file        OutputHandler.cpp
 * \date        Nov 20, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 *
 **/
#include "OutputHandler.h"

#include <cmath>
#include <cstdarg> // va_start and va_end

void OutputHandler::incrementWarnings()
{
    _nWarnings += 1;
}

int OutputHandler::GetWarnings()
{
    return _nWarnings;
}

void OutputHandler::incrementErrors()
{
    _nErrors += 1;
}

int OutputHandler::GetErrors()
{
    return _nErrors;
}


void OutputHandler::incrementDeletedAgents()
{
    _nDeletedAgents += 1;
}

int OutputHandler::GetDeletedAgents()
{
    return _nDeletedAgents;
}


void OutputHandler::Write(const std::string & str)
{
    std::cout << std::endl << str;
}

void OutputHandler::Write(const char * message, ...)
{
    char msg[CLENGTH] = "";
    va_list ap;
    va_start(ap, message);
    vsprintf(msg, message, ap);
    va_end(ap);

    std::string str(msg);

    if(str.find("ERROR") != std::string::npos) {
        std::cerr << std::endl << msg;
        std::cerr.flush();
        incrementErrors();
    } else if(str.find("WARNING") != std::string::npos) {
        std::cerr << std::endl << msg;
        std::cerr.flush();
        incrementWarnings();
    } else { // infos
        std::cout << std::endl << msg;
        std::cout.flush();
    }
}

void STDIOHandler::Write(const std::string & str)
{
    if(str.find("ERROR") != std::string::npos) {
        std::cerr << std::endl << str;
        std::cerr.flush();
        incrementErrors();
    } else if(str.find("WARNING") != std::string::npos) {
        std::cerr << std::endl << str;
        std::cerr.flush();
        incrementWarnings();
    } else { // infos
        std::cout << std::endl << str;
        std::cout.flush();
    }
}

FileHandler::FileHandler(const fs::path & path)
{
    _pfp.open(path.string());
    if(!_pfp.is_open()) {
        std::cerr << "Error!!! File " << path << " could not be opened" << std::endl;
        exit(0);
    }
}

FileHandler::~FileHandler()
{
    _pfp.close();
}

void FileHandler::Write(const std::string & str)
{
    _pfp << str << std::endl;
    _pfp.flush();

    if(str.find("ERROR") != std::string::npos) {
        incrementErrors();
    } else if(str.find("WARNING") != std::string::npos) {
        incrementWarnings();
    }
}

void FileHandler::Write(const char * str_msg, ...)
{
    char msg[CLENGTH] = "";
    va_list ap;
    va_start(ap, str_msg);
    vsprintf(msg, str_msg, ap);
    va_end(ap);
    _pfp << msg << std::endl;
    _pfp.flush();

    std::string str(msg);
    if(str.find("ERROR") != std::string::npos) {
        incrementErrors();
    } else if(str.find("WARNING") != std::string::npos) {
        incrementWarnings();
    }
}
