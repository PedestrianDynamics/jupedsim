/**
 * \file        ConfigData_DIJ.cpp
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
 **/

#include "ConfigData_DIJ.h"

ConfigData_DIJ::ConfigData_DIJ()
{
    // Todo How to init?
    _individual_FD_flags;
    _start_frames;
    _stop_frames;

    _getProfile  = false;
    _grid_size_X = 0.10;
    _grid_size_Y = 0.10;

    _cutByCircle = false;
    _cutRadius   = -1;
    _circleEdges = -1;

    _isOneDimensional = false;
};