/*
 * File:   Crossing.h
 * Author: andrea
 *
 * Created on 16. November 2010, 12:56
 */

#ifndef _CROSSING_H
#define	_CROSSING_H

#include "Line.h"
#include "Room.h"

class Crossing : public Line {
private:
    int pID; // eindeutig zwischen ALLEN Crossings und Transitions
    Room* pRoom1; // benötigt nur einen Raum in dem beide SubRooms enthalten sind
    string pCaption;
    SubRoom* pSubRoom1; // SubRoom1
    SubRoom* pSubRoom2; // SubRoom2

public:
    // Konstruktoren
    Crossing();
    Crossing(const Point& p1, const Point& p2);
    Crossing(const Crossing& orig);
    virtual ~Crossing();
    // Setter -Funktionen
    void SetIndex(int ID);
    void SetRoom1(Room* r);
    void SetCaption(string s);
    void SetSubRoom1(SubRoom* r1);
    void SetSubRoom2(SubRoom* r2);
    // Getter - Funktionen
    int GetIndex() const;
    string GetCaption() const;
    Room* GetRoom1() const;
    SubRoom* GetSubRoom(int ID) const;
    SubRoom* GetSubRoom1() const;
    SubRoom* GetSubRoom2() const;
    // Sonstiges
    bool IsInSubRoom(int subroomID) const;


    // virtuelle Funktionen
    virtual bool IsOpen() const; // prüft ob Ausgang nach draußen, für Croosings IMMER true
    virtual bool IsExit() const; // prüft ob Ausgang nach draußen, für Croosings IMMER false
    virtual bool IsTransition() const; // check whether this is a transition or not
    virtual bool IsInRoom(int roomID) const; // prüft ob Crossing zu Raum mit roomID gehört
    virtual void UpdatePedestrian(int RoomID, int SubID, int PedID, Crossing* goal_new) const;
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const; // gibt anderen SubRoom zurück

    // Ausgabe
    virtual void WriteToErrorLog() const;
    virtual string WriteElement() const; // TraVisTo Ausgabe
};

#endif	/* _CROSSING_H */

