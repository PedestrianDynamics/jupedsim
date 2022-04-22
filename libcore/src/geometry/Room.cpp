/**
 * \file        Room.cpp
 * \date        Sep 30, 2010
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
#include "Room.hpp"

#include "IO/OutputHandler.hpp"
#include "SubRoom.hpp"

#include <Logger.hpp>
#include <memory>
#include <sstream>

/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room()
{
    _id = -1;
    _caption = "no room caption";
    _zPos = -1.0;
    _outputFile = nullptr;
    _egressTime = 0;
}

Room::Room(const Room& orig)
{
    _id = orig.GetID();
    _caption = orig.GetCaption();
    _zPos = orig.GetZPos();
    _outputFile = orig.GetOutputHandler();
    _egressTime = orig.GetEgressTime();
}

Room::~Room()
{
}

void Room::SetID(int ID)
{
    _id = ID;
}

void Room::SetCaption(const std::string& s)
{
    _caption = s;
}

void Room::SetZPos(double z)
{
    _zPos = z;
}

void Room::SetEgressTime(double time)
{
    _egressTime = time;
}

int Room::GetID() const
{
    return _id;
}

const std::string& Room::GetCaption() const
{
    return _caption;
}

double Room::GetZPos() const
{
    return _zPos;
}

double Room::GetEgressTime() const
{
    return _egressTime;
}

int Room::GetNumberOfSubRooms() const
{
    return _subRooms.size();
}

const std::map<int, std::shared_ptr<SubRoom>>& Room::GetAllSubRooms() const
{
    return _subRooms;
}

SubRoom* Room::GetSubRoom(const Point position) const
{
    auto it = std::find_if(_subRooms.begin(), _subRooms.end(), [position](const auto& val) {
        return val.second->IsInSubRoom(position);
    });
    if(it != _subRooms.end()) {
        return it->second.get();
    }
    throw std::runtime_error(fmt::format(
        FMT_STRING("Position {} could not be found in any subroom."), position.toString()));
}

SubRoom* Room::GetSubRoom(int index) const
{
    // TODO the check is done in _subRooms.at(index);
    if(_subRooms.count(index) == 0) {
        LOG_ERROR("Room::GetSubRoom() No subroom id [{}] present in room id [{}].", index, _id);
        return nullptr;
    }
    return _subRooms.at(index).get();
}

void Room::AddSubRoom(SubRoom* r)
{
    _subRooms[r->GetSubRoomID()] = std::shared_ptr<SubRoom>(r);
}

const std::vector<int>& Room::GetAllTransitionsIDs() const
{
    return _transitionsIDs;
}

void Room::AddTransitionID(int ID)
{
    _transitionsIDs.push_back(ID);
}

void Room::SetOutputHandler(OutputHandler* oh)
{
    _outputFile = oh;
}

std::vector<Point> Room::GetBoundaryVertices() const
{
    double xMin = FLT_MAX;
    double yMin = FLT_MAX;
    double xMax = -FLT_MAX;
    double yMax = -FLT_MAX;

    for(auto&& itr_subroom : this->GetAllSubRooms()) {
        const std::vector<Point> vertices = itr_subroom.second->GetPolygon();

        for(Point point : vertices) {
            if(point.x > xMax)
                xMax = point.x;
            if(point.x < xMin)
                xMin = point.x;
            if(point.y > yMax)
                yMax = point.y;
            if(point.y < yMin)
                yMin = point.y;
        }
    }

    return std::vector<Point>{
        Point(xMin, yMin), Point(xMin, yMax), Point(xMax, yMax), Point(xMax, yMin)};
}

OutputHandler* Room::GetOutputHandler() const
{
    return _outputFile;
}
