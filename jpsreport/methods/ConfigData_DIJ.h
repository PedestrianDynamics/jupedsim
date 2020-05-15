/**
 * \file        ConfigData_DIJ.h
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
 * A simple class for storing the configuration data for Method D, I and J
 *
 **/

#pragma once

#include <vector>

struct ConfigData_DIJ {
    //parameters that are provided for measurement areas
    std::vector<bool> individual_FD_flags;
    std::vector<int> start_frames;
    std::vector<int> stop_frames;
    std::vector<int> areaIDs;

    //parameters for creating profiles
    bool getProfile   = false;
    float grid_size_X = 0.1;
    float grid_size_Y = 0.1;

    //parameters for cutting voronoi cells
    bool cutByCircle = false;
    float cutRadius  = 50;
    int circleEdges  = 6;

    //general parameters
    bool isOneDimensional = false;
    //TODO Does it make sense to include the configs of measurement areas here as well?
};
