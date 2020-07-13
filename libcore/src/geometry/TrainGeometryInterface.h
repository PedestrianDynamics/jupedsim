/** JuPedSim -- Jülich Pedestrian Dynamics Simulator
 *
 *  Copyright <2009-2020> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 *  This file is part of JuPedSim.
 *
 *  JuPedSim is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  JuPedSim is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with JuPedSim.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "Wall.h"

#include <string>
#include <vector>

/**
 * Information where a specific track is located, and which walls describe the platform edges.
 */
struct Track {
    int _id;                  /** ID of track, used for describing where a train arrives/departs. */
    int _roomID;              /** ID of room, where the track is located. */
    int _subRoomID;           /** ID of subroom, where the track is located. */
    std::vector<Wall> _walls; /** Platform edges. */
};

/**
 * Information of train doors, as position, width, and allowed flow.
 */
struct TrainDoor {
    double _distance; /** Distance to start of train. */
    double _width;    /** Width of train door. */
    double _flow;     /** Max. allowed flow at train door. */
};

/**
 * Information of the train.
 */
struct TrainType {
    std::string _type; /** Name of the train type, used for stating which train arrives/deptarts. */
    int _maxAgents;    /** Max. number of agents allowed in the train. */
    double _length;    /** Length of the train. */
    std::vector<TrainDoor> _doors; /** Doors of the train. */
};
