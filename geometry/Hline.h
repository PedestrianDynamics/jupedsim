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

class Line;
class SubRoom;


class Hline: public Line {

private:
    int pID;
    Room* pRoom;
    string pCaption;
    SubRoom* pSubRoom;

public:
	Hline();
	virtual ~Hline();

    // Setter -Funktionen
    void SetID(int ID);
    void SetRoom(Room* r);
    void SetCaption(string s);
    void SetSubRoom(SubRoom* r);

    // Getter - Funktionen
    int GetID() const;
    string GetCaption() const;
    Room* GetRoom() const;
    SubRoom* GetSubRoom() const;

    // Sonstiges
    bool IsInSubRoom(int subroomID) const;
    bool IsInRoom(int roomID) const;

    // Ausgabe
    void WriteToErrorLog() const;
    string WriteElement() const;
};

#endif /* HLINE_H_ */
