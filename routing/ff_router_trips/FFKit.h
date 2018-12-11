//
// Created by arne on 3/29/17.
//
/**
 * \file        ffRouter.h
 * \date        Feb 19, 2016
 * \version     v0.8
 * \copyright   <2016-2022> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * The Floorfield Kit is the rework of the prototype FloorfieldViaFM - class.
 * It will restructur the former version to avoid several downsides (diamond
 * inheritance, line/centerpoint version (DRY), overdoing the precalc, ...)
 * We try to build it in a cleaner, easier version.
 *
 * A floorfield will be calculated and stored. The user can access distances(cost)
 * values as well as the gradient of that.
 *
 * Cost Values are calculated via the fastmarching algorithm. Other algorithms might
 * follow, if they provide better abilities to work in paralell.
 *
 * The new grid-code/subroom version should be reused for all floorfields, that work
 * on the same part of the geometry (room/subroom e.g. floor/room)
 *
 * old ffrouter description:
 * This router is an update of the former Router.{cpp, h} - Global-, Quickest
 * Router System. In the __former__ version, a graph was created with doors and
 * hlines as nodes and the distances of (doors, hlines), connected with a line-
 * of-sight, was used as edge-costs. If there was no line-of-sight, there was no
 * connecting edge. On the resulting graph, the Floyd-Warshall algorithm was
 * used to find any paths. In the "quickest-___" variants, the edge cost was not
 * determined by the distance, but by the distance multiplied by a speed-
 * estimate, to find the path with minimum travel times. This whole construct
 * worked pretty well, but dependend on hlines to create paths with line-of-
 * sights to the next target (hline/door).
 *
 * In the ffRouter, we want to overcome hlines by using floor fields to
 * determine the distances. A line of sight is not required any more. We hope to
 * reduce the graph complexity and the preparation-needs for new geometries.
 *
 * To find a counterpart for the "quickest-____" router, we can either use
 * __special__ floor fields, that respect the travel time in the input-speed map,
 * or take the distance-floor field and multiply it by a speed-estimate (analog
 * to the former construct.
 *
 * We will derive from the <Router> class to fit the interface.
 *
 * Questions to solve: how to deal with goalID == doorID problem in matrix
 *
 * Restrictions/Requirements: Floorfields are not really 3D supporting:
 *
 * A room may not consist of subrooms which overlap in their projection (onto
 * x-y-plane). So subrooms, that are positioned on top of others (in stairways
 * for example), must be separated into different rooms.
 *
 * floorfields do not consider z-coordinates. Distances of two grid points are
 * functions of (x, y) and not (x, y, z). Any slope will be neglected.
 *
 **/

#ifndef JPSCORE_FFKIT_H
#define JPSCORE_FFKIT_H

#include <vector>
#include <unordered_set>
#include <cmath>
#include <functional>
#include "mesh/RectGrid.h"
#include "../../geometry/Wall.h"
#include "../../geometry/Point.h"
#include "../../geometry/Building.h"
#include "../../geometry/SubRoom.h" //check: should Room.h include SubRoom.h??
#include "./mesh/Trial.h"
#include "../../pedestrian/Pedestrian.h"

//class Building;
//class Pedestrian;
class OutputHandler;

//log output
extern OutputHandler* Log;




#endif //JPSCORE_FFKIT_H
