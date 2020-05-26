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

#include <cstdarg> // va_start and va_end
#include <libshared>

FileHandler::FileHandler(const fs::path & path)
{
    _pfp.open(path.string());
    if(!_pfp.is_open()) {
        LOG_ERROR("Cannot open file {}", path.string());
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
}

void FileHandler::Write(const char * str_msg, ...)
{
    char msg[1024] = "";
    va_list ap;
    va_start(ap, str_msg);
    vsprintf(msg, str_msg, ap);
    va_end(ap);
    _pfp << msg << std::endl;
    _pfp.flush();
}
