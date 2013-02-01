/**
 * File:   Transition.h
 *
 * Created on 16. November 2010, 12:57
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
 */

#ifndef _TRANSITION_H
#define	_TRANSITION_H

#include "Crossing.h"
#include <string>

class Room;
class Subroom;

class Transition : public Crossing {
private:
    Room* _room2;
    bool _isOpen;
    std::string _type;

public:

    Transition();
    virtual ~Transition();

    /**
     * Close the transition/door
     */
    void Close();

    /**
     * Open the transition/door
     */
    void Open();

    /**
     * Set/Get the type of the transition
     * TODO: where is type defined?
     */
    void SetType(std::string s);

    /**
     * Set/Get the second room associated with this transition.
     * The first one is set in the crossing class.
     */
    void SetRoom2(Room* ID);


    /**
     * Set/Get the type of the transition
     * TODO: where is type defined?
     */
    std::string GetType() const;

    /**
     * Set/Get the second room associated with this transition.
     * The first one is set in the crossing class.
     */
    Room* GetRoom2() const;


    /**
     * @return the other room.
     */
    Room* GetOtherRoom(int room_id) const;

    // virtual functions
    virtual bool IsOpen() const;
    virtual bool IsExit() const;
    virtual bool IsTransition() const;
    virtual bool IsInRoom(int roomID) const;
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const;


    virtual void WriteToErrorLog() const;
    virtual std::string WriteElement() const; // TraVisTo Ausgabe
};

#endif	/* _TRANSITION_H */

