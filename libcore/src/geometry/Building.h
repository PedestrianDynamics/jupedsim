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

#include "Goal.h"
#include "Hline.h"
#include "Line.h"
#include "Obstacle.h"
#include "Room.h"
#include "TrainGeometryInterface.h"
#include "Transition.h"
#include "Wall.h"
#include "general/Configuration.h"
#include "general/Filesystem.h"
#include "neighborhood/NeighborhoodSearch.h"

#include <optional>

using PointWall = std::pair<Point, Wall>;

class RoutingEngine;

class Pedestrian;

class Transition;

class PedDistributor;

class WaitingArea;

class Building
{
private:
    Configuration * _configuration = nullptr;
    NeighborhoodSearch _neighborhoodSearch;
    std::vector<std::unique_ptr<Pedestrian>> * _allPedestrians;
    std::map<int, std::shared_ptr<Room>> _rooms;
    std::map<int, Crossing *> _crossings;
    std::map<int, Transition *> _transitions;
    std::map<int, Hline *> _hLines;
    std::map<int, Goal *> _goals;
    std::map<int, TrainType> _trains;

    std::map<int, Track> _tracks;
    std::map<int, Point> _trackStarts;

    /**
     * Map of walls added temporarily for a specific train
     */
    std::map<int, std::vector<Wall>> _trainWallsAdded;

    /**
     * Map of walls removed temporarily for a specific train
     */
    std::map<int, std::vector<Wall>> _trainWallsRemoved;

    /**
     * Map of doors added temporarily for a specific train
     */
    std::map<int, std::vector<Transition>> _trainDoorsAdded;

public:
    /// constructor
    Building(std::vector<std::unique_ptr<Pedestrian>> * agents);

    Building(Configuration * config, std::vector<std::unique_ptr<Pedestrian>> * agents);

    /// destructor
    virtual ~Building();

    void SetAgents(std::vector<std::unique_ptr<Pedestrian>> * agents) { _allPedestrians = agents; }

    Configuration * GetConfig() const;

    void GetPedestrians(int room, int subroom, std::vector<Pedestrian *> & peds) const;

    RoutingEngine * GetRoutingEngine() const;
    const std::map<int, std::shared_ptr<Room>> & GetAllRooms() const;

    int GetNumberOfRooms() const;

    int GetNumberOfGoals() const;

    Room * GetRoom(int index) const;

    std::tuple<Room *, SubRoom *> GetRoomAndSubRoom(const Point position) const;

    std::tuple<int, int, int> GetRoomAndSubRoomIDs(const Point position) const;

    bool IsInAnySubRoom(const Point pos) const;

    Room * GetRoom(const Point position) const;
    SubRoom * GetSubRoom(const Point position) const;

    Transition * GetTransition(int id) const;

    /**
      * return the subroom with the corresponding unique identifier
      * @param uid ,the unique identifier
      * @return NULL if no exists with that identifier.
      */
    SubRoom * GetSubRoomByUID(int uid) const;

    /**
      * @return true if the two points are visible from each other.
      * Alls walls and transitions and crossings are used in this check.
      * The use of hlines is optional, because they are not real, can be considered transparent
      */
    bool IsVisible(
        const Point & p1,
        const Point & p2,
        const std::vector<SubRoom *> & subrooms,
        bool considerHlines = false);

    /**
      * @return a crossing or a transition or a hline matching the given uid.
      * Return NULL if none is found
      */
    Hline * GetTransOrCrossByUID(int uid) const;

    /**
      * @return the transition matching the uid
      */
    Transition * GetTransitionByUID(int uid) const;

    Crossing * GetCrossingByUID(int uid) const;

    //TOD0: rename later to GetGoal
    Goal * GetFinalGoal(int id) const;

    const NeighborhoodSearch & GetNeighborhoodSearch() const;

    // convenience methods
    bool InitGeometry();


    void InitGrid();

    void AddRoom(Room * room);

    void UpdateGrid();

    void
    AddSurroundingRoom(); // add a final room (outside or world), that encompasses the complete geometry

    const std::map<int, Crossing *> & GetAllCrossings() const;

    const std::map<int, Transition *> & GetAllTransitions() const;

    const std::map<int, Hline *> & GetAllHlines() const;

    const std::map<int, Goal *> & GetAllGoals() const;

    // --------------- Trains interface
    const std::map<int, Track> & GetTracks() const;

    const std::vector<Wall>
    GetTrackWalls(Point TrackStart, Point TrackEnd, int & room_id, int & subroom_id) const;

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
    bool AddCrossing(Crossing * line);

    bool RemoveTransition(Transition * line);

    bool AddTransition(Transition * line);

    bool AddHline(Hline * line);

    bool AddGoal(Goal * goal);

    const fs::path & GetProjectRootDir() const;

    const fs::path & GetProjectFilename() const;

    /**
      * Check the scenario for possible errors and
      * output user specific informations.
      */
    bool SanityCheck();

    /**
      * Triangulate the geometry
      */
    bool Triangulate();

    /**
      * @return Vector with the vertices of the geometry's outer boundary rect
      */
    std::vector<Point> GetBoundaryVertices() const;

    /**
     * Adds a train to the building
     * @param trainID ID of the added train
     * @param type type of the train
     */
    void AddTrain(int trainID, TrainType type);

    /**
     * Get the type of train with ID \p trainID
     * @param trainID ID of the train
     * @return Type of train with ID \p trainID
     */
    TrainType GetTrain(int trainID);

    /**
     * Get the train types as map
     * @return train types of the building with trainID as key
     */
    std::map<int, TrainType> GetTrains() const;

    /**
     * Get the train types as vector, each type only once
     * @return vector of train types in \a _trains
     */
    std::vector<TrainType> GetTrainTypes();

    void AddTrackWall(int trackID, int roomID, int subRoomID, Wall trackWall);

    std::optional<Track> GetTrack(int track) const;

    void AddTrackStart(int trackID, Point trackStart);

    std::optional<Point> GetTrackStart(int trackID) const;

private:
    bool InitInsideGoals();
    void InitPlatforms();
};
