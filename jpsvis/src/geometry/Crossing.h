/**
 * File:   Crossing.h
 *
 *
 * Created on 16. November 2010, 12:56
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#ifndef _CROSSING_H
#define	_CROSSING_H

#include "NavLine.h"

//class Line;
class Room;
class SubRoom;

class Crossing : public NavLine {
private:
    /// ? unique between crossings and transitions ?
    int _id;
    /// only one room needed, since a crossing only separates 2 subrooms
    Room* _room1;
    std::string _caption;
    SubRoom* _subRoom1;
    SubRoom* _subRoom2;

public:
    Crossing();
    virtual ~Crossing();

    /**
     * Set/Get the Id of the crossing
     */
    void SetID(int ID);

    /**
     * Set/Get the Id of the crossing
     */
    int GetID () const;

    /**
     * Set/Get the first room
     */
    void SetRoom1(Room* r);

    /**
     * Set/Get the crossing caption
     */
    void SetCaption(std::string s);

    /**
     * Set/Get the first subroom
     */
    void SetSubRoom1(SubRoom* r1);

    /**
     * Set/Get the second subroom
     */
    void SetSubRoom2(SubRoom* r2);

    /**
     * Set/Get the crossing caption
     */
    std::string GetCaption() const;

    /**
     * Set/Get the first room
     */
    Room* GetRoom1() const;

    /**
     * Set/Get the first subroom
     */
    SubRoom* GetSubRoom1() const;

    /**
     * Set/Get the second subroom
     */
    SubRoom* GetSubRoom2() const;

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
     * @return true if the crossing/transintion/hline is associated with the room
     */
    virtual bool IsInRoom(int roomID) const;

    /**
     * @return the other subroom not matching the data
     */
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const;

    /**
     * Debug output
     */
    virtual void WriteToErrorLog() const;

    /**
     * @return a nicely formatted string representation of the object
     */
    virtual std::string WriteElement() const;
};

#endif	/* _CROSSING_H */

