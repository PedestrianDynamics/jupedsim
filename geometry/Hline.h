/**
 * \file        Hline.h
 * \date        Aug 1, 2012
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

#include "NavLine.h"

class SubRoom;
class Room;


class Hline: public NavLine {

protected:
     int _id;
     std::string _caption;

//TODO: these should be redesigned as private
//and accessed via functions in the derived classes.
protected:
     Room* _room1=nullptr;
     SubRoom* _subRoom1=nullptr;
     SubRoom* _subRoom2=nullptr;

public:
     /**
      * Constructor
      */
     Hline();

     /**
      * Destructor
      */
     virtual ~Hline();

     /**
      * Set/Get the id of the line
      */
     void SetID(int ID);

     /**
      * Set/Get the Room containing this line
      */
     void SetRoom1(Room* r);

     /**
      * Set/Get the line caption
      */
     void SetCaption(std::string s);

     /**
      * Set/Get the subroom containing this line
      */
     void SetSubRoom1(SubRoom* r);
     /**
      * Set/Get the second subroom
      */
     void SetSubRoom2(SubRoom* r2);

     /**
      * Set/Get the id of the line
      */
     int GetID() const;

     /**
      * Set/Get the line caption
      */
     virtual std::string GetCaption() const;

     /**
      * Set/Get the Room containing this line
      */
     Room* GetRoom1() const;

     /**
      * Set/Get the subroom containing this line
      */
     SubRoom* GetSubRoom1() const;

     /**
      * Set/Get the second subroom
      */
     SubRoom* GetSubRoom2() const;

     /**
      * @return true if the line is in the given subroom
      */
     virtual bool IsInSubRoom(int subroomID) const;

     /**
      * @return true if the line is in the given room
      */
     virtual bool IsInRoom(int roomID) const;

     /**
      * Debug output
      */
     virtual void WriteToErrorLog() const;

     /**
      * @return a nicely formatted string representation of the object
      */
     virtual std::string GetDescription() const;
};
