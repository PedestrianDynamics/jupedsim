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

#include "../general/Macros.h"

#include <vector>


struct ConfigData_D {
    //parameters that are provided for measurement areas
    std::vector<bool> calcLocalIFD;
    std::vector<int> startFrames;
    std::vector<int> stopFrames;
    std::vector<int> areaIDs;

    //parameters for creating profiles
    bool getProfile = false;
    float gridSizeX = 0.1;
    float gridSizeY = 0.1;

    //parameters for cutting voronoi cells
    bool cutByCircle = false;
    float cutRadius  = 50;
    int circleEdges  = 6;

    //general parameters
    bool isOneDimensional = false;
    bool useBlindPoints   = true;

    std::string densityType  = "Voronoi";
    std::string velocityType = "Voronoi";

    /** @brief Parameter for velocity calculation function
     * @details Default velocity calculation function is based on the Voronoi velocity method using pedestrians' Voronoi cell and their instantaneous velocity.
     * @param[input]: polygons polygon list containing intersecting polygons with MA
     * @param[input]: individualVelocity vector with instantaneous velocities
     * @param[input]: measurementArea measurement area for which the data are calculated
     * @return: mean velocity in the MA
    **/
    std::function<double(const polygon_list &, const std::vector<double> &, const polygon_2d &)>
        velocityCalcFunc = [](const polygon_list & polygons,
                              const std::vector<double> & individualVelocity,
                              const polygon_2d & measurementArea) -> double {
        double sumV = 0;
        for(std::size_t i = 0; i < polygons.size(); ++i) {
            sumV += individualVelocity[i] * boost::geometry::area(polygons[i]);
        }
        double meanV = sumV / boost::geometry::area(measurementArea);
        return meanV;
    };

    //TODO Does it make sense to include the configs of measurement areas here as well?
};
