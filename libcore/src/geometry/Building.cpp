/**
 * \file        Building.cpp
 * \date        Oct 1, 2014
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
#include "Building.h"

#include "IO/GeoFileParser.h"
#include "IO/TrainFileParser.h"
#include "general/Configuration.h"
#include "general/Filesystem.h"
#include "general/Macros.h"
#include "geometry/Building.h"
#include "geometry/Crossing.h"
#include "geometry/Goal.h"
#include "geometry/Hline.h"
#include "geometry/Line.h"
#include "geometry/Obstacle.h"
#include "geometry/Point.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"
#include "geometry/helper/CorrectGeometry.h"
#include "neighborhood/NeighborhoodSearch.h"
#include "pedestrian/Pedestrian.h"
#include "routing/RoutingEngine.h"

#include <Logger.h>
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <cfloat>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Building::Building(std::vector<std::unique_ptr<Pedestrian>> * agents) : _allPedestrians(agents) {}

Building::Building(
    Configuration * configuration,
    std::vector<std::unique_ptr<Pedestrian>> * agents) :
    _configuration(configuration), _allPedestrians(agents)
{
    {
        std::unique_ptr<GeoFileParser> parser(new GeoFileParser(_configuration));
        parser->LoadBuilding(this);
    }

    if(!InitGeometry()) {
        LOG_ERROR("Could not initialize the geometry!");
        exit(EXIT_FAILURE);
    }

    InitGrid();

    if(!SanityCheck()) {
        LOG_ERROR("There are sanity errors in the geometry file");
        exit(EXIT_FAILURE);
    }
}

Building::~Building()
{
    for(std::map<int, Crossing *>::const_iterator iter = _crossings.begin();
        iter != _crossings.end();
        ++iter) {
        // Don't delete crossings from goals
        bool goalCrossing = false;
        for(auto [_, goal] : _goals) {
            if(goal->GetCentreCrossing()->GetUniqueID() == iter->second->GetUniqueID()) {
                goalCrossing = true;
            }
        }
        if(!goalCrossing) {
            delete iter->second;
        }
    }
    for(std::map<int, Transition *>::const_iterator iter = _transitions.begin();
        iter != _transitions.end();
        ++iter) {
        delete iter->second;
    }
    for(std::map<int, Hline *>::const_iterator iter = _hLines.begin(); iter != _hLines.end();
        ++iter) {
        delete iter->second;
    }
    for(std::map<int, Goal *>::const_iterator iter = _goals.begin(); iter != _goals.end(); ++iter) {
        delete iter->second;
    }
}

Configuration * Building::GetConfig() const
{
    return _configuration;
}

int Building::GetNumberOfRooms() const
{
    return (int) _rooms.size();
}

int Building::GetNumberOfGoals() const
{
    return (int) (_transitions.size() + _hLines.size() + _crossings.size());
}

const std::map<int, std::shared_ptr<Room>> & Building::GetAllRooms() const
{
    return _rooms;
}

Room * Building::GetRoom(int index) const
{
    //TODO: obsolete since the check is done by .at()
    if(_rooms.count(index) == 0) {
        LOG_ERROR(
            "Wrong 'index' in CBuiling::GetRoom() Room ID: {} size: {}", index, _rooms.size());
        LOG_INFO("Control your rooms ID and make sure they are in the order 0, 1, 2,.. ");
        return nullptr;
    }
    return _rooms.at(index).get();
}

std::tuple<Room *, SubRoom *> Building::GetRoomAndSubRoom(const Point position) const
{
    for(auto const & [_, room] : _rooms) {
        auto it = std::find_if(
            room->GetAllSubRooms().begin(),
            room->GetAllSubRooms().end(),
            [position](const auto & val) { return val.second->IsInSubRoom(position); });
        if(it != room->GetAllSubRooms().end()) {
            return {room.get(), it->second.get()};
        }
    }
    throw std::runtime_error(fmt::format(
        FMT_STRING("Position {} could not be found in any subroom."), position.toString()));
}

Room * Building::GetRoom(const Point position) const
{
    return std::get<0>(GetRoomAndSubRoom(position));
}

SubRoom * Building::GetSubRoom(const Point position) const
{
    return std::get<1>(GetRoomAndSubRoom(position));
}
std::tuple<int, int, int> Building::GetRoomAndSubRoomIDs(const Point position) const
{
    auto [room, subroom] = GetRoomAndSubRoom(position);
    return {room->GetID(), subroom->GetSubRoomID(), subroom->GetUID()};
}


bool Building::IsInAnySubRoom(const Point pos) const
{
    for(auto const & [_, room] : _rooms) {
        auto it = std::find_if(
            room->GetAllSubRooms().begin(), room->GetAllSubRooms().end(), [pos](const auto & val) {
                return val.second->IsInSubRoom(pos);
            });
        if(it != room->GetAllSubRooms().end()) {
            return true;
        }
    }
    return false;
}

const NeighborhoodSearch & Building::GetNeighborhoodSearch() const
{
    return _neighborhoodSearch;
}

void Building::AddRoom(Room * room)
{
    _rooms[room->GetID()] = std::shared_ptr<Room>(room);
}

void Building::AddSurroundingRoom()
{
    LOG_INFO("Adding the room 'outside' ");
    // first look for the geometry boundaries
    double x_min = FLT_MAX;
    double x_max = -FLT_MAX;
    double y_min = FLT_MAX;
    double y_max = -FLT_MAX;
    //finding the bounding of the grid
    // and collect the pedestrians

    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            for(auto && wall : itr_subroom.second->GetAllWalls()) {
                double x1 = wall.GetPoint1()._x;
                double y1 = wall.GetPoint1()._y;
                double x2 = wall.GetPoint2()._x;
                double y2 = wall.GetPoint2()._y;

                double xmax = (x1 > x2) ? x1 : x2;
                double xmin = (x1 > x2) ? x2 : x1;
                double ymax = (y1 > y2) ? y1 : y2;
                double ymin = (y1 > y2) ? y2 : y1;

                x_min = (xmin <= x_min) ? xmin : x_min;
                x_max = (xmax >= x_max) ? xmax : x_max;
                y_max = (ymax >= y_max) ? ymax : y_max;
                y_min = (ymin <= y_min) ? ymin : y_min;
            }
        }
    }

    for(auto && itr_goal : _goals) {
        for(auto && wall : itr_goal.second->GetAllWalls()) {
            double x1 = wall.GetPoint1()._x;
            double y1 = wall.GetPoint1()._y;
            double x2 = wall.GetPoint2()._x;
            double y2 = wall.GetPoint2()._y;

            double xmax = (x1 > x2) ? x1 : x2;
            double xmin = (x1 > x2) ? x2 : x1;
            double ymax = (y1 > y2) ? y1 : y2;
            double ymin = (y1 > y2) ? y2 : y1;

            x_min = (xmin <= x_min) ? xmin : x_min;
            x_max = (xmax >= x_max) ? xmax : x_max;
            y_max = (ymax >= y_max) ? ymax : y_max;
            y_min = (ymin <= y_min) ? ymin : y_min;
        }
    }
    //make the grid slightly larger.
    x_min = x_min - 10.0;
    x_max = x_max + 10.0;
    y_min = y_min - 10.0;
    y_max = y_max + 10.0;

    SubRoom * bigSubroom = new NormalSubRoom();
    bigSubroom->SetRoomID((int) _rooms.size());
    bigSubroom->SetSubRoomID(0); // should be the single subroom
    bigSubroom->AddWall(Wall(Point(x_min, y_min), Point(x_min, y_max)));
    bigSubroom->AddWall(Wall(Point(x_min, y_max), Point(x_max, y_max)));
    bigSubroom->AddWall(Wall(Point(x_max, y_max), Point(x_max, y_min)));
    bigSubroom->AddWall(Wall(Point(x_max, y_min), Point(x_min, y_min)));

    Room * bigRoom = new Room();
    bigRoom->AddSubRoom(bigSubroom);
    bigRoom->SetCaption("outside");
    bigRoom->SetID((int) _rooms.size());
    AddRoom(bigRoom);
}

bool Building::InitGeometry()
{
    Logging::Info("Init Geometry");
    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            //create a close polyline out of everything
            std::vector<Line *> goals = std::vector<Line *>();

            //  collect all crossings
            for(auto && cros : itr_subroom.second->GetAllCrossings()) {
                goals.push_back(cros);
            }
            //collect all transitions
            for(auto && trans : itr_subroom.second->GetAllTransitions()) {
                goals.push_back(trans);
            }
            // initialize the poly
            if(!itr_subroom.second->ConvertLineToPoly(goals))
                return false;
            itr_subroom.second->CalculateArea();

            //do the same for the obstacles that are closed
            for(auto && obst : itr_subroom.second->GetAllObstacles()) {
                if(!obst->ConvertLineToPoly())
                    return false;
            }

            //here we can create a boost::geometry::model::polygon out of the vector<Point> objects created above
            itr_subroom.second->CreateBoostPoly();

            double minElevation = FLT_MAX;
            double maxElevation = -FLT_MAX;
            for(auto && wall : itr_subroom.second->GetAllWalls()) {
                const Point & P1 = wall.GetPoint1();
                const Point & P2 = wall.GetPoint2();
                if(minElevation > itr_subroom.second->GetElevation(P1)) {
                    minElevation = itr_subroom.second->GetElevation(P1);
                }

                if(maxElevation < itr_subroom.second->GetElevation(P1)) {
                    maxElevation = itr_subroom.second->GetElevation(P1);
                }

                if(minElevation > itr_subroom.second->GetElevation(P2)) {
                    minElevation = itr_subroom.second->GetElevation(P2);
                }

                if(maxElevation < itr_subroom.second->GetElevation(P2)) {
                    maxElevation = itr_subroom.second->GetElevation(P2);
                }
            }
            itr_subroom.second->SetMaxElevation(maxElevation);
            itr_subroom.second->SetMinElevation(minElevation);
        }
    }


    // look and save the neighbor subroom for improving the runtime
    // that information is already present in the crossing/transitions

    for(const auto & cross : _crossings) {
        SubRoom * s1 = cross.second->GetSubRoom1();
        SubRoom * s2 = cross.second->GetSubRoom2();
        if(s1)
            s1->AddNeighbor(s2);
        if(s2)
            s2->AddNeighbor(s1);
    }

    for(const auto & trans : _transitions) {
        SubRoom * s1 = trans.second->GetSubRoom1();
        SubRoom * s2 = trans.second->GetSubRoom2();
        if(s1)
            s1->AddNeighbor(s2);
        if(s2)
            s2->AddNeighbor(s1);
    }

    InitInsideGoals();
    InitPlatforms();
    LOG_INFO("Init Geometry successful!!!");

    return true;
}

void Building::InitPlatforms()
{
    for(auto & [trackID, track] : _tracks) {
        LOG_INFO("track {}:", trackID);
        geometry::helper::SortWalls(track._walls, _trackStarts.at(trackID));
        for(const auto & wall : track._walls) {
            LOG_INFO("wall: {}", wall.toString());
        }
        LOG_INFO("track start: {}", _trackStarts.at(trackID).toString());
    }
}

bool Building::InitInsideGoals()
{
    bool found = false;
    for(auto & goalItr : _goals) {
        Goal * goal = goalItr.second;
        if(goal->GetRoomID() == -1) {
            found = true;
            continue;
        }

        for(auto & roomItr : _rooms) {
            Room * room = roomItr.second.get();

            if(goal->GetRoomID() != room->GetID()) {
                continue;
            }

            for(auto & subRoomItr : room->GetAllSubRooms()) {
                SubRoom * subRoom = subRoomItr.second.get();

                if((goal->GetSubRoomID() == subRoom->GetSubRoomID()) &&
                   (subRoom->IsInSubRoom(goal->GetCentroid()))) {
                    Crossing * crossing = goal->GetCentreCrossing();
                    subRoom->AddCrossing(crossing);
                    crossing->SetRoom1(room);
                    crossing->SetSubRoom1(subRoom);
                    crossing->SetSubRoom2(subRoom);
                    AddCrossing(crossing);
                    found = true;
                    break;
                }
            }
        }

        if(!found) {
            LOG_WARNING(
                "Goal {} seems to have no subroom and is not outside, please check your input",
                goal->GetId());
        }
        found = false;
    }

    LOG_INFO("InitInsideGoals successful!!!");

    return true;
}

const fs::path & Building::GetProjectFilename() const
{
    return _configuration->iniFile;
}

const fs::path & Building::GetProjectRootDir() const
{
    return _configuration->projectRootDir;
}

bool Building::AddCrossing(Crossing * line)
{
    int IDRoom     = line->GetRoom1()->GetID();
    int IDLine     = line->GetUniqueID();
    int IDCrossing = 1000 * IDRoom + IDLine;
    if(_crossings.count(IDCrossing) != 0) {
        LOG_ERROR("Duplicate index for crossing found [{}] in Routing::AddCrossing()", IDCrossing);
        exit(EXIT_FAILURE);
    }
    _crossings[IDCrossing] = line;
    return true;
}

bool Building::RemoveTransition(Transition * line)
{
    if(_transitions.count(line->GetID()) != 0) {
        _transitions.erase(line->GetID());
        return true;
    }
    return false;
}

bool Building::AddTransition(Transition * line)
{
    if(_transitions.count(line->GetID()) != 0) {
        LOG_ERROR(
            "Duplicate index for transition found [{}] in Routing::AddTransition()", line->GetID());
        exit(EXIT_FAILURE);
    }
    _transitions[line->GetID()] = line;

    return true;
}

bool Building::AddHline(Hline * line)
{
    if(_hLines.count(line->GetID()) != 0) {
        // check if the lines are identical
        Hline * ori = _hLines[line->GetID()];
        if(ori->operator==(*line)) {
            LOG_INFO("Skipping identical hlines with ID [{}]", line->GetID());
            return false;
        } else {
            LOG_ERROR(
                "Duplicate index for hlines found [{}] in Routing::AddHline(). You have "
                "[{}] hlines",
                line->GetID(),
                _hLines.size());
            exit(EXIT_FAILURE);
        }
    }
    _hLines[line->GetID()] = line;
    return true;
}

bool Building::AddGoal(Goal * goal)
{
    if(_goals.count(goal->GetId()) != 0) {
        LOG_ERROR("Duplicate index for goal found [{}] in Routing::AddGoal()", goal->GetId());
        exit(EXIT_FAILURE);
    }
    _goals[goal->GetId()] = goal;


    return true;
}

const std::map<int, Crossing *> & Building::GetAllCrossings() const
{
    return _crossings;
}

const std::map<int, Transition *> & Building::GetAllTransitions() const
{
    return _transitions;
}

const std::map<int, Hline *> & Building::GetAllHlines() const
{
    return _hLines;
}


const std::map<int, Goal *> & Building::GetAllGoals() const
{
    return _goals;
}

Transition * Building::GetTransition(int ID) const //ar.graf: added const 2015-12-10
{
    if(_transitions.count(ID) == 1) {
        return _transitions.at(ID);
    } else {
        if(ID == -1)
            return nullptr;
        else {
            LOG_ERROR(
                "I could not find any transition with the 'ID' [{}]. You have defined [{}] "
                "transitions",
                ID,
                _transitions.size());
            exit(EXIT_FAILURE);
        }
    }
}

Goal * Building::GetFinalGoal(int ID) const
{
    if(_goals.count(ID) == 1) {
        return _goals.at(ID);
    } else {
        if(ID == -1)
            return nullptr;
        else {
            LOG_ERROR(
                "I could not find any goal with the 'ID' [{}]. You have defined [{}] goals",
                ID,
                _goals.size());
            exit(EXIT_FAILURE);
        }
    }
}

Hline * Building::GetTransOrCrossByUID(int id) const
{
    {
        //eventually transitions
        std::map<int, Transition *>::const_iterator itr;
        for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
            if(itr->second->GetUniqueID() == id)
                return itr->second;
        }
    }
    {
        //then the  crossings
        std::map<int, Crossing *>::const_iterator itr;
        for(itr = _crossings.begin(); itr != _crossings.end(); ++itr) {
            if(itr->second->GetUniqueID() == id)
                return itr->second;
        }
    }
    {
        //finally the  hlines
        for(auto itr = _hLines.begin(); itr != _hLines.end(); ++itr) {
            if(itr->second->GetUniqueID() == id)
                return itr->second;
        }
    }
    LOG_ERROR("No Transition, Crossing or hline with ID {} found.", id);
    return nullptr;
}

SubRoom * Building::GetSubRoomByUID(int uid) const
{
    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            if(itr_subroom.second->GetUID() == uid)
                return itr_subroom.second.get();
        }
    }
    LOG_ERROR("No subroom exits with the unique id {}", uid);
    return nullptr;
}

bool Building::IsVisible(
    const Point & p1,
    const Point & p2,
    const std::vector<SubRoom *> & subrooms,
    bool considerHlines)
{
    //loop over all subrooms if none is provided
    if(subrooms.empty()) {
        for(auto && itr_room : _rooms) {
            for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
                if(!itr_subroom.second->IsVisible(p1, p2, considerHlines))
                    return false;
            }
        }
    } else {
        for(auto && sub : subrooms) {
            if(sub && !sub->IsVisible(p1, p2, considerHlines))
                return false;
        }
    }

    return true;
}

bool Building::Triangulate()
{
    LOG_INFO("Triangulating the geometry.");
    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            if(!itr_subroom.second->Triangulate())
                return false;
        }
    }
    LOG_INFO("Done.");
    return true;
}

std::vector<Point> Building::GetBoundaryVertices() const
{
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double xMax = -FLT_MAX;
    double yMax = -FLT_MAX;
    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            const std::vector<Point> vertices = itr_subroom.second->GetPolygon();

            for(Point point : vertices) {
                if(point._x > xMax)
                    xMax = point._x;
                else if(point._x < xMin)
                    xMin = point._x;
                if(point._y > yMax)
                    yMax = point._y;
                else if(point._y < yMin)
                    yMin = point._y;
            }
        }
    }
    return std::vector<Point>{
        Point(xMin, yMin), Point(xMin, yMax), Point(xMax, yMax), Point(xMax, yMin)};
}

bool Building::SanityCheck()
{
    LOG_INFO("Checking the geometry for artifacts: (Ignore Warnings, if ff_[...] router is used!)");
    bool status = true;

    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            if(!itr_subroom.second->SanityCheck())
                status = false;
        }
    }

    LOG_INFO("...Done.");
    return status;
}

void Building::UpdateGrid()
{
    _neighborhoodSearch.Update(*_allPedestrians);
}

void Building::InitGrid()
{
    // first look for the geometry boundaries
    double x_min = FLT_MAX;
    double x_max = FLT_MIN;
    double y_min = FLT_MAX;
    double y_max = FLT_MIN;

    //finding the bounding of the grid
    // and collect the pedestrians
    for(auto && itr_room : _rooms) {
        for(auto && itr_subroom : itr_room.second->GetAllSubRooms()) {
            for(auto && wall : itr_subroom.second->GetAllWalls()) {
                double x1 = wall.GetPoint1()._x;
                double y1 = wall.GetPoint1()._y;
                double x2 = wall.GetPoint2()._x;
                double y2 = wall.GetPoint2()._y;

                double xmax = (x1 > x2) ? x1 : x2;
                double xmin = (x1 > x2) ? x2 : x1;
                double ymax = (y1 > y2) ? y1 : y2;
                double ymin = (y1 > y2) ? y2 : y1;

                x_min = (xmin <= x_min) ? xmin : x_min;
                x_max = (xmax >= x_max) ? xmax : x_max;
                y_max = (ymax >= y_max) ? ymax : y_max;
                y_min = (ymin <= y_min) ? ymin : y_min;
            }
        }
    }

    double cellSize = _configuration->linkedCellSize;
    //make the grid slightly larger.
    x_min = x_min - 1 * cellSize;
    x_max = x_max + 1 * cellSize;
    y_min = y_min - 1 * cellSize;
    y_max = y_max + 1 * cellSize;


    //no algorithms
    // the domain is made of a single cell
    if(cellSize == -1) {
        LOG_INFO("Brute Force will be used for neighborhoods query");
        if((x_max - x_min) < (y_max - y_min)) {
            cellSize = (y_max - y_min);
        } else {
            cellSize = (x_max - x_min);
        }

    } else {
        LOG_INFO("Initializing the grid with cell size: {}.", cellSize);
    }

    _neighborhoodSearch = NeighborhoodSearch(x_min, x_max, y_min, y_max, cellSize);

    LOG_INFO("Done with Initializing the grid");
}

void Building::GetPedestrians(int room, int subroom, std::vector<Pedestrian *> & peds) const
{
    for(auto && ped : *_allPedestrians) {
        auto [ped_roomid, ped_subroomid, _] = GetRoomAndSubRoomIDs(ped->GetPos());
        if((room == ped_roomid) && (subroom == ped_subroomid)) {
            peds.push_back(ped.get());
        }
    }
}

Transition * Building::GetTransitionByUID(int uid) const
{
    for(auto && trans : _transitions) {
        if(trans.second->GetUniqueID() == uid)
            return trans.second;
    }
    return nullptr;
}

Crossing * Building::GetCrossingByUID(int uid) const
{
    for(auto && cross : _crossings) {
        if(cross.second->GetUniqueID() == uid)
            return cross.second;
    }
    return nullptr;
}

void Building::AddTrain(int trainID, TrainType type)
{
    _trains.emplace(trainID, type);
}

TrainType Building::GetTrain(int trainID)
{
    return _trains.at(trainID);
}

std::map<int, TrainType> Building::GetTrains() const
{
    return _trains;
}

std::vector<TrainType> Building::GetTrainTypes()
{
    std::vector<TrainType> trainTypes;
    trainTypes.reserve(_trains.size());

    for(auto const & [trainID, trainType] : _trains) {
        trainTypes.emplace_back(trainType);
    }

    std::unique(
        std::begin(trainTypes), std::end(trainTypes), [](const TrainType & a, const TrainType & b) {
            return a._type == b._type;
        });

    return trainTypes;
}

void Building::AddTrainWallAdded(int trainID, Wall trainAddedWall)
{
    auto iter = _trainWallsAdded.find(trainID);

    if(iter != _trainWallsAdded.end()) {
        iter->second.emplace_back(trainAddedWall);
    } else {
        _trainWallsAdded.emplace(trainID, std::vector<Wall>{trainAddedWall});
    }
}

void Building::ClearTrainWallsAdded(int trainID)
{
    _trainWallsAdded.erase(trainID);
}

std::optional<std::vector<Wall>> Building::GetTrainWallsAdded(int trainID)
{
    auto iter = _trainWallsAdded.find(trainID);

    if(iter != _trainWallsAdded.end()) {
        return iter->second;
    }

    return std::nullopt;
}

void Building::AddTrainWallRemoved(int trainID, Wall trainRemovedWall)
{
    auto iter = _trainWallsRemoved.find(trainID);

    if(iter != _trainWallsRemoved.end()) {
        iter->second.emplace_back(trainRemovedWall);
    } else {
        _trainWallsRemoved.emplace(trainID, std::vector<Wall>{trainRemovedWall});
    }
}

void Building::ClearTrainWallsRemoved(int trainID)
{
    _trainWallsRemoved.erase(trainID);
}

std::optional<std::vector<Wall>> Building::GetTrainWallsRemoved(int trainID)
{
    auto iter = _trainWallsRemoved.find(trainID);

    if(iter != _trainWallsRemoved.end()) {
        return iter->second;
    }

    return std::nullopt;
}

void Building::AddTrainDoorAdded(int trainID, Transition trainAddedDoor)
{
    auto iter = _trainDoorsAdded.find(trainID);

    if(iter != _trainDoorsAdded.end()) {
        iter->second.emplace_back(trainAddedDoor);
    } else {
        _trainDoorsAdded.emplace(trainID, std::vector<Transition>{trainAddedDoor});
    }
}

void Building::ClearTrainDoorsAdded(int trainID)
{
    _trainDoorsAdded.erase(trainID);
}

std::optional<std::vector<Transition>> Building::GetTrainDoorsAdded(int trainID)
{
    auto iter = _trainDoorsAdded.find(trainID);

    if(iter != _trainDoorsAdded.end()) {
        return iter->second;
    }

    return std::nullopt;
}

void Building::AddTrackWall(int trackID, int roomID, int subRoomID, Wall trackWall)
{
    auto iter = _tracks.find(trackID);

    if(iter != _tracks.end()) {
        iter->second._walls.emplace_back(trackWall);
    } else {
        Track track{trackID, roomID, subRoomID, std::vector<Wall>{trackWall}};
        _tracks.emplace(trackID, track);
    }
}

std::optional<Track> Building::GetTrack(int trackID) const
{
    auto iter = _tracks.find(trackID);

    if(iter != _tracks.end()) {
        return iter->second;
    }

    return std::nullopt;
}

void Building::AddTrackStart(int trackID, Point trackStart)
{
    if(_trackStarts.find(trackID) != _trackStarts.end()) {
        LOG_WARNING("Track start already exist, will be overwritten.");
    }

    _trackStarts[trackID] = trackStart;
}

std::optional<Point> Building::GetTrackStart(int trackID) const
{
    auto iter = _trackStarts.find(trackID);

    if(iter != _trackStarts.end()) {
        return iter->second;
    }

    return std::nullopt;
}
