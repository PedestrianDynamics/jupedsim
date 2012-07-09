/*
 * File:   Transition.h
 * Author: andrea
 *
 * Created on 16. November 2010, 12:57
 */

#ifndef _TRANSITION_H
#define	_TRANSITION_H

#include "Room.h"
#include "Crossing.h"


class Transition : public Crossing {
private:
    Room* pRoom2; // zweiter Room zusätzlich zu Crossing
    bool pIsOpen;

public:
    // Konstruktoren
    Transition();
    Transition(const Transition& orig);
    virtual ~Transition();

    // Setter -Funktionen
    void Close(); // schliesst Tür
    void Open(); // öffnet Tür
    //void SetCaption(string s);
    void SetRoom2(Room* ID);

    // Getter - Funktionen
    //Room* GetRoom(int ID) const;
    Room* GetRoom2() const;
    // Sonstiges
    Room* GetOtherRoom(int room_id) const; // gibt anderen Room zurück

    // virtuelle Funktionen
    virtual bool IsOpen() const;
    virtual bool IsExit() const; // prüft ob Ausgang nach draußen
    virtual bool IsTransition() const; //check whether this is a transition or not
    virtual bool IsInRoom(int roomID) const; // prüft ob Transition zu Raum mit roomID gehört
    virtual void UpdatePedestrian(int RoomID, int SubID, int PedID, Crossing* goal_new) const;
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const; // gibt anderen SubRoom zurück

    // Ein-Ausgabe
    virtual void WriteToErrorLog() const;
    virtual string WriteElement() const; // TraVisTo Ausgabe
};

#endif	/* _TRANSITION_H */

