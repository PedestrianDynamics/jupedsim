/*
 * Hline.h
 *
 *  Created on: Aug 1, 2012
 *      Author: Ulrich Kemloh
 */

#ifndef HLINE_H_
#define HLINE_H_

#include "Line.h"
#include "SubRoom.h"
#include "Room.h"

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
