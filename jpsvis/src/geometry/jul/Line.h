/* 
 * File:   Line.h
 * Author: andrea
 *
 * Created on 30. September 2010, 09:40
 */

#ifndef _LINE_H
#define	_LINE_H

#include <stdio.h>
#include <stdlib.h>


#include "CPoint.h"


class Line {
private:
    CPoint pPoint1; // (Koordinaten des ersten Punkts)
    CPoint pPoint2; // (Koordinaten des zweiten Punkts)
public:
    Line();
    Line(const CPoint& p1, const CPoint& p2);
    Line(const Line& orig);
    virtual ~Line();

    // Setter -Funktionen
    void SetPoint1(const CPoint& p); // setzt die Anfangskoordinaten
    void SetPoint2(const CPoint& p); // setzt die Endkoordinaten

    // Getter - Funktionen
    const CPoint& GetPoint1(void) const; // gibt die Koordinaten des Anfangspunkts zurück
    const CPoint& GetPoint2(void) const; // gibt die Koordinaten des Endpunkts zurück

    // Sonstiges
    CPoint NormalVec() const; /* Normalen_Vector zu Cline*/
    float NormalComp(const CPoint& v) const; /* Normale Komponente von v auf l */
    CPoint ShortestPoint(const CPoint& p) const; /* Punkt auf Cline mit kürzstem Abstand zu p */
    bool IsInLine(const CPoint& p) const; /* Prüft, ob der p in der Wand ist oder außerhalb */
    float DistTo(const CPoint& p) const;

    //Ausgabe

};

#endif	/* _LINE_H */

