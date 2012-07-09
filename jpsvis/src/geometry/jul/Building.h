/* 
 * File:   Building.h
 * Author: andrea
 *
 * Created on 1. Oktober 2010, 09:25
 */

#ifndef _BUILDING_H
#define	_BUILDING_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

#include "Room.h"
#include "Macros.h"

class Building {
private:
    string pCaption; // Name des Projekts
    vector<Room> pRooms; // Liste der Räume

    // wird nur innerhalb von Building benötigt
    void LoadHeader(ifstream* buildingfile, int* i);
    void LoadRooms(ifstream* buildingfile, int* i);
    void LoadRoom(ifstream* buildingfile, int* i);
    
    void LoadTransitions(ifstream* buildingfile, int* i);
    void LoadTransition(ifstream* buildingfile, int* i);
public:
    Building();
    Building(const Building& orig);
    virtual ~Building();

    // Setter -Funktionen
    void SetCaption(string s);
    void SetAllRooms(const vector<Room>& rooms);
    void SetRoom(const Room& room, int index);

    // Getter - Funktionen
    string GetCaption() const;
    const vector<Room>& GetAllRooms() const;
    const Room& GetRoom(int index) const; // Gibt Raum der Nummer "index" zurueck

    // Sonstiges
    void InitGeometry();
    void AddRoom(const Room& room);
    int RoomIndexForThisID(int ID) const;

    // Ein-Ausgabe
    void LoadFromFile(string filename); // Laedt Geometrie-Datei
    void WriteToErrorLog() const;
};

#endif	/* _BUILDING_H */

