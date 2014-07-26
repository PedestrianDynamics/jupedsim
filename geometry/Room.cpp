/**
 * \file        Room.cpp
 * \date        Sep 30, 2010
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
#include "../IO/OutputHandler.h"

#include <sstream>

using namespace std;

/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room()
{
     _id = -1;
     _state=ROOM_CLEAN; //smoke-free
     _caption = "no room caption";
     _zPos = -1.0;
     _subRooms = vector<SubRoom* > ();
     _outputFile=NULL;
}

Room::Room(const Room& orig)
{
     _id = orig.GetID();
     _caption = orig.GetCaption();
     _zPos = orig.GetZPos();
     _subRooms = orig.GetAllSubRooms();
     _state=orig.GetState();
     _outputFile=orig.GetOutputHandler();
}

Room::~Room()
{
     for (unsigned int i = 0; i < _subRooms.size(); i++)
          delete _subRooms[i];
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Room::SetID(int ID)
{
     _id = ID;
}

void Room::SetCaption(string s)
{
     _caption = s;
}

void Room::SetZPos(double z)
{
     _zPos = z;
}

void Room::SetSubRoom(SubRoom* subroom, int index)
{
     if ((index >= 0) && (index < GetNumberOfSubRooms())) {
          _subRooms[index] = subroom;
     } else {
          Log->Write("ERROR: Wrong Index in Room::SetSubRoom()");
          exit(0);
     }
}

void Room::SetState(RoomState state)
{
     _state=state;
}


/*************************************************************
 Getter-Functions
 ************************************************************/
int Room::GetID() const
{
     return _id;
}

string Room::GetCaption() const
{
     return _caption;
}

double Room::GetZPos() const
{
     //if(pCaption=="070") return pZPos+1.0;
     return _zPos;
}

int Room::GetNumberOfSubRooms() const
{
     return _subRooms.size();
}

const vector<SubRoom*>& Room::GetAllSubRooms() const
{
     return _subRooms;
}

SubRoom* Room::GetSubRoom(int index) const
{
     if ((index >= 0) && (index < (int) _subRooms.size()))
          return _subRooms[index];
     else {
          char tmp[CLENGTH];
          sprintf(tmp,"ERROR: Room::GetSubRoom() Wrong subroom index [%d] for room index [%d] ",index,_id);
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
}


#ifdef _SIMULATOR

int Room::GetNumberOfPedestrians() const
{
     int sum = 0;
     for (int i = 0; i < GetNumberOfSubRooms(); i++) {
          sum += GetSubRoom(i)->GetNumberOfPedestrians();
     }
     return sum;
}

#endif // _SIMULATOR

RoomState Room::GetState() const
{
     return _state;
}



/*************************************************************
 Sonstige Funktionen
 ************************************************************/
void Room::AddSubRoom(SubRoom* r)
{
     _subRooms.push_back(r);
}

void Room::DeleteSubRoom(int index)
{
     if ((index >= 0) && (index < (int) _subRooms.size()))
          _subRooms.erase(_subRooms.begin() + index);
     else {
          Log->Write("ERROR: Wrong Index in Room::DeleteSubRoom()");
          exit(0);
     }
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/


void Room::WriteToErrorLog() const
{
     char tmp[CLENGTH];
     string s;
     sprintf(tmp, "\tRaum: %d [%s]:\n", _id, _caption.c_str());
     s.append(tmp);
     Log->Write(s);
     // SubRooms
     for (int i = 0; i < GetNumberOfSubRooms(); i++) {
          SubRoom* s = GetSubRoom(i);
          s->WriteToErrorLog();
     }

}

const vector<int>& Room::GetAllTransitionsIDs() const
{
     return _transitionsIDs;
}

void Room::AddTransitionID(int ID)
{
     _transitionsIDs.push_back(ID);
}

void Room::SetOutputHandler(OutputHandler* oh)
{
     _outputFile=oh;
}

OutputHandler* Room::GetOutputHandler() const
{
     return _outputFile;
}
