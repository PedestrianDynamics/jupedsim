/**
 * File:   Ellipse.cpp
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

#include "Ellipse.h"

using namespace std;

/*************************************************************
 private Funktionen
 ************************************************************/

/* effektiver Abstand Segement l und Ellipse (geg. durch ActionPoint)
 * Effektiver Abstand = Abstand Punkt auf dem Rand der Ellipse
 * Algorithms:
 *   - bestimme Lotfußpunkt (P) von L zum ActionPoint der Ellipse (AP)
 *     (ggf. Randpunkte des Segments L)
 *   - R ist der Schnittpunkt der Geraden durch AP und P
 *   - gesuchter Abstand zwischen R und AP
 * Parameter:
 *   Line l muss in den Koordinaten der Ellipse übergeben werden
 * */

double JEllipse::EffectiveDistanceToLine(const Line& linE) const {
	double eff_dist;
	// Koordinaten alle im System der Ellipse
	Point APinE = Point(_Xp, 0);
	Point PinE = linE.ShortestPoint(APinE); // Punkt auf l mit kuerzestem Abstand zu AP
	if (IsOn(PinE)) {// P liegt bereits auf der Ellipse => Abstand 0
		eff_dist = 0.0;
	} else {
		Point P = PinE.CoordTransToCart(this->_center, this->_cosPhi,
				this->_sinPhi); // in "normalen" Koordinaten
		Point R; //Schnittpunkt der geraden durch AP und P mit E

//		double dist = (PinE - APinE).Norm(); // beide Punkte in E
		R = this->PointOnEllipse(PinE); // PointOnEllipe gibt "normale" Koordinaten zurück
		eff_dist = (R - P).Norm(); // beide Punkte in "normalen" Koordinaten
	}
	return eff_dist;
}

/* Distance of Closest Approach of two arbitrary ellipses
 * ai: semi-axis length with a1>=b1, a2>=b2,
 * angle1=orientation of E1 with respect to C1C2, with C1: centre E1, C2 center E2.
 * angle2=orientation of E2 with respect to C1C2.
 *
 * Original Version: http://www.math.kent.edu/~zheng/ellipsoid.c
 * thanks to Xiaoyu zheng for sharing!
 * */

double JEllipse::Distance2d(const JEllipse& Ell) const {
	/*----------------- INPUT Params --------------*/
	double a1 = this->GetEA();
	double b1 = this->GetEB();
	double a2 = Ell.GetEA();
	double b2 = Ell.GetEB();
	double cos1 = _cosPhi;
	double sin1 = _sinPhi;
	double cos2 = Ell.GetCosPhi();
	double sin2 = Ell.GetSinPhi();
	Point c1c2 = (_center - Ell.GetCenter()).Normalized();
	double dummy;

	if (a1 < b1)
	{
		//switch axes
		dummy = a1;
		a1 = b1;
		b1 = dummy;
		// orthogonal axis --> phi+pi
		dummy = cos1;
		cos1 = -sin1;
		sin1 = cos1;
	}
	if (a2 < b2)
	{
		//switch axes
		dummy = a2;
		a2 = b2;
		b2 = dummy;
		// orthogonal axis --> phi+pi
		dummy = cos2;
		cos2 = -sin2;
		sin2 = cos2;
	}

	double qa1 = a1 * a1;
	double qb1 = b1 * b1;
	double qa2 = a2 * a2;
	double qb2 = b2 * b2;
	//make sure that the the major axis points always points into the uppper
	// two quadrants, Bug found by Sean Curtis
	if (sin1 < 0 )
	{
		sin1 = -sin1;
		cos1 = -cos1;
	}
	if (sin2 < 0 )
	{
		sin2 = -sin2;
		cos2 = -cos2;
	}
	//the fix on July 2012
/*	if(fabs(angle2-angle1)==pi)
	{
		angle2=angle1;
	}*/
	if ( fabs(sin1*cos2 - cos1*sin2) < J_EPS )//angle2-angle1=pi eqv. sin(angle1-angl1)=sin(pi)=0
	{
		sin1 = sin2;
		cos1 = cos2;
	}
	//

	Point e11 = Point(cos1, sin1); //unit vector of the direction of E1
	Point e12 = Point(cos2, sin2); //unit vector of the direction of E2
	/*----------------------------------------------*/

	double eps1, eps2, k1dotd, k2dotd, k1dotk2, nu, Ap[2][2], lambdaplus,
			lambdaminus, bp2, ap2, cosphi, tanphi2, delta, dp;
	complex<double> A, B, C, D, E, alpha, beta, gamma, P, Q, U, y, qu;


	eps1 = 1.0 - qb1 / qa1;
	eps2 = 1.0 - qb2 / qa2;
	k1dotd = e11.ScalarP(c1c2);
	k2dotd = e12.ScalarP(c1c2);
	k1dotk2 = e11.ScalarP(e12);
	nu = a1 / b1 - 1.0;

	double qk1dotk2 = k1dotk2 * k1dotk2;
	double qk1dotd = k1dotd * k1dotd;
	double qnu = nu * nu;
	Ap[0][0] = qb1 / qb2 * (1.0 + 0.5 * (1.0 + k1dotk2) * (nu * (2.0 + nu)
			- eps2 * (1.0 + nu * k1dotk2) * (1.0 + nu * k1dotk2)));
	Ap[1][1] = qb1 / qb2 * (1.0 + 0.5 * (1.0 - k1dotk2) * (nu * (2.0 + nu)
			- eps2 * (1.0 - nu * k1dotk2) * (1.0 - nu * k1dotk2)));

	// original:
	//Ap[0][1] = qb1 / qb2 * 0.5 * sqrt(1.0 - qk1dotk2)*
	//        (nu * (2.0 + nu) + eps2 * (1.0 - qnu * qk1dotk2));

	double tmp0 = fabs(1.0 - qk1dotk2) < J_EPS ? 0.0 : (1.0 - qk1dotk2);
	Ap[0][1] = qb1 / qb2 * 0.5 * sqrt(tmp0) * (nu * (2.0 + nu) + eps2 * (1.0
			- qnu * qk1dotk2));

	double tmp1 = 0.5 * (Ap[0][0] + Ap[1][1]);
	double tmp3 = Ap[0][0] - Ap[1][1];
	double tmp2 = 0.25 * tmp3 * tmp3 + Ap[0][1] * Ap[0][1];
	tmp3 = sqrt(tmp2);
	lambdaplus = tmp1 + tmp3;
	lambdaminus = tmp1 - tmp3;
	tmp1 = sqrt(lambdaplus);
	bp2 = 1.0 / tmp1;
	ap2 = 1.0 / sqrt(lambdaminus);

	//original: if (fabs(k1dotk2) == 1.0) {
	if ( fabs(k1dotk2) > 1.0 - J_EPS) {
		if (Ap[0][0] > Ap[1][1])
			cosphi = qb1 / qa1 * qk1dotd / (1.0 - eps1 * k1dotd * k1dotd);
		else
			cosphi = (1.0 - k1dotd * k1dotd) / (1.0 - eps1 * k1dotd * k1dotd);
	} else {
		double Tmp1, Tmp2, Tmp3, Tmp4, Tmp5, Tmp6, Tmp7, Tmp8;
		Tmp1 = 2.0 * (Ap[0][1] * Ap[0][1] + (lambdaplus - Ap[0][0])
				* (lambdaplus - Ap[0][0]));
		Tmp2 = (1.0 - eps1 * k1dotd * k1dotd);
		Tmp8 = Tmp1 * Tmp2;

		Tmp3 = k2dotd + (b1 / a1 - 1.0) * k1dotd * k1dotk2;
		Tmp4 = b1 / a1 * k1dotd;
		Tmp5 = Tmp4 + Tmp3;
		Tmp6 = Tmp4 - Tmp3;
		Tmp7 = Ap[0][1] / sqrt(1.0 + k1dotk2) * (Tmp5)
				+ (lambdaplus - Ap[0][0]) / sqrt(1.0 - k1dotk2) * (Tmp6);
		cosphi = 1.0 / Tmp8 * (Tmp7 * Tmp7);
	}
	double qap2 = ap2 * ap2;
	double qbp2 = bp2 * bp2;
	delta = qap2 / qbp2 - 1.0;
	if (delta == 0.0 || cosphi == 0.0)
		dp = 1.0 + ap2;
	else {
		double Tmp, Tmp1;
		tanphi2 = 1.0 / (cosphi) - 1.0;
		Tmp = 1.0 + tanphi2;
		Tmp1 = 1.0 + delta;
		A = -(Tmp) / qbp2;
		B = -2.0 * (Tmp + delta) / bp2;
		C = -tanphi2 - Tmp1 * Tmp1 + (1.0 + Tmp1 * tanphi2) / qbp2;
		D = 2.0 * Tmp * Tmp1 / bp2;
		E = (Tmp + delta) * Tmp1;
		complex<double> qA = A * A;
		complex<double> qB = B * B;
		alpha = -3.0 * qB / (8.0 * qA) + C / A;
		beta = qB * B / (8.0 * qA * A) - B * C / (2.0 * qA) + D / A;
		gamma = -3.0 * qB * qB / (256.0 * qA * qA) + C * qB / (16.0 * qA * A)
				- B * D / (4.0 * qA) + E / A;
		complex<double> qalpha = alpha * alpha;
		if (beta == 0.0) {
			qu = -B / (4.0 * A) + sqrt(
					0.5 * (-alpha + sqrt(qalpha - 4.0 * gamma)));
		} else {
			P = -qalpha / 12.0 - gamma;
			Q = -qalpha * alpha / 108.0 + alpha * gamma / 3.0 - beta * beta
					/ 8.0;
			U = c_cbrt(-Q * 0.5 + sqrt(Q * Q * 0.25 + P * P * P / 27.0));
			if (U == 0.0)
				y = -5.0 * alpha / 6.0 - c_cbrt(Q);
			else
				y = -5.0 * alpha / 6.0 + U - P / (3.0 * U);
			complex<double> tsqrt = sqrt(alpha + 2.0 * y);

			qu = -B / (4.0 * A) + 0.5 * (tsqrt + sqrt(
					-(3.0 * alpha + 2.0 * y + 2.0 * beta / tsqrt)));

		}
		complex<double> Tmp2 = (qu * qu - 1.0) / delta * (1.0 + bp2 * (1.0
				+ delta) / qu) * (1.0 + bp2 * (1.0 + delta) / qu) + (1.0 - (qu
				* qu - 1.0) / delta) * (1.0 + bp2 / qu) * (1.0 + bp2 / qu);
		dp = real(sqrt(Tmp2));
	}
	double result = dp * b1 / sqrt(1.0 - eps1 * qk1dotd);
	if (result != result || result < 0) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tEllipse::Distance2d(): result=%f\n", result);
		Log->Write(tmp);
		exit(0);
	}
	return (result);
}

/************************************************************
 Konstruktoren
 ************************************************************/

JEllipse::JEllipse() {
	_vel = Point(); // Geschwindigkeitskoordinaten
	_center = Point(); // cartesian-coord of the centre
	_cosPhi = 1; // = cos(0)
	_sinPhi = 0; // = sin(0)
	_Xp = 0; //x Ellipse-coord of the centre (Center in (xc,yc) )
	_Amin = 0.18; // Laenge 1. Achse:  pAmin + V * pAv
	_Av = 0.53;
	_Bmin = 0.20; // Laenge 2. Achse: pBmax - V *[(pBmax - pBmin) / V0]
	_Bmax = 0.25;
	__vel0 = 0; // Wunschgeschwindigkeit (Betrag)
}

JEllipse::JEllipse(const JEllipse& orig) {
	_vel = orig.GetV(); // Geschwindigkeitskoordinaten
	_center = orig.GetCenter();
	_cosPhi = orig.GetCosPhi();
	_sinPhi = orig.GetSinPhi();
	_Xp = orig.GetXp(); //x Ellipse-coord of the centre (Center in (xc,yc) )
	_Amin = orig.GetAmin(); // Laenge 1. Achse:  pAmin + V * pAv
	_Av = orig.GetAv();
	_Bmin = orig.GetBmin(); // Laenge 2. Achse: pBmax - V *[(pBmax - pBmin) / V0]
	_Bmax = orig.GetBmax();
	__vel0 = orig.GetV0(); // Wunschgeschwindigkeit (Betrag)
}


/*************************************************************
 Setter-Funktionen
 ************************************************************/

void JEllipse::SetV(const Point& v) {
	_vel = v;
}

void JEllipse::SetCenter(Point pos) {
	_center = pos;
}

void JEllipse::SetCosPhi(double c) {
	_cosPhi = c;
}

void JEllipse::SetSinPhi(double s) {
	_sinPhi = s;
}

void JEllipse::SetXp(double xp) {
	_Xp = xp;
}

void JEllipse::SetAmin(double a_min) {
	_Amin = a_min;
}

void JEllipse::SetAv(double a_v) {
	_Av = a_v;
}

void JEllipse::SetBmin(double b_min) {
	_Bmin = b_min;
}

void JEllipse::SetBmax(double b_max) {
	_Bmax = b_max;
}

void JEllipse::SetV0(double v0) {
	__vel0 = v0;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

const Point& JEllipse::GetV() const {
	return _vel;
}

const Point& JEllipse::GetCenter() const {
	return _center;
}

double JEllipse::GetCosPhi() const {
	return _cosPhi;
}

double JEllipse::GetSinPhi() const {
	return _sinPhi;
}

double JEllipse::GetXp() const {
	return _Xp;
}

double JEllipse::GetAmin() const {
	return _Amin;
}

double JEllipse::GetAv() const {
	return _Av;
}

double JEllipse::GetBmin() const {
	return _Bmin;
}

double JEllipse::GetBmax() const {
	return _Bmax;
}

double JEllipse::GetV0() const {
	return __vel0;
}
double JEllipse::GetArea() const {
	double x = (_Bmax - _Bmin) / __vel0;
	double V = _vel.Norm();
	double ea = _Amin + V * _Av;
	double eb = _Bmax - V * x;
	return ea * eb * M_PI;
}

// ellipse  semi-axis in the direction of the velocity
	double JEllipse::GetEA() const {
		return _Amin + _vel.Norm() * _Av;
	}

// ellipse semi-axis in the orthogonal direction of the velocity
double JEllipse::GetEB() const {
	double x = (_Bmax - _Bmin) / __vel0;
	return _Bmax - _vel.Norm() * x;
}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/

/* effektiver Abstand zwischen zwei Ellipsen (geg. durch ActionPoints),
 * Effektiver Abstand = Abstand zwischen den Randpunkten der Ellipsen
 * Algorithms:
 *   - ActionPoint von E1 (AP1) in Koordinaten von E2 transformieren
 *   - ActionPoint von E2 (AP2) in Koordinaten von E1 transformieren
 *   - R1 ist Schnittpunkt von E1 mit Geraden durch AP1 und AP2
 *     (ACHTUNG richtige Koordinaten verwenden!!!)
 *   - R2 ist Schnittpunkt von E2 mit Geraden durch AP1 und AP2
 *   - dist = ||AP1-AP2||
 *   - gesuchter effektiver Abstand = ||R1-R2||
 * Parameter:
 *   - Ellipse E2 in eigenen Koordinaten
 *   - dist: ist Rückgabewert
 * Rückgabewerte:
 *   - dist: Abstand der ActionPoints
 *   - gesuchter effektiver Abstand
 * */

double JEllipse::EffectiveDistanceToEllipse(const JEllipse& E2, double* dist) const {
	//  E1 ist Objekt auf dem aufgerufen wird
	Point AP1inE1 = Point(this->GetXp(), 0); //Koords in E1
	Point AP2inE2 = Point(E2.GetXp(), 0); //Koords in E2
	Point AP1inE2, AP2inE1;

	// "normale" Koordinaten
	Point R1, R2;
	Point AP1, AP2;
	// Koordinaten transformieren
	AP1 = AP1inE1.CoordTransToCart(this->GetCenter(), this->GetCosPhi(),
			this->GetSinPhi());
	AP2 = AP2inE2.CoordTransToCart(E2.GetCenter(), E2.GetCosPhi(),
			E2.GetSinPhi());
	AP1inE2 = AP1.CoordTransToEllipse(E2.GetCenter(), E2.GetCosPhi(),
			E2.GetSinPhi());
	AP2inE1 = AP2.CoordTransToEllipse(this->GetCenter(), this->GetCosPhi(),
			this->GetSinPhi());

	// Abstand zwischen den beiden Actionpoints
	*dist = (AP1 - AP2).Norm();
	/* fuer E1: Berechnung R1 */
	R1 = this->PointOnEllipse(AP2inE1); // kuerzester Schnittpunkt E1 mit Gerade durch AP2 von E2
	/* fuer E2: Berechung R2 */
	R2 = E2.PointOnEllipse(AP1inE2); // kuerzester Schnittpunkt E2 mit Gerade durch AP1 von E1
	// Abstand zwischen den beiden Ellipsen
	return (AP1 - AP2).Norm() - (AP1 - R1).Norm() - (AP2 - R2).Norm(); // negative Werte sind bei Überlappung möglich
}


//
// input: P is a point in the ellipse world.
// output: The point on the ellipse (in cartesian coord) that lays on the same line OP
// O being the center of the ellipse
Point JEllipse::PointOnEllipse(const Point& P) const {
	double x = P.GetX(), y = P.GetY();
	double r = x*x + y*y;
	if ( r < J_EPS*J_EPS)
	{
		return _center;
	}

	r = sqrt(r);

	double cosTheta = x/r;
	double sinTheta = y/r;
	double a = GetEA();
	double b = GetEB();
	Point S;
	S.SetX(a*cosTheta);
	S.SetY(b*sinTheta);
	return S.CoordTransToCart(_center, _cosPhi, _sinPhi);
}


// thanks to Sean Curtis. see manuals/Ellipsen/ellipseLineSean.pdf
double JEllipse::MinimumDistanceToLine(const Line& l) const {
	 Point AinE = l.GetPoint1().CoordTransToEllipse(_center, _cosPhi, _sinPhi);
	 Point BinE = l.GetPoint2().CoordTransToEllipse(_center, _cosPhi, _sinPhi);

	 	// Action Point der Ellipse
	 Point APinE = Point(_Xp, 0);
	 Line linE = Line(AinE, BinE);
	 double xa = linE.GetPoint1().GetX();
	 double ya = linE.GetPoint1().GetY();
	 double xb = linE.GetPoint2().GetX();
	 double yb = linE.GetPoint2().GetY();
	 double a = GetEA();
	 double b = GetEB();
	 Line l_strich_inE;
	 // Punkt auf line mit kürzestem Abstand zum Action Point der Ellipse
	Point PinE = linE.ShortestPoint(APinE);


	 double mindist; // Rückgabewert

	 // kürzester Punkt ist Randpunkt
	 if (PinE == AinE || PinE == BinE) {
		 mindist = 0;
	 } else {
		 double Dx, Dy      // D
		 , NormD, NormT;
		 double Nx, Ny;	    // N
		 double P1x, P1y; 	// P1
		 double Rx, Ry;     // R
		 double Tx, Ty;     // R
		 double d, e;
		 double dummy;
		 Dx = xa - xb;
		 Dy = ya - yb;

		 if(Dx*ya - Dy*xa < 0)
		 {
			 Dx = -Dx;
			 Dy = -Dy;
		 }

		 NormD = sqrt(Dx*Dx + Dy*Dy);
		 Dx /= NormD;
		 Dy /= NormD;
		 //N. The normal of the line
		 Nx = -Dy;
		 Ny = Dx;

		 Tx = -Dy/b;
		 Ty = Dx/a;
		 NormT = sqrt(Tx*Tx + Ty*Ty);
		 Tx /= NormT;
		 Ty /= NormT;

		 P1x = a*Nx;
		 P1y = b*Ny; //Eq. (2.3)

		 dummy = Nx*xa + Ny*ya; //second part of Eq. (2.1)

		 e = Nx*P1x + Ny*P1y - dummy; //Eq. (2.4)

		 //R
		 Rx = a*Tx;
		 Ry = b*Ty; // Eq. (2.13)

		 d = Nx*Rx + Ny*Ry - dummy;


/*		 if (1)
		 {
			 printf("\n----------- dca2 --------------\n");
			 printf("Dx = %.2f, Dy=%.2f (det=%.2f)\n", Dx, Dy, Dx*ya - Dy*xa);
			 printf("Nx = %.2f, Ny=%.2f\n", Nx, Ny);
			 printf("P1x = %.2f, P1y=%.2f\n", P1x, P1y);
			 printf("Rx = %.2f, Ry=%.2f\n", Rx, Ry);
			 printf("dummy=%f\n",dummy);
			 printf("theta=%.2f\n",theta*180/PI);
			 printf("e=%f, d=%f\n",e, d);
			 printf("-------------------------\n\n");
		 }*/
		 mindist = d - e;
	 }
	 return mindist;
}


/*
 min distance ellipses
 closest approach between two ellipses

 P1(x1,y1): Action-point in E1
 P2(x2,y2): Action-point in E2


 C1*------------*C2
 /              \
     /                \
    /                  \
P1 *			\
 *P2

 1. find the closest distance between C1 and C2: d
 2. translate   Ellipse2 (from right to left) with C1C2-d
 3. call edes with E1 and the translated Ellipse.

 *\  A
 *\ B
 \
-------*  C


 (xA-xB)/(xA-xC) = AB/AC
 --> xB = xA - (xA-xC)*AB/AC
 (yA-yB)/(yA-yC) = AB/AC
 --> yB = yA - (yA-yC)*AB/AC
 */
double JEllipse::MinimumDistanceToEllipse(const JEllipse& E2) const {
	JEllipse tmpE2 = E2; // verschobene Ellipse E2, so dass Berührung mit E1
	Point v; // Verschiebungsvektor
	Point tmpC2 = Point(); // verschobenes Centrum von E2
	const Point& C1 = this->GetCenter();
	const Point& C2 = E2.GetCenter();

	//double d = this->Distance2d(E2); //dist closest approach between centres
	double d = 0.0; //this->Distance2d(E2); //dist closest approach between centres
	double distance; //between c1 and c2
	double mind; // Rueckgabewert
	if (d < 0 || d != d) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() d=%f\n", d);
		Log->Write(tmp);
		exit(0);
	}

	if ((C1 - C2).NormSquare() < J_EPS*J_EPS) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() m=0\n"
			"xc1: %f xc2: %f yc1: %f yc2: %f\n", C1.GetX(), C2.GetX(),
				C1.GetY(), C2.GetY());
		Log->Write(tmp);
		exit(EXIT_FAILURE);
		//return 0.0;
	}
	// Verschiebungsvektor bestimmen und normieren
	v = (C2 - C1).Normalized();
	tmpC2 = C1 + v * d;
	tmpE2.SetCenter(tmpC2);
	mind = this->EffectiveDistanceToEllipse(tmpE2, &distance);
	if (mind < 0) { //Overlapping
		mind = 0.0;
	}
	return mind;
}


bool JEllipse::IsInside(const Point& p) const {
	double a = GetEA();
	double b = GetEB();
	double x = p.GetX();
	double y = p.GetY();

	return (x * x) / (a * a) + (y * y) / (b * b) < 1 + J_EPS_DIST;
}


bool JEllipse::IsOutside(const Point& p) const {
	double a = GetEA();
	double b = GetEB();
	double x = p.GetX();
	double y = p.GetY();

	return (x * x) / (a * a) + (y * y) / (b * b) > 1 - J_EPS_DIST;
}

/* prüft, ob ein Punkt sich außerhalb der Ellipse befindet
 * * Parameter:
 *    Point p muss in Koordinaten der Ellipse gegeben sein
 * Rückgabewert:
 *    - true, wenn auf der Ellipse
 *    - false, sonst
 * */

bool JEllipse::IsOn(const Point& p) const {
	double a = GetEA();
	double b = GetEB();
	double x = p.GetX();
	double y = p.GetY();
	double impliciteEllipse = (x * x) / (a * a) + (y * y) / (b * b) - 1;
	return (-J_EPS_DIST < impliciteEllipse) && (impliciteEllipse < J_EPS_DIST);
}

/*bool Ellipse::IntersectionWithLine(const Line& line) {
	//int Mathematics::IntersectionWithLine(ELLIPSE * E, float xa, float xb, float ya, float yb){
	//	float xc = E->xc, yc = E->yc;
	//	float phi = E->phi;
	//	float xanew, yanew;
	//	float xbnew, ybnew;
	//	float delta = -1;
	//	float c,d;
	//	float a2 = E->a*E->a;
	//	float b2 = E->b*E->b;
	//	int is = 0;
	//
	//	coord_trans(xa, ya, xc, yc, phi, &xanew, &yanew);
	//	coord_trans(xb, yb, xc, yc, phi, &xbnew, &ybnew);
	//
	//
	//	if(xanew != xbnew){
	//		c = (yanew - ybnew)/(xanew - xbnew);
	//		d = yanew - c * xanew;
	//		float d2=d*d;
	//		//delta = 4*c*c*d*d/b2/b2 - 4*( d*d/b2 - 1 )*( 1.0/a2 + c*c/b2 );
	//		delta = 4*(1.0/a2 + c*c/b2 -d2/b2/a2);
	//		is = (delta < 0)?0:1;
	//	}
	//	else{
	//		is = (E->a < fabs(xanew))?0:1;
	//	}
	//	return is;
	exit(EXIT_FAILURE); // what are you looking for here?
}
*/
