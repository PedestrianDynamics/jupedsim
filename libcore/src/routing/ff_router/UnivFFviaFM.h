//
// Created by arne on 5/9/17.
//
/**
 * \file        UnivFFviaFM.h
 * \date        May 09, 2017
 * \version     N/A (v0.8.x)
 * \copyright   <2017-2020> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * Implementation of classes for a reworked floorfield. A floorfield in general
 * yields a cost field to a specific goal and a correlated vectorfield for the
 * optimal path in terms of the cost value.
 *
 * Rework focused on a cleaner structure and less inheritance (no diamond) and
 * less workarounds.
 *
 *
 **/
#pragma once

#include "general/Filesystem.h"
#include "general/Macros.h"

#include <map>
#include <string>
#include <vector>

class Pedestrian;
class Room;
class SubRoom;
class Building;
class Configuration;
class Point;
class RectGrid;
class Line;

/**
 * Class used to compare cost (distances) in std::priority_queue in UnivFFviaFM::calcFF.
 */
class CompareCostTrips
{
public:
    /**
     * Constructor.
     * @param costarray costs at each cell of grid.
     */
    CompareCostTrips(double * costarray) : _costarray(costarray) {}

    /**
     * Comparison operator, returns if cost of a is larger than cost of b.
     * @param a index of cell 1.
     * @param b index of cell 2.
     * @return cost of a is larger than cost of b.
     */
    bool operator()(const int a, const int b) const { return _costarray[a] > _costarray[b]; }

private:
    /**
     * cost (distances) of grid cells.
     */
    double * _costarray = nullptr;
};

class UnivFFviaFM
{
public:
    /**
     * Constructs the floor field in a specific room.
     * @param room room in which the floor field should be created.
     * @param building geometry used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     */
    UnivFFviaFM(
        Room * room,
        Building * building,
        double hx,
        double wallAvoid,
        bool useWallDistances);

    /**
     * Constructs the floor field in a specific subroom.
     * @param subroom subroom in which the floor field should be created.
     * @param building geometry used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     */
    UnivFFviaFM(
        SubRoom * subroom,
        Building * building,
        double hx,
        double wallAvoid,
        bool useWallDistances);

    /**
     * Constructs the floor field in a specific room.
     * @param room room in which the floor field should be created.
     * @param config configuration used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     */
    UnivFFviaFM(
        Room * room,
        Configuration * config,
        double hx,
        double wallAvoid,
        bool useWallDistances);

    /**
     * Constructs the floor field in a specific room.
     * @param room room in which the floor field should be created.
     * @param config configuration used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     * @param wantedDoors doors which should be considered
     *
     * @todo check if this is really needed, wantedDoors always set to empty vector.
     */
    UnivFFviaFM(
        Room * room,
        Configuration * config,
        double hx,
        double wallAvoid,
        bool useWallDistances,
        const std::vector<int> & wantedDoors);

    /**
     * Constructs the floor field in a specific subroom.
     * @param subroom subroom in which the floor field should be created.
     * @param config configuration used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     */
    UnivFFviaFM(
        SubRoom * subroom,
        Configuration * config,
        double hx,
        double wallAvoid,
        bool useWallDistances);

    /**
     * Constructs the floor field in a specific room.
     * @param subroom subroom in which the floor field should be created.
     * @param config configuration used in simulation.
     * @param hx grid size for floor fields.
     * @param wallAvoid wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered in floor field.
     * @param wantedDoors doors which should be considered
     *
     * @todo check if this is really needed, wantedDoors always set to empty vector.
     */
    UnivFFviaFM(
        SubRoom * subroom,
        Configuration * config,
        double hx,
        double wallAvoid,
        bool useWallDistances,
        const std::vector<int> & wantedDoors);

    /**
     * Disable default constructor.
     */
    UnivFFviaFM() = delete;

    /**
     * Disable copy constructor.
     */
    UnivFFviaFM(UnivFFviaFM &) = delete;

    /**
     * Deconstructor.
     */
    virtual ~UnivFFviaFM();


    /**
     * Computes floor fields for all doors.
     */
    void AddAllTargetsParallel();

    /**
     * Returns the known door UIDs.
     * @return known door UIDs.
     */
    std::vector<int> GetKnownDoorUIDs();

    /**
     * Sets the usage propose.
     * @param userArg wanted user mode.
     */
    void SetUser(int user);

    /**
     * Sets the mode.
     * @param mode wanted mode.
     */
    void SetMode(int mode);

    /**
     * Sets the speed mode.
     * @param speedMode wanted speed mode.
     */
    void SetSpeedMode(int speedMode);

    /**
     * Returns the cost from \p position to \p destID.
     * Using precomputed cost if available, otherwise they will get computed now.
     * @param destID id of destination.
     * @param position position from which the cost should be returned.
     * @return cost from \p position to \p destID with \p mode
     */
    double GetCostToDestination(int destID, const Point & position);

    /**
     * Returns the distance between doors with IDs \p door1ID and \p door2ID.
     * @param door1ID ID of first door.
     * @param door2ID ID of second door.
     * @return distance between doors considering the floor field.
     */
    double GetDistanceBetweenDoors(int door1ID, int door2ID);

    /**
     * Returns the grid used for computing the floor fields.
     * @return grid used for computing the floor fields.
     */
    RectGrid * GetGrid();

    /**
     * Gives the direction to go from \p pos to door with \p destID.
     * @param destID ID of target.
     * @param pos position from which the direction is computed.
     * @param[out] direction direction for next step to go from \p pos to door with \p destID.
     */
    void GetDirectionToUID(int destID, const Point & pos, Point & direction);

    /**
     * Returns the distance to the closest wall of point \p pos.
     * @param pos position from which the wall distance should be returned.
     * @return distance to the closest wall of point \p pos.
     */
    double GetDistance2WallAt(const Point & pos);

    /**
     * Gives the direction to the closest wall of point \p pos.
     * @param pos position from which the wall direction should be returned.
     * @param[out] p direction of closest wall of point \p pos.
     */
    void GetDir2WallAt(const Point & pos, Point & p);

    /**
     * Writes the computed floor fields of all doors to file \p filename.
     * @param filename file to which the floor fields should be written.
     * @param targetID list of doors which should be written.
     */
    void WriteFF(const fs::path & filename, std::vector<int> targetID);

private:
    /**
     * Sets up and computes the floor fields.
     * @param walls all walls and closed door of geometry.
     * @param doors all doors in geometry.
     * @param targetUIDs list of target doors.
     * @param mode mode for floor field.
     * @param spacing grid size.
     * @param wallAvoidDist wall avoidance distance.
     * @param useWallDistances should wall avoidance be considered.
     */
    void Create(
        std::vector<Line> & walls,
        std::map<int, Line> & doors,
        const std::vector<int> & targetUIDs,
        int mode,
        double spacing,
        double wallAvoidDist,
        bool useWallDistances);

    /**
     * Returns the cost from \p position to \p destID with \p mode.
     * Using precomputed cost if available, otherwise they will get computed now.
     * @param destID id of destination.
     * @param position position from which the cost should be returned.
     * @param mode mode used to compute cost.
     * @return cost from \p position to \p destID with \p mode
     */
    double GetCostToDestination(int destID, const Point & position, int mode);

    /**
     * Returns the direction to move from \p position to \p destID with \p mode.
     * @param destID id of destination.
     * @param key
     * @param direction
     * @param mode mode used to compute direction.
     */
    void GetDirectionToUID(int destID, long int key, Point & direction, int mode);

    /**
     * Returns the direction to go from \p position to \p destID.
     * Using precomputed cost if available, otherwise they will get computed now.
     * @param destID id of destination.
     * @param key grid key to a specific position.
     * @param[out] direction direction for next step to go from \p pos to door with \p destID.
     */
    void GetDirectionToUID(int destID, long int key, Point & direction);

    /**
     * Set up grid for computing the floor fields.
     * @param walls walls which should be considered.
     * @param doors doors which should be considered.
     * @param spacing grid size.
     */
    void CreateRectGrid(std::vector<Line> & walls, std::map<int, Line> & doors, double spacing);

    /**
     * Process the geometry. Marks doors and walls on \a _grid.
     * @param walls walls which should be considered.
     * @param doors doors which should be considered.
     */
    void ProcessGeometry(std::vector<Line> & walls, std::map<int, Line> & doors);

    /**
     * Add a target and compute the corresponding floor field.
     * @param uid ID of door.
     * @param costarray array containing the costs.
     * @param gradarray array containing the gradients.
     */
    void AddTarget(int uid, double * costarray = nullptr, Point * gradarray = nullptr);

    /**
     * Add targets and compute the corresponding floor fields.
     * @param wantedDoors doors which should be added.
     */
    void AddTargetsParallel(std::vector<int> wantedDoors);

    /**
     * Mark subroom in grid.
     * @param insidePoint some point inside subroom \p subroom.
     * @param subroom subroom which should be marked.
     */
    void MarkSubroom(const Point & insidePoint, SubRoom * subroom);

    /**
     * Computes the speed reduced close to walls.
     * @param[out] reduWallSpeed altered speed array.
     *
     * @todo check if really needed.
     */
    void CreateReduWallSpeed(double * reduWallSpeed);

    /**
     * Marks a line in grid.
     * @param uid UID of door which should be marked.
     * @param tempTargetLine
     * @param newArrayPt
     * @param passvector
     *
     * @todo check if really needed.
     */
    void FinalizeTargetLine(
        int uid,
        const Line & tempTargetLine,
        Point * newArrayPt,
        Point & passvector);

    /**
     * Draw lines on grid.
     * @param doors doors which should be drawn to grid.
     * @param[out] grid grid on which the lines are drawn.
     */
    void DrawLinesOnGrid(std::map<int, Line> & doors, int * grid);

    /**
     * Draws lines on grid.
     * @tparam T grid data type.
     * @param wallArg lines which should be drawn.
     * @param target grid on which the line is drawn.
     * @param value value which should be used for points on line.
     */
    template <typename T>
    void DrawLinesOnGrid(std::vector<Line> & wallArg, T * target, T value);

    /**
     * Draw a line on grid.
     * @tparam T grid data type.
     * @param line line which should be drawn.
     * @param target grid on which the line is drawn.
     * @param value value which should be used for points on line.
     */
    template <typename T>
    void DrawLinesOnGrid(Line & line, T * target, T value);

    /**
     * Draws lines on wall, e.g. doors.
     * @tparam T grid data type.
     * @param wallArg lines which should be drawn.
     * @param target grid on which the line is drawn.
     * @param value value which should be used for points on line.
     */
    template <typename T>
    void DrawLinesOnWall(std::vector<Line> & wallArg, T * target, T value);

    /**
     * Draw a line on wall, e.g. door.
     * @tparam T grid data type.
     * @param line line which should be drawn.
     * @param target grid on which the line is drawn.
     * @param value value which should be used for points on line.
     */
    template <typename T>
    void DrawLinesOnWall(Line & line, T * target, T value);

    /**
     * Computes the floor field.
     * @param[out] costOutput costs corresponding to floor field.
     * @param[out] directionOutput direction corresponding to floor field.
     * @param speed speed field used for computing floor field.
     */
    void CalcFF(double * costOutput, Point * directionOutput, const double * speed);

    /**
     * Compute cost floor field.
     * @param key key of position in grid.
     * @param[out] cost costs corresponding to floor field.
     * @param[out] dir direction corresponding to floor field.
     * @param speed speed field used for computing floor field.
     */
    void CalcCost(long int key, double * cost, Point * dir, const double * speed);

    /**
     * Compute distance floor field.
     * @param[out] costOutput costs corresponding to floor field.
     * @param[out] directionOutput direction corresponding to floor field.
     * @param speed speed field used for computing floor field.
     */
    void CalcDF(double * costOutput, Point * directionOutput, const double * speed);

    /**
     * Compute distances.
     * @param key key of position in grid.
     * @param[out] cost costs corresponding to floor field.
     * @param[out] dir direction corresponding to floor field.
     * @param speed speed field used for computing floor field.
     */
    void CalcDist(long int key, double * cost, Point * dir, const double * speed);

    /**
     * Computes cell distance for von Neumann neighborhood.
     * @param xy
     * @param hDivF
     * @return \p xy + \p hDivF
     */
    inline double OnesidedCalc(double xy, double hDivF);

    /**
     * Computes cell distance for moore neighborhood.
     * @param x
     * @param y
     * @param hDivF
     * @return
     */
    inline double TwosidedCalc(double x, double y, double hDivF);

private:
    /**
     * Geometry used for simulation.
     */
    Building * _building = nullptr;

    /**
     * Configuration used for simulation.
     */
    Configuration * _configuration = nullptr;

    /**
     * ID of room for which this floor field is computed.
     */
    int _room = -1;

    /**
     * Mode for computing the floor field.
     */
    int _mode = LINESEGMENT; //default

    /**
     * Usage for floor field.
     */
    int _user = DISTANCE_AND_DIRECTIONS_USED; //default

    /**
     * Speedmode for computing the floor field.
     */
    int _speedmode = FF_HOMO_SPEED; //default

    /**
     * Grid used for computing the floor field.
     */
    RectGrid * _grid = nullptr;

    /**
     * Number of cells in \a _grid.
     */
    long int _nPoints = 0;

    /**
     * Vector containing the different speed fields.
     * [0] homogeneous speed in walkable area, nealy zero in walls
     * [1] reduced wall speed
     * [2] standing agents reduce speed, so that jams will be considered in ff
     */
    std::vector<double *> _speedFieldSelector;

    /**
     * Grid marking the geometrical properties of the room.
     */
    int * _gridCode = nullptr;

    /**
     * List of subrooms in room.
     * Remark: this is an array (first asterisk) of pointers (second asterisk)
     */
    SubRoom ** _subrooms = nullptr;

    /**
     * Wall avoidance distance.
     */
    double _wallAvoidDistance = 0.;

    /**
     * State if wall avoidance should be used.
     * Could be used in DirectionStrategy even if mode _speedmode is FF_HOMO_SPEED.
     */
    bool _useWallDistances = false;

    //the following maps are responsible for dealloc the arrays
    /**
     * Map containing the cost field for the corresponding door.
     */
    std::map<int, double *> _costFieldWithKey;

    /**
     * Map containing the direction fields for the corresponding door.
     */
    std::map<int, Point *> _directionFieldWithKey;

    /**
     * List of door UIDs.
     */
    std::vector<int> _uids;

    /**
     * Map containing the door and the corresponding UID.
     */
    std::map<int, Line> _doors;

    /**
     * Map containing a inside point to each subroom.
     */
    std::map<SubRoom *, Point> _subRoomPtrTOinsidePoint;
};