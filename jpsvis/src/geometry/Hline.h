/**
 * Hline.h
 *
 *  Created on: Aug 1, 2012
 *
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

#ifndef HLINE_H_
#define HLINE_H_

#include "Room.h"
#include "NavLine.h"

class SubRoom;
class Room;


class Hline: public NavLine {

private:
    int _id;
    Room* _room;
    std::string _caption;
    SubRoom* _subRoom;

public:
    Hline();
    virtual ~Hline();

    /**
     * Set/Get the id of the line
     */
    void SetID(int ID);

    /**
     * Set/Get the Room containing this line
     */
    void SetRoom(Room* r);

    /**
     * Set/Get the line caption
     */
    void SetCaption(std::string s);

    /**
     * Set/Get the subroom containing this line
     */
    void SetSubRoom(SubRoom* r);


    /**
     * Set/Get the id of the line
     */
    int GetID() const;

    /**
     * Set/Get the line caption
     */
    std::string GetCaption() const;

    /**
     * Set/Get the Room containing this line
     */
    Room* GetRoom() const;

    /**
     * Set/Get the subroom containing this line
     */
    SubRoom* GetSubRoom() const;


    /**
     * @return true if the line is in the given subroom
     */
    bool IsInSubRoom(int subroomID) const;

    /**
     * @return true if the line is in the given room
     */
    bool IsInRoom(int roomID) const;

    /**
     * Debug output
     */
    void WriteToErrorLog() const;

    /**
     * @return a nicely formatted string representation of the object
     */
    std::string WriteElement() const;
};

#endif /* HLINE_H_ */
