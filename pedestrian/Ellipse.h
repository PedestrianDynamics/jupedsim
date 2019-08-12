/**
 * \file        Ellipse.h
 * \date        Sep 30, 2010
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#pragma once

#include "general/Macros.h"
#include "geometry/Line.h"
#include "math/Mathematics.h"

class JEllipse {
private:
     Point _vel; // velocity vector
     Point _center; // cartesian-coord of the centre
     double _cosPhi; // cos(phi)
     double _sinPhi; // sin(phi)
     double _Xp; //x Ellipse-coord of the centre (Center in (xc,yc) )
     double _Amin; //   pAmin + V * pAv
     double _Av;
     double _Bmin; //  pBmax - V *[(pBmax - pBmin) / V0]
     double _Bmax;
     double _vel0; // desired speed
     bool _do_stretch = true; //Kraus-Model



public:
     JEllipse();


     void SetV(const Point& v);
     void SetCenter(Point pos);
     void SetCosPhi(double c);
     void SetSinPhi(double s);
     void SetXp(double xp);
     void SetAmin(double a_min);
     void SetAv(double a_v);
     void SetBmin(double b_min);
     void SetBmax(double b_max);
     void SetV0(double v0);
     void DoStretch(bool stretch);


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

     double GetEA()const;// ellipse semi-axis in the direction of the velocity
     double GetEB()const;// ellipse semi-axis in the orthogonal direction of the velocity
     double GetMaxEA()const;
     double GetMaxEB()const;
     double GetArea()const;
     bool DoesStretch() const;


     // Effective distance between two ellipses
     double EffectiveDistanceToEllipse(const JEllipse& other, double* dist) const;
     // Effective distance between ellipse and line segment
     double EffectiveDistanceToLine(const Line& l) const;
     // Schnittpunkt der Ellipse mit der Gerade durch P und AP (=ActionPoint von E)
     Point PointOnEllipse(const Point& p) const;
     // Schnittpunkt der Ellipse mit dem Liniensegment line
     Point PointOnEllipse(const Line& line, const Point& P) const;
     // minimal möglicher Abstand (durch Verschiebung) Ellipse <-> Segment
     //double MinimumDistanceToLine(const Line& l)const;
     // minimal möglicher Abstand (durch Verschiebung) Ellipse <-> Ellipse
     //double MinimumDistanceToEllipse(const JEllipse& E) const;
     // Check if point p is inside the ellipse
     bool IsInside(const Point& p) const;
     // Check if point p is outside the ellipse
     bool IsOutside(const Point& p) const;
     // Check if point p is on the ellipse
     bool IsOn(const Point& p) const;

     bool IntersectionWithLine(const Line& line);


};
