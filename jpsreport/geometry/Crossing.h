/**
 * \file        Crossing.h
 * \date        Nov 16, 2010
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


#ifndef _CROSSING_H
#define _CROSSING_H

#include "Hline.h"
#include "NavLine.h"


//class Line;
class Room;
class SubRoom;

class Crossing : public Hline
{
private:
    /// TODO ? unique between crossings and transitions ?
    int _id;
    /// only one room needed, since a crossing only separates 2 subrooms
    //Room* _room1;
    //SubRoom* _subRoom1;
    //SubRoom* _subRoom2;

public:
    /**
      * Constructor
      */
    Crossing();

    /**
      * Destructor
      */
    virtual ~Crossing();

    /**
      * Set/Get the Id of the crossing
      * TODO: do you really want to shadow ?
      */
    void SetID(int ID);

    /**
      * Set/Get the Id of the crossing
      * TODO: do you really want to shadow ?
      */
    int GetID() const;

    /**
      * Set/Get the second subroom
      */
    //void SetSubRoom2(SubRoom* r2);

    /**
      * Set/Get the second subroom
      */
    //SubRoom* GetSubRoom2() const;

    /**
      * \return true if the subroomID is associated with the current crossing
      */
    bool IsInSubRoom(int subroomID) const;

    /**
      * @return true if the crossing is open = passable
      */
    virtual bool IsOpen() const;

    /**
      * @return true if the crossing is an exit/transition. (Transitions are derived from this class)
      * @see Transition
      */
    virtual bool IsExit() const;

    /**
      *
      * @return true if it is a transition
      */
    virtual bool IsTransition() const;

    /**
      * @return the other subroom not matching the data
      */
    virtual SubRoom * GetOtherSubRoom(int roomID, int subroomID) const;

    /**
      * Debug output
      */
    virtual void WriteToErrorLog() const;

    /**
      * @return a nicely formatted string representation of the object
      */
    virtual std::string GetDescription() const;
};

#endif /* _CROSSING_H */
