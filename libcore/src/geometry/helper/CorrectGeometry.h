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

bool CorrectSubRoom(SubRoom & subRoom);

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
 * Projects the door coordinates to the walls of the track.
 * @param trackWalls walls of the track
 * @param trainDoors doors of the train
 * @return Vector containing: [[Point1 of door, wall containing Point1],
 *                             [Point2 of door, wall containing Point2]]
 */
std::vector<std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>> ComputeTrainDoorCoordinates(
    const std::vector<Wall> & trackWalls,
    const std::vector<Transition> & trainDoors);

/**
 * Interface for adding trains to the geometry. Adds the \p trainDoors to the given \p subroom and
 * updates afterwards. \p trainDoors will also be add to \p building. Geometry changes added/removed
 * walls and added doors will be saved with \p trainID as identifier.
 * @param trainID ID of the arriving train
 * @param building simulation geometry
 * @param subroom subroom containing the platform the train is arriving
 * @param trackWalls platform edges where the train might arrive
 * @param trainDoors doors of the train
 */
void AddTrainDoors(
    int trainID,
    Building & building,
    SubRoom & subroom,
    const std::vector<Wall> & trackWalls,
    const std::vector<Transition> & trainDoors);

/**
 * Splits the given \p trackWalls at the \p wallDoorIntersectionPoints. The new line segments to
 * create a closed polygon are returned in addedWalls (size 2). Each wall between the door points is
 * returned in removedDoors.
 * @param wallDoorIntersectionPoints door coordinates and the corresponding wall containings these
 * points
 * @param trackWalls platform edges where the train might arrive
 * @param door train door to add
 * @return [addedWalls, removedWalls]
 */
std::tuple<std::vector<Wall>, std::vector<Wall>> SplitWall(
    const std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> & wallDoorIntersectionPoints,
    const std::vector<Wall> & trackWalls,
    const Transition & door);

} // namespace geometry::helper
