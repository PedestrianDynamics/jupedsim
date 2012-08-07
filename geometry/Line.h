/*
 * File:   Line.h
 * Author: andrea
 *
 * Created on 30. September 2010, 09:40
 */

#ifndef _LINE_H
#define	_LINE_H

#include "Point.h"
#include "../IO/OutputHandler.h"
#include "../general/Macros.h"

extern OutputHandler* Log;

class Line {
private:
	Point pPoint1; // (Koordinaten des ersten Punkts)
	Point pPoint2; // (Koordinaten des zweiten Punkts)
	Point pCentre;
public:
	// Konstruktoren
	Line();
	Line(const Point& p1, const Point& p2);
	Line(const Line& orig);
	virtual ~Line();

	// Setter -Funktionen
	void SetPoint1(const Point& p); // setzt die Anfangskoordinaten
	void SetPoint2(const Point& p); // setzt die Endkoordinaten

	// Getter - Funktionen
	const Point& GetPoint1(void) const; // gibt die Koordinaten des Anfangspunkts zurück
	const Point& GetPoint2(void) const; // gibt die Koordinaten des Endpunkts zurück
	const Point& GetCentre(void) const;

	// Ausgabe
	virtual string Write() const;

    // Sonstiges
    Point NormalVec() const; // Normalen_Vector zu Line
    double NormalComp(const Point& v) const; // Normale Komponente von v auf l
    Point LotPoint(const Point& p) const;
    Point ShortestPoint(const Point& p) const; // Punkt auf Line mit kürzstem Abstand zu p
    bool IsInLine(const Point& p) const; // Prüft, ob p in der Wand (Segment) liegt oder außerhalb
    double DistTo(const Point& p) const;
    double DistToSquare(const Point& p) const;
    double Length() const; // return the length/norm of the line
    double LengthSquare() const;  // return the square of the norm, for convenience sving some computation time

	bool operator==(const Line& l) const; // Vergleicht zwei Linien (Anfangs- und Endpunkte gleich)
	bool operator!=(const Line& l) const;

	//http://alienryderflex.com/intersect/
	//http://social.msdn.microsoft.com/Forums/en-US/
	//	csharpgeneral/thread/e5993847-c7a9-46ec-8edc-bfb86bd689e3/
	bool IntersectionWith(const Line& l) const; // check two segments for intersections

	bool IntersectionWithCircle(const Point& centre, double radius=0.30 /*m for pedestrians*/);

};



#endif	/* _LINE_H */

