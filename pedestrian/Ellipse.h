/**
 * File:   Ellipse.h
 * Author: andrea
 *
 * Created on 30. September 2010, 17:14
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

#ifndef _ELLIPSE_H
#define	_ELLIPSE_H

#include "../geometry/Point.h"
#include "../geometry/Line.h"
#include "../general/Macros.h"
#include "../math/Mathematics.h"

class Ellipse {
private:
    Point pV; // Geschwindigkeitskoordinaten
    Point pCenter; // cartesian-coord of the centre
    double pCosPhi; // cos(phi)
    double pSinPhi; // sin(phi)
    double pXp; //x Ellipse-coord of the centre (Center in (xc,yc) )
    double pAmin; // Laenge 1. Achse:  pAmin + V * pAv
    double pAv;
    double pBmin; // Laenge 2. Achse: pBmax - V *[(pBmax - pBmin) / V0]
    double pBmax;
    double pV0; // Wunschgeschwindigkeit (Betrag)

    // private Funktionen:
    // effektiver Abstand Segement l und Ellipse (geg. durch ActionPoint)
    double EffectiveDistanceToLine(const Line& l) const;
    // Distance of Closest Approach of two arbitrary ellipses
    double Distance2d(const Ellipse& E) const;

public:
    /* Konstruktoren */
    Ellipse();
    Ellipse(const Ellipse& orig);
    //virtual ~Ellipse();

    /* Setter-Funktionen */
    void SetV(const Point& v); // setzt x und y-Koordinaten der Geschwindigkeit
    void SetCenter(Point pos); // setzt x und y-Koordinaten
    void SetCosPhi(double c);
    void SetSinPhi(double s);
    void SetXp(double xp);
    void SetAmin(double a_min);
    void SetAv(double a_v);
    void SetBmin(double b_min);
    void SetBmax(double b_max);
    void SetV0(double v0);

    /* Getter-Funktionen */
    const Point& GetV() const;
    const Point& GetCenter() const;
    double GetCosPhi() const;
    double GetSinPhi() const;
    double GetXp() const;
    double GetAmin() const;
    double GetAv() const;
    double GetBmin() const;
    double GetBmax() const;
    double GetV0() const;

    double GetEA()const;// ellipse axe in the direction of the velocity
    double GetEB()const;// ellipse axe in the orthogonal direction of the velocity
    double GetArea()const;

    // Sonstige Funktionen

    // effektiver Abstand zwischen zwei Ellipsen (geg. durch ActionPoints),
    double EffectiveDistanceToEllipse(const Ellipse& other, double* dist) const;
    // Schnittpunkt der Ellipse mit der Gerade durch P und AP (=ActionPoint von E)
    Point PointOnEllipse(const Point& p) const;
    // Schnittpunkt der Ellipse mit dem Liniensegment line
    Point PointOnEllipse(const Line& line, const Point& P) const;
    // minimal möglicher Abstand (durch Verschiebung) Ellipse <-> Segment
    double MinimumDistanceToLine(const Line& l)const;
    // minimal möglicher Abstand (durch Verschiebung) Ellipse <-> Ellipse
    double MinimumDistanceToEllipse(const Ellipse& E) const;
    // prüft, ob ein Punkt sich innerhalb der Ellipse befindet
    bool IsInside(const Point& p) const;
    // prüft, ob ein Punkt sich außerhalb der Ellipse befindet
    bool IsOutside(const Point& p) const;
    // prüft, ob ein Punkt sich auf der Ellipse befindet
    bool IsOn(const Point& p) const;

    bool IntersectionWithLine(const Line& line);


};

#endif	/* _ELLIPSE_H */

