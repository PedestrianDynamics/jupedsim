/* 
 * File:   Transition.h
 * Author: andrea
 *
 * Created on 30. September 2010, 10:12
 */

#ifndef _TRANSITION_H
#define	_TRANSITION_H

#include <string>
using namespace std;

#include "Line.h"

class Transition {
private:
    Line pLine;
    bool pIsOpen;
    int pTransitionID;
    string pCaption;
    int pRoom1;
    int pRoom2;
public:
    Transition();
    Transition(const Transition& orig);
    virtual ~Transition();

    // Setter -Funktionen
    void SetLine(const Line& l);
    void Close(); // schliesst Tür
    void Open(); // öffnet Tür
    void SetID(int ID);
    void SetCaption(string s);
    void SetRoom1(int r1);
    void SetRoom2(int r2);

    // Getter - Funktionen
    const Line& GetLine() const;
    bool GetIsOpen() const;
    int GetID() const;
    string GetCaption() const;
    int GetRoom1() const;
    int GetRoom2() const;

    // Sonstiges
    bool Connects(int r1id, int r2id) const;
    int GetOtherRoom(int room_id) const;
    
    // Ein-Ausgabe
    void WriteToErrorLog() const;
};

#endif	/* _TRANSITION_H */

