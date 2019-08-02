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

#include "IO/OutputHandler.h"

#include <sstream>
#include <memory>


/************************************************************
  Konstruktoren
 ************************************************************/

Room::Room()
{
     _id = -1;
     _state=ROOM_CLEAN; //smoke-free
     _caption = "no room caption";
     _zPos = -1.0;
     _outputFile=nullptr;
     _egressTime=0;
}

Room::Room(const Room& orig)
{
     _id = orig.GetID();
     _state=orig.GetState();
     _caption = orig.GetCaption();
     _zPos = orig.GetZPos();
     _outputFile=orig.GetOutputHandler();
     _egressTime=orig.GetEgressTime();
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

void Room::SetCaption(const std::string& s)
{
     _caption = s;
}

void Room::SetZPos(double z)
{
     _zPos = z;
}

void Room::SetState(RoomState state)
{
     _state=state;
}

void Room::SetEgressTime(double time)
{
     _egressTime=time;
}

/*************************************************************
 Getter-Functions
 ************************************************************/
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

const std::map<int, std::shared_ptr<SubRoom> >& Room::GetAllSubRooms() const
{
     return _subRooms;
}

SubRoom* Room::GetSubRoom(int index) const
{
     //todo: the check is done in _subRooms.at(index);
     if(_subRooms.count(index)==0)
     {
          Log->Write("ERROR: Room::GetSubRoom() No subroom id [%d] present in room id [%d] ",index,_id);
          return nullptr;
     }
     return _subRooms.at(index).get();
}


#ifdef _SIMULATOR

#endif // _SIMULATOR

const RoomState& Room::GetState() const
{
     return _state;
}



/*************************************************************
 Sonstige Funktionen
 ************************************************************/
void Room::AddSubRoom(SubRoom* r)
{
     //_subRooms.push_back(r);
     _subRooms[r->GetSubRoomID()]=std::shared_ptr<SubRoom>(r);
}

/*************************************************************
 Ein-Ausgabe
 ************************************************************/


void Room::WriteToErrorLog() const
{
     char tmp[CLENGTH];
     std::string s;
     sprintf(tmp, "\tRaum: %d [%s]:\n", _id, _caption.c_str());
     s.append(tmp);
     Log->Write(s);
     // SubRooms
     for (int i = 0; i < GetNumberOfSubRooms(); i++) {
          SubRoom*sub = GetSubRoom(i);
          sub->WriteToErrorLog();
     }

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
     _outputFile=oh;
}

std::vector<Point> Room::GetBoundaryVertices() const
{

    double xMin=FLT_MAX;
    double yMin=FLT_MAX;
    double xMax=-FLT_MAX;
    double yMax=-FLT_MAX;

     for(auto&& itr_subroom: this->GetAllSubRooms())
     {
         const std::vector<Point> vertices = itr_subroom.second->GetPolygon();

         for (Point point:vertices)
         {
             if (point._x>xMax)
                 xMax=point._x;
             if (point._x<xMin)
                 xMin=point._x;
             if (point._y>yMax)
                 yMax=point._y;
             if (point._y<yMin)
                 yMin=point._y;
         }
     }

    return std::vector<Point>{Point(xMin,yMin),Point(xMin,yMax),Point(xMax,yMax),Point(xMax,yMin)};

}


OutputHandler* Room::GetOutputHandler() const
{
     return _outputFile;
}
