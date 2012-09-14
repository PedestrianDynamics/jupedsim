/**
 * File:   Line.h
 *
 * Created on 30. September 2010, 09:40
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
    bool IsInLineSegment(const Point& p) const;
    
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

