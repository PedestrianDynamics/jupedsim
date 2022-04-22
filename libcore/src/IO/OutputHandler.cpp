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
#include "OutputHandler.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>

FileHandler::FileHandler(const fs::path& path)
{
    if(path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }
    _pfp.open(path.string());
    if(!_pfp.good()) {
        throw std::runtime_error(fmt::format(FMT_STRING("Cannot open {:s}"), path.string()));
    }
}

FileHandler::~FileHandler()
{
    _pfp.close();
}

void FileHandler::Write(const std::string& str)
{
    if(!_pfp.good()) {
        throw std::runtime_error("Error writing trajectories");
    }
    _pfp << str << std::endl;
}
