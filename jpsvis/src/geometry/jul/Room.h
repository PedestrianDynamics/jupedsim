/* 
 * File:   Room.h
 * Author: andrea
 *
 * Created on 30. September 2010, 11:58
 */

#ifndef _ROOM_H
#define	_ROOM_H

#include <stdio.h>


#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
using namespace std;

#include "Transition.h"
#include "Wall.h"
#include "Macros.h"

class Room {
private:
    int pRoomID; // Raumindex
    string pCaption; // Raumname
    int pZPos; // Hoehenindex, kontinuierlich vielleicht unnötig?
    vector<Wall> pWalls; // Wände
    vector<Transition> pTransitions; // Ausgänge
    vector<CPoint> pPoly; // Alle Eckpunkte des Raums als Polygon

public:
    Room();
    Room(const Room& orig);
    virtual ~Room();

    // Setter -Funktionen
    void SetRoomID(int ID);
    void SetCaption(string s);
    void SetZPos(int z);
    void SetAllWalls(const vector<Wall>& walls);
    void SetWall(const Wall& wall, int index);
    void SetAllTransitions(const vector<Transition>& transitions);
    void SetTransition(const Transition& transition, int index);
    void SetPolygon(const vector<CPoint>& poly);

    // Getter - Funktionen
    int GetRoomID() const;
    string GetCaption() const;
    int GetZPos() const;
    const vector<Wall>& GetAllWalls() const;
    const Wall GetWall(int index) const;
    const vector<Transition> GetAllTransitions() const;
    const Transition GetTransition(int index) const;
    const vector<CPoint>& GetPolygon() const;

    // Sonstiges
    void AddWall(const Wall& w);
    void DeleteWall(int index);
    void AddTransition(const Transition& trans);
    void DeleteTransition(int index);
    void CloseTransition(int index);
    void ConvertLineToPoly(); // Erstellt einen Polygonzug aus pLines
    bool IsInRoom(const CPoint& ped) const;
    bool IsStart() const; //später ändern im Moment wird Anfang und Ende in Caption festgelegt
    bool IsExit() const; //später ändern im Moment wird Anfang und Ende in Caption festgelegt
    CPoint GetCentre() const;


    // Ein-Ausgabe
    void LoadWalls(ifstream* buildingfile, int* i);
    void WriteToErrorLog() const;

};

#endif	/* _ROOM_H */

