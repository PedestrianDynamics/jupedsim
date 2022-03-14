/**
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
 *
 *
 **/
#pragma once

class Building;
class Wall;
class SubRoom;
class Line;
class Point;
class Transition;

#include "Geometry.hpp"
#include "geometry/TrainGeometryInterface.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

namespace geometry::helper
{
/**
  * @brief Computes the train door coordinates by finding the points with the distance and width given by
  * \p train on \p track, considering \p trainStartOffset and \p fromEnd.
  *
  * @param train train to be added to geometry
  * @param track track at which the train should be added
  * @param trainStartOffset distance the train start has from the track start
  * @param fromEnd train should be added from end or beginning of track walls
  * @return All found endpoints of the train doors on the track walls
  */
std::map<int, std::pair<Point, Point>> ComputeTrainDoorCoordinates(
    const TrainType & train,
    const Track & track,
    double trainStartOffset,
    bool fromEnd);

/**
 * @brief Adds train doors defined by \p train, \p trainStartOffset, and \p fromEnd to \p subroom
 * and \p building.
 *
 * Interface for adding trains to the geometry. Adds the \p trainDoors to the given \p subroom and
 * updates afterwards. \p trainDoors will also be add to \p building. Geometry changes added/removed
 * walls and added doors will be saved with \p trainID as identifier.
 *
 * @param trainID ID of the arriving train
 * @param trackID ID where the train should be
 * @param building simulation geometry
 * @param train train to be added to geometry
 * @param trainStartOffset distance the train start has from the track start
 * @param fromEnd train should be added from end or beginning of track walls
 */
void AddTrainDoors(
    int trainId,
    int trackId,
    Building & building,
    const TrainType & train,
    double trainStartOffset,
    bool fromEnd,
    Geometry & geometry);

/**
 * Splits the given \p trackWalls with the \p doors. The new line segments to create a closed
 * polygon are returned in addedWalls (size 2). Each wall between the door points is returned in
 * removedWalls.
 * @param walls walls containing the \p doors
 * @param doors doors to be included into the \p walls
 * @return [addedWalls, removedWalls] walls need to be added/removed to create a closed line
 */
std::tuple<std::vector<Wall>, std::vector<Wall>>
SplitWalls(const std::vector<Wall> & walls, const std::vector<Transition> & doors);

/**
 * @brief Sorts the \p walls, to create a line chain, starting with the wall containing \p start
 *
 * The walls in \p walls will be sorted in a way, that the first wall has \p start as Point1. All
 * lines will be connected in a way that wall.Point2 == nextWall.Point1.
 *
 * @throw std::runtime_error If no continuous line chain can be created from the given \p walls
 * @throw std::runtime_error No wall in \p walls has endpoint \p start.
 *
 * @param trackWalls in: unsorted track walls, out: sorted track walls
 * @param start start point of the sorted walls (is an endpoint of one of the walls in \p walls)
 */
void SortWalls(std::vector<Wall> & walls, const Point & start);

/**
 * @brief Finds a point with \p distance to starting point along the \p walls.
 * @pre trackWalls sorted, such that first element contains track start, last element contains
 * track end. [trackStart] -> [wall] -> ... -> [trackEnd]
 * @warning: only looks for the point in walls following (including) wall containing \p starting
 * point
 *
 * @param walls walls to look for point with \p distance.
 * @param origin reference point the distance is measured from.
 * @param distance distance from origin to desired point
 * @return if point with \p distance exist desired point, else std::nullopt
 */
std::optional<Point> FindWallPointWithDistanceOnWall(
    const std::vector<Wall> & walls,
    const Point & origin,
    double distance);

} // namespace geometry::helper
