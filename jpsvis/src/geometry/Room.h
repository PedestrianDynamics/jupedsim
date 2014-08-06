/**
 * \file File:   Room.h
 *
 * Created on 30. September 2010, 11:58
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

#ifndef _ROOM_H
#define	_ROOM_H

#include <string>
#include <algorithm>

#include "../geometry/SubRoom.h"
//class SubRoom;

class Room {
private:

    /// room ID and index
    int _id;
    /// room state
    RoomState _state;
    /// room caption
    std::string _caption;
    /// room elevation
    double _zPos;
    /// all subrooms/partitions of the room
    std::vector<SubRoom*> _subRooms;
    /// all transitions ids
    std::vector<int> _transitionsIDs;
    /// needed if the trajectories for this room are to be write in a special way
    OutputHandler* _outputFile;

public:
    Room();
    Room(const Room& orig);
    virtual ~Room();


    /**
     * Set/Get the id of the room which is also used as index
     */
    void SetID(int ID);

    /**
     * Set/Get the caption of the room
     */
    void SetCaption(std::string s);

    /**
     * Set/Get the elevation of the room
     */
    void SetZPos(double z);

    /**
     * Add a SubRoom at the given index
     */
    void SetSubRoom(SubRoom* subroom, int index);

    /**
     * Set/Get the state of the room as defined in the macro.h file
     */
    void SetState(RoomState state);

    /**
     * Set/Get the id of the room which is also used as index
     */
    int GetID() const;

    /**
     * Set/Get the caption of the room
     */
    std::string GetCaption() const;

    /**
     * Set/Get the elevation of the room
     */
    double GetZPos() const;

    /**
     * @return the number of subrooms
     */
    int GetNumberOfSubRooms() const;

    /**
     * @return a vector containing all subrooms
     */
    const std::vector<SubRoom*>& GetAllSubRooms() const;

    /**
     * @return a vector containing all transitions Ids
     */
    const std::vector<int>& GetAllTransitionsIDs() const;

    /**
     * @return the Subroom with the corresponding index
     */
    SubRoom* GetSubRoom(int index) const;

    /**
     * @return the number of pedestrians in the rooms (all subrooms)
     */
    int GetNumberOfPedestrians() const;

    /**
     * @return the state for this room
     */
    RoomState GetState()const;

    /**
     * Push a new subroom in the vector
     */
    void AddSubRoom(SubRoom* r);

    /**
     * Delete the subroom at the specified index
     */
    void DeleteSubRoom(int index);

    /**
     * Add a new transition id
     */
    void AddTransitionID(int ID);


    /**
     * Debug output for this class
     */
    void WriteToErrorLog() const;

    /**
     * Used by MPI in the case each room should be written in a specific file
     */
    void SetOutputHandler(OutputHandler* oh);

    /**
     * Used by MPI in the case each room should be written in a specific file
     */
    OutputHandler* GetOutputHandler() const;

};

#endif	/* _ROOM_H */

