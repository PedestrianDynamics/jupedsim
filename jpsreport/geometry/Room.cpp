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


#include "Room.h"

#include "SubRoom.h"

#include <Logger.h>
#include <memory>
#include <sstream>

using namespace std;

/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room()
{
    _id         = -1;
    _state      = ROOM_CLEAN; //smoke-free
    _egressTime = 0;
    _caption    = "no room caption";
    _zPos       = -1.0;
}

Room::Room(const Room & orig)
{
    _id         = orig.GetID();
    _caption    = orig.GetCaption();
    _zPos       = orig.GetZPos();
    _state      = orig.GetState();
    _egressTime = orig.GetEgressTime();
}

Room::~Room()
{
    //for (unsigned int i = 0; i < _subRooms.size(); i++)
    //delete _subRooms[i];
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Room::SetID(int ID)
{
    _id = ID;
}

void Room::SetCaption(const string & s)
{
    _caption = s;
}

void Room::SetZPos(double z)
{
    _zPos = z;
}

void Room::SetState(RoomState state)
{
    _state = state;
}

void Room::SetEgressTime(double time)
{
    _egressTime = time;
}

/*************************************************************
 Getter-Functions
 ************************************************************/
int Room::GetID() const
{
    return _id;
}

const string & Room::GetCaption() const
{
    return _caption;
}

double Room::GetZPos() const
{
    //if(pCaption=="070") return pZPos+1.0;
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

const std::map<int, std::unique_ptr<SubRoom>> & Room::GetAllSubRooms() const
{
    return _subRooms;
}

SubRoom * Room::GetSubRoom(int index) const
{
    //todo: the check is done in _subRooms.at(index);
    if(_subRooms.count(index) == 0) {
        LOG_ERROR("Room::GetSubRoom() No subroom id [{}] present in room id [{}] ", index, _id);
        return nullptr;
    }
    return _subRooms.at(index).get();
}


#ifdef _SIMULATOR

#endif // _SIMULATOR

const RoomState & Room::GetState() const
{
    return _state;
}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/
void Room::AddSubRoom(SubRoom * r)
{
    //_subRooms.push_back(r);
    _subRooms[r->GetSubRoomID()] = std::unique_ptr<SubRoom>(r);
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/


void Room::WriteToErrorLog() const
{
    LOG_ERROR("Room: {} [{}]:\n", _id, _caption);

    // SubRooms
    for(int i = 0; i < GetNumberOfSubRooms(); i++) {
        SubRoom * sub = GetSubRoom(i);
        sub->WriteToErrorLog();
    }
}

const vector<int> & Room::GetAllTransitionsIDs() const
{
    return _transitionsIDs;
}

void Room::AddTransitionID(int ID)
{
    _transitionsIDs.push_back(ID);
}
