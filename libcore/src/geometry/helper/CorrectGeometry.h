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

#include "geometry/TrainGeometryInterface.h"

#include <optional>
#include <vector>

namespace geometry::helper
{
/**
 * \ingroup
 * Correct geometries by deleting "big" walls
 * In a subroom, "big" refers to a wall that intersects with other walls in the same subroom in a point,
 * which does not coincide with one of the end points.
 * For example:
 *
 *```
 *                            C
 *  A x------------------------o-----------------x B
 *                ^            |
 *                wall1        |
 *                             |  <--- wall2
 *                             o
 *                             D
 *```
 *  Here wall `[AB]` is a big wall, then it intersects wall `[CD]` whether in A nor in B
 *  What happens in this method:
 *  1. `[AB]` will be splited into two segments`[AC]` and `[CB]`
 *  2. `[AB]` will be removed
 *  3. `[AC]` or `[CB]` will be added to the subroom
 *
 */
void CorrectInputGeometry(Building & building);

bool RemoveOverlappingWall(const Line & exit, SubRoom & subroom);

/**
 * @brief Removes doors on walls
 */
bool RemoveWallsOverlappingWithDoors(SubRoom & subroom);
std::optional<Point> ComputeSplitPoint(const Wall & wall, const Line & line);

/**
 * @brief Split a wall in several small walls
 *
 * search all walls+crossings+transitions that intersect <bigwall>
 * not in an endpoint
 *
 * @param subroom: subroom containing <bigwall>
 * @param bigWall: wall to split
 * @return std::vector: a vector of all small walls. Can be empty.
 */
std::optional<std::vector<Wall>> SplitWall(const SubRoom & subroom, const Wall & bigWall);

/**
 * @brief Checks if the wall is inside the polygon of the subroom and does not exist already
 */
bool IsConnectedWall(const SubRoom & subroom, const Wall & wall);

/**
 * @brief Adds wall pieces to a subroom if they are new and connected
 */
int AddConnectedWallsToSubroom(SubRoom & subroom, const std::vector<Wall> & wallPieces);

/**
 * @brief Replace BigWall with some of the wallPieces
 */
void ReplaceBigWall(SubRoom & subroom, const Wall & bigWall, const std::vector<Wall> & wallPieces);
bool RemoveBigWalls(SubRoom & subroom);

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
std::vector<std::pair<Point, Point>> ComputeTrainDoorCoordinates(
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
 * @param building simulation geometry
 * @param subroom subroom containing the platform the train is arriving
 * @param train train to be added to geometry
 * @param track track at which the train should be added
 * @param trainStartOffset distance the train start has from the track start
 * @param fromEnd train should be added from end or beginning of track walls
 */
void AddTrainDoors(
    int trainID,
    Building & building,
    SubRoom & subroom,
    const TrainType & train,
    const Track & track,
    double trainStartOffset,
    bool fromEnd);

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
