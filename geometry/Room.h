/*
 * File:   Room.h
 * Author: andrea
 *
 * Created on 30. September 2010, 11:58
 */

#ifndef _ROOM_H
#define	_ROOM_H

#include <string>
#include <algorithm>

#include "../geometry/SubRoom.h"

class Room {
private:
    int pRoomID; // Raumindex
    int pState; // ROOM_SMOKED or ROOM_CLEAN
    std::string pCaption; // Raumname
    double pZPos; // Hoehenindex für Trajektorien
    std::vector<SubRoom*> pSubRooms; // alle Unterräume (Stairs und "normale")
    std::vector<int> pTransitionsIDs; // transitions only
    OutputHandler* pOutputFile;

public:
    Room();
    Room(const Room& orig);
    virtual ~Room();

    // Setter -Funktionen
    void SetRoomID(int ID);
    void SetCaption(string s);
    void SetZPos(double z);
    void SetAllSubRooms(const vector<SubRoom*>& subrooms);
    void SetSubRoom(SubRoom* subroom, int index);
    void SetRoomState(int state);

    // Getter - Funktionen
    int GetRoomID() const;
    string GetCaption() const;
    double GetZPos() const;
    int GetAnzSubRooms() const;
    const std::vector<SubRoom*>& GetAllSubRooms() const;
	const std::vector<int>& GetAllTransitionsIDs() const;
    SubRoom* GetSubRoom(int index) const;
    int GetRoomState()const;


    // Sonstiges
    void AddSubRoom(SubRoom* r);
    void DeleteSubRoom(int index);
	void AddTransitionID(int ID);


    // Ein-Ausgabe
    void LoadNormalSubRoom(std::ifstream* buildingfile, int* i);
    void LoadStair(std::ifstream* buildingfile, int* i);
    void WriteToErrorLog() const;
    //MPI
    void SetOutputHandler(OutputHandler* oh);
    OutputHandler* GetOutputHandler() const;

};

#endif	/* _ROOM_H */

