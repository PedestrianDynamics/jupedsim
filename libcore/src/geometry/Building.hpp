/**
 * \file        Building.h
 * \date        Oct 1, 2010
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
#pragma once

#include "Goal.hpp"
#include "Hline.hpp"
#include "Line.hpp"
#include "NavLineParameters.hpp"
#include "Obstacle.hpp"
#include "Room.hpp"
#include "TrainGeometryInterface.hpp"
#include "Transition.hpp"
#include "Wall.hpp"
#include "general/Filesystem.hpp"
#include "neighborhood/NeighborhoodSearch.hpp"

#include <optional>

using PointWall = std::pair<Point, Wall>;

class Pedestrian;
class Transition;
class PedDistributor;
class WaitingArea;
struct Configuration;

class Building
{
private:
    Configuration* _configuration = nullptr;
    std::map<int, std::shared_ptr<Room>> _rooms;
    std::map<int, Crossing*> _crossings;
    std::map<int, Transition*> _transitions;
    std::map<int, Hline*> _hLines;
    std::map<int, Goal*> _goals;
    std::map<int, TrainType> _trains;
    std::map<int, Track> _tracks;
    std::map<int, Point> _trackStarts;
    /// Map of walls added temporarily for a specific train
    std::map<int, std::vector<Wall>> _trainWallsAdded;
    /// Map of walls removed temporarily for a specific train
    std::map<int, std::vector<Wall>> _trainWallsRemoved;
    /// Map of doors added temporarily for a specific train
    std::map<int, std::vector<Transition>> _trainDoorsAdded;

public:
    explicit Building(Configuration* config);

    /// destructor
    ~Building();

    const std::map<int, std::shared_ptr<Room>>& GetAllRooms() const;

    Room* GetRoom(int index) const;

    std::tuple<Room*, SubRoom*> GetRoomAndSubRoom(const Point position) const;

    std::tuple<int, int, int> GetRoomAndSubRoomIDs(const Point position) const;

    bool IsInAnySubRoom(const Point pos) const;

    SubRoom* GetSubRoom(const Point position) const;

    Transition* GetTransition(int id) const;

    /**
     * @return the transition matching the uid
     */
    Transition* GetTransitionByUID(int uid) const;

    // convenience methods
    bool InitGeometry();

    void InitGrid();

    void AddRoom(Room* room);

    const std::map<int, Crossing*>& GetAllCrossings() const;

    const std::map<int, Transition*>& GetAllTransitions() const;

    const std::map<int, Goal*>& GetAllGoals() const;

    void AddTrainWallAdded(int trainID, Wall trainAddedWall);
    void ClearTrainWallsAdded(int trainID);
    std::optional<std::vector<Wall>> GetTrainWallsAdded(int trainID);

    void AddTrainWallRemoved(int trainID, Wall trainRemovedWall);
    void ClearTrainWallsRemoved(int trainID);
    std::optional<std::vector<Wall>> GetTrainWallsRemoved(int trainID);

    void AddTrainDoorAdded(int trainID, Transition trainAddedDoor);
    void ClearTrainDoorsAdded(int trainID);
    std::optional<std::vector<Transition>> GetTrainDoorsAdded(int trainID);

    // ------------------------------------
    bool AddCrossing(Crossing* line);

    bool RemoveTransition(const Transition* line);

    bool AddTransition(Transition* line);

    bool AddGoal(Goal* goal);

    /**
     * Check the scenario for possible errors and
     * output user specific informations.
     */
    bool SanityCheck();

    /**
     * Adds a train to the building
     * @param trainID ID of the added train
     * @param type type of the train
     */
    void AddTrainType(int trainID, TrainType type);

    /**
     * Get the train types as map
     * @return train types of the building with trainID as key
     */
    std::map<int, TrainType> GetTrains() const;

    void AddTrackWall(int trackID, int roomID, int subRoomID, Wall trackWall);

    std::optional<Track> GetTrack(int track) const;

    void AddTrackStart(int trackID, Point trackStart);

    std::optional<Point> GetTrackStart(int trackID) const;

private:
    bool InitInsideGoals();
    void InitPlatforms();
};
