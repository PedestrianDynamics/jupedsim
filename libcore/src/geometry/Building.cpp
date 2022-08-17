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
#include "Building.hpp"

#include "IO/GeoFileParser.hpp"
#include "Line.hpp"
#include "Macros.hpp"
#include "NeighborhoodSearch.hpp"
#include "Pedestrian.hpp"
#include "Point.hpp"
#include "general/Configuration.hpp"
#include "geometry/Building.hpp"
#include "geometry/Crossing.hpp"
#include "geometry/Goal.hpp"
#include "geometry/Hline.hpp"
#include "geometry/Obstacle.hpp"
#include "geometry/Room.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/Transition.hpp"
#include "geometry/Wall.hpp"

#include <Logger.hpp>
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

Building::Building(Configuration* configuration) : _configuration(configuration)
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
    for(std::map<int, Crossing*>::const_iterator iter = _crossings.begin();
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
    for(std::map<int, Transition*>::const_iterator iter = _transitions.begin();
        iter != _transitions.end();
        ++iter) {
        delete iter->second;
    }
    for(std::map<int, Hline*>::const_iterator iter = _hLines.begin(); iter != _hLines.end();
        ++iter) {
        delete iter->second;
    }
    for(std::map<int, Goal*>::const_iterator iter = _goals.begin(); iter != _goals.end(); ++iter) {
        delete iter->second;
    }
}

const std::map<int, std::shared_ptr<Room>>& Building::GetAllRooms() const
{
    return _rooms;
}

std::tuple<Room*, SubRoom*> Building::GetRoomAndSubRoom(const Point position) const
{
    for(auto const& [_, room] : _rooms) {
        auto it = std::find_if(
            room->GetAllSubRooms().begin(),
            room->GetAllSubRooms().end(),
            [position](const auto& val) { return val.second->IsInSubRoom(position); });
        if(it != room->GetAllSubRooms().end()) {
            return {room.get(), it->second.get()};
        }
    }
    throw std::runtime_error(fmt::format(
        FMT_STRING("Position {} could not be found in any subroom."), position.toString()));
}

SubRoom* Building::GetSubRoom(const Point position) const
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
    for(auto const& [_, room] : _rooms) {
        auto it = std::find_if(
            room->GetAllSubRooms().begin(), room->GetAllSubRooms().end(), [pos](const auto& val) {
                return val.second->IsInSubRoom(pos);
            });
        if(it != room->GetAllSubRooms().end()) {
            return true;
        }
    }
    return false;
}

void Building::AddRoom(Room* room)
{
    _rooms[room->GetID()] = std::shared_ptr<Room>(room);
}

bool Building::InitGeometry()
{
    LOG_INFO("Init Geometry");
    for(auto&& itr_room : _rooms) {
        for(auto&& itr_subroom : itr_room.second->GetAllSubRooms()) {
            // create a close polyline out of everything
            std::vector<Line*> goals = std::vector<Line*>();

            //  collect all crossings
            for(auto&& cros : itr_subroom.second->GetAllCrossings()) {
                goals.push_back(cros);
            }
            // collect all transitions
            for(auto&& trans : itr_subroom.second->GetAllTransitions()) {
                goals.push_back(trans);
            }
            // initialize the poly
            if(!itr_subroom.second->ConvertLineToPoly(goals))
                return false;
            itr_subroom.second->CalculateArea();

            // do the same for the obstacles that are closed
            for(auto&& obst : itr_subroom.second->GetAllObstacles()) {
                if(!obst->ConvertLineToPoly())
                    return false;
            }

            // here we can create a boost::geometry::model::polygon out of the vector<Point> objects
            // created above
            itr_subroom.second->CreateBoostPoly();

            double minElevation = FLT_MAX;
            double maxElevation = -FLT_MAX;
            for(auto&& wall : itr_subroom.second->GetAllWalls()) {
                const Point& P1 = wall.GetPoint1();
                const Point& P2 = wall.GetPoint2();
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

    for(const auto& cross : _crossings) {
        SubRoom* s1 = cross.second->GetSubRoom1();
        SubRoom* s2 = cross.second->GetSubRoom2();
        if(s1)
            s1->AddNeighbor(s2);
        if(s2)
            s2->AddNeighbor(s1);
    }

    for(const auto& trans : _transitions) {
        SubRoom* s1 = trans.second->GetSubRoom1();
        SubRoom* s2 = trans.second->GetSubRoom2();
        if(s1)
            s1->AddNeighbor(s2);
        if(s2)
            s2->AddNeighbor(s1);
    }

    InitInsideGoals();
    LOG_INFO("Init Geometry successful!!!");

    return true;
}

bool Building::InitInsideGoals()
{
    bool found = false;
    for(auto& goalItr : _goals) {
        Goal* goal = goalItr.second;
        if(goal->GetRoomID() == -1) {
            found = true;
            continue;
        }

        for(auto& roomItr : _rooms) {
            Room* room = roomItr.second.get();

            if(goal->GetRoomID() != room->GetID()) {
                continue;
            }

            for(auto& subRoomItr : room->GetAllSubRooms()) {
                SubRoom* subRoom = subRoomItr.second.get();

                if((goal->GetSubRoomID() == subRoom->GetSubRoomID()) &&
                   (subRoom->IsInSubRoom(goal->GetCentroid()))) {
                    Crossing* crossing = goal->GetCentreCrossing();
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

bool Building::AddCrossing(Crossing* line)
{
    int IDRoom = line->GetRoom1()->GetID();
    int IDLine = line->GetUniqueID();
    int IDCrossing = 1000 * IDRoom + IDLine;
    if(_crossings.count(IDCrossing) != 0) {
        LOG_ERROR("Duplicate index for crossing found [{}] in Routing::AddCrossing()", IDCrossing);
        exit(EXIT_FAILURE);
    }
    _crossings[IDCrossing] = line;
    return true;
}

bool Building::RemoveTransition(const Transition* line)
{
    if(_transitions.count(line->GetID()) != 0) {
        _transitions.erase(line->GetID());
        return true;
    }
    return false;
}

bool Building::AddTransition(Transition* line)
{
    if(_transitions.count(line->GetID()) != 0) {
        LOG_ERROR(
            "Duplicate index for transition found [{}] in Routing::AddTransition()", line->GetID());
        exit(EXIT_FAILURE);
    }
    _transitions[line->GetID()] = line;

    return true;
}

bool Building::AddGoal(Goal* goal)
{
    if(_goals.count(goal->GetId()) != 0) {
        LOG_ERROR("Duplicate index for goal found [{}] in Routing::AddGoal()", goal->GetId());
        exit(EXIT_FAILURE);
    }
    _goals[goal->GetId()] = goal;

    return true;
}

const std::map<int, Crossing*>& Building::GetAllCrossings() const
{
    return _crossings;
}

const std::map<int, Transition*>& Building::GetAllTransitions() const
{
    return _transitions;
}

const std::map<int, Goal*>& Building::GetAllGoals() const
{
    return _goals;
}

Transition* Building::GetTransition(int ID) const // ar.graf: added const 2015-12-10
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

Room* Building::GetRoom(int index) const
{
    // TODO: obsolete since the check is done by .at()
    if(_rooms.count(index) == 0) {
        LOG_ERROR(
            "Wrong 'index' in CBuiling::GetRoom() Room ID: {} size: {}", index, _rooms.size());
        LOG_INFO("Control your rooms ID and make sure they are in the order 0, 1, 2,.. ");
        return nullptr;
    }
    return _rooms.at(index).get();
}

Transition* Building::GetTransitionByUID(int uid) const
{
    for(auto&& trans : _transitions) {
        if(trans.second->GetUniqueID() == uid)
            return trans.second;
    }
    return nullptr;
}

bool Building::SanityCheck()
{
    LOG_INFO("Checking the geometry for artifacts: (Ignore Warnings, if ff_[...] router is used!)");
    bool status = true;

    for(auto&& itr_room : _rooms) {
        for(auto&& itr_subroom : itr_room.second->GetAllSubRooms()) {
            if(!itr_subroom.second->SanityCheck())
                status = false;
        }
    }

    LOG_INFO("...Done.");
    return status;
}

void Building::InitGrid()
{
    // first look for the geometry boundaries
    double x_min = FLT_MAX;
    double x_max = FLT_MIN;
    double y_min = FLT_MAX;
    double y_max = FLT_MIN;

    // finding the bounding of the grid
    //  and collect the pedestrians
    for(auto&& itr_room : _rooms) {
        for(auto&& itr_subroom : itr_room.second->GetAllSubRooms()) {
            for(auto&& wall : itr_subroom.second->GetAllWalls()) {
                double x1 = wall.GetPoint1().x;
                double y1 = wall.GetPoint1().y;
                double x2 = wall.GetPoint2().x;
                double y2 = wall.GetPoint2().y;

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
    // make the grid slightly larger.
    x_min = x_min - 1 * cellSize;
    x_max = x_max + 1 * cellSize;
    y_min = y_min - 1 * cellSize;
    y_max = y_max + 1 * cellSize;

    // no algorithms
    //  the domain is made of a single cell
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

    LOG_INFO("Done with Initializing the grid");
}
