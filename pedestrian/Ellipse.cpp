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

double Ellipse::EffectiveDistanceToLine(const Line& linE) const {
	double eff_dist;
	// Koordinaten alle im System der Ellipse
	Point APinE = Point(pXp, 0);
	Point PinE = linE.ShortestPoint(APinE); // Punkt auf l mit kuerzestem Abstand zu AP
	if (IsOn(PinE)) {// P liegt bereits auf der Ellipse => Abstand 0
		eff_dist = 0.0;
	} else {
		Point P = PinE.CoordTransToCart(this->pCenter, this->pCosPhi,
				this->pSinPhi); // in "normalen" Koordinaten
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

double Ellipse::Distance2d(const Ellipse& Ell) const {
	/*----------------- INPUT Params --------------*/
	double a1 = this->GetLargerAxis(), qa1 = a1 * a1;
	double b1 = this->GetSmallerAxis(), qb1 = b1 * b1;
	double a2 = Ell.GetLargerAxis(), qa2 = a2 * a2;
	double b2 = Ell.GetSmallerAxis(), qb2 = b2 * b2;
	double cos1 = pCosPhi;
	double sin1 = pSinPhi;
	double cos2 = Ell.GetCosPhi();
	double sin2 = Ell.GetSinPhi();
	Point c1c2 = (pCenter - Ell.GetCenter()).Normalized();
	Point e11 = Point(cos1, sin1); //unit vector of the direction of E1
	Point e12 = Point(cos2, sin2); //unit vector of the direction of E2
	/*----------------------------------------------*/
	double eps1, eps2, k1dotd, k2dotd, k1dotk2, nu, Ap[2][2], lambdaplus,
			lambdaminus, bp2, ap2, cosphi, tanphi2, delta, dp;
	complex<double> A, B, C, D, E, alpha, beta, gamma, P, Q, U, y, qu;

	if (a1 < b1 || a2 < b2) {
		char tmp[CLENGTH];
		sprintf(
				tmp,
				"ERROR: \tEllipse::Distance2d() a1=%f < b1=%f || a2=%f<b2=%f\n",
				a1, b1, a2, b2);
		Log->write(tmp);
		exit(0);
	}
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

	double tmp0 = fabs(1.0 - qk1dotk2) < EPS ? 0.0 : (1.0 - qk1dotk2);
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
	if ((fabs(k1dotk2) - 1.0) < EPS) {
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
		Log->write(tmp);
		exit(0);
	}
	return (result);
}

/************************************************************
 Konstruktoren
 ************************************************************/

Ellipse::Ellipse() {
	pV = Point(); // Geschwindigkeitskoordinaten
	pCenter = Point(); // cartesian-coord of the centre
	pCosPhi = 1; // = cos(0)
	pSinPhi = 0; // = sin(0)
	pXp = 0; //x Ellipse-coord of the centre (Center in (xc,yc) )
	pAmin = 0.18; // Laenge 1. Achse:  pAmin + V * pAv
	pAv = 0.53;
	pBmin = 0.20; // Laenge 2. Achse: pBmax - V *[(pBmax - pBmin) / V0]
	pBmax = 0.25;
	pV0 = 0; // Wunschgeschwindigkeit (Betrag)
}

Ellipse::Ellipse(const Ellipse& orig) {
	pV = orig.GetV(); // Geschwindigkeitskoordinaten
	pCenter = orig.GetCenter();
	pCosPhi = orig.GetCosPhi();
	pSinPhi = orig.GetSinPhi();
	pXp = orig.GetXp(); //x Ellipse-coord of the centre (Center in (xc,yc) )
	pAmin = orig.GetAmin(); // Laenge 1. Achse:  pAmin + V * pAv
	pAv = orig.GetAv();
	pBmin = orig.GetBmin(); // Laenge 2. Achse: pBmax - V *[(pBmax - pBmin) / V0]
	pBmax = orig.GetBmax();
	pV0 = orig.GetV0(); // Wunschgeschwindigkeit (Betrag)
}

//Ellipse::~Ellipse() {
//}

/*************************************************************
 Setter-Funktionen
 ************************************************************/

void Ellipse::SetV(const Point& v) {
	pV = v;
}

void Ellipse::SetCenter(Point pos) {
	pCenter = pos;
}

void Ellipse::SetCosPhi(double c) {
	pCosPhi = c;
}

void Ellipse::SetSinPhi(double s) {
	pSinPhi = s;
}

void Ellipse::SetXp(double xp) {
	pXp = xp;
}

void Ellipse::SetAmin(double a_min) {
	pAmin = a_min;
}

void Ellipse::SetAv(double a_v) {
	pAv = a_v;
}

void Ellipse::SetBmin(double b_min) {
	pBmin = b_min;
}

void Ellipse::SetBmax(double b_max) {
	pBmax = b_max;
}

void Ellipse::SetV0(double v0) {
	pV0 = v0;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

const Point& Ellipse::GetV() const {
	return pV;
}

const Point& Ellipse::GetCenter() const {
	return pCenter;
}

double Ellipse::GetCosPhi() const {
	return pCosPhi;
}

double Ellipse::GetSinPhi() const {
	return pSinPhi;
}

double Ellipse::GetXp() const {
	return pXp;
}

double Ellipse::GetAmin() const {
	return pAmin;
}

double Ellipse::GetAv() const {
	return pAv;
}

double Ellipse::GetBmin() const {
	return pBmin;
}

double Ellipse::GetBmax() const {
	return pBmax;
}

double Ellipse::GetV0() const {
	return pV0;
}
double Ellipse::GetArea() const {
	double x = (pBmax - pBmin) / pV0;
	double V = pV.Norm();
	double ea = pAmin + V * pAv;
	double eb = pBmax - V * x;
	return ea * eb * M_PI;
}
double Ellipse::GetLargerAxis() const {
	if (pV0 < EPS) {
		Log->write("ERROR: \tCEllipse::GetLargerAxis() v0 of ped is null\n");
		exit(0);
	}
	double x = (pBmax - pBmin) / pV0;
	double V = pV.Norm();
	double ea = pAmin + V * pAv;
	double eb = pBmax - V * x;
	// groessere Achse zurueckgeben
	return (ea > eb) ? ea : eb;
}

// ellipse axe in the direction of the velocity
double Ellipse::GetEA() const {
	return pAmin + pV.Norm() * pAv;
}

// ellipse axe in the direction of the velocity
double Ellipse::GetEB() const {
	double x = (pBmax - pBmin) / pV0;
	return pBmax - pV.Norm() * x;
}

double Ellipse::GetSmallerAxis() const {
	if (pV0 < EPS) {
		Log->write("ERROR: \tCEllipse::GetSmallerAxis: v0 of ped is null\n");
		exit(0);
	}
	double x = (pBmax - pBmin) / pV0;
	double V = pV.Norm();
	double ea = pAmin + V * pAv;
	double eb = pBmax - V * x;
	// kleinere Achse zurueck
	return (ea < eb) ? ea : eb;
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

double Ellipse::EffectiveDistanceToEllipse(const Ellipse& E2, double* dist) const {
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

/* Berechnet den Punkt auf der Ellipse, der Schnittpunkt der Geraden durch P und dem
 * ActionPoint (AP) der Ellipse E, ist (wird beim effektiven Abstand benötigt)
 * Algorithms:
 *   - berechne Gerade g: y=m*x+n durch P und AP
 *   - Schnittpunkte S1, S2 von g mit E bestimmen
 *   - Gesuchter Punkt ist der mit kürzestem Abstand zu P
 *   - emtsprechend S1 oder S2 zrückgeben
 * Parameter:
 *   - Point P in muss in den Koordinaten der Ellipse gegeben sein
 * Rückgabewerte:
 *   - Punkt auf der Ellipse in "normalen" Koordinaten (S1 oder S2)
 * */

Point Ellipse::PointOnEllipse(const Point& P) const {
	double m, n; // y=m*x+n Steigung und y-Achsenabschnitt der Geraden
	double a, b;
	double a2, b2, m2, n2, b4; // a2 = a*a usw. damit Berechnung nur einmal
	double x1, x2;
	Point S1, S2;
	Point rueck;

	a = GetLargerAxis();
	b = GetSmallerAxis();
	a2 = a * a;
	b2 = b * b;
	b4 = b2 * b2;

	if (fabs(P.GetX() - pXp) < EPS) {// Sonderfall: Ellipsen liegen Parallel
		double y1, y2;
		y1 = sqrt(-pXp * pXp + a2) * b / a;
		y2 = -y1;
		S1 = Point(pXp, y1);
		S2 = Point(pXp, y2);
	} else { // m und n bestimmen
		m = P.GetY() / (P.GetX() - pXp); // y-Wert des ActionPoints der Ellipse ist 0
		n = -m * pXp;
		m2 = m * m;
		n2 = n * n;

		// Schnittpunkte bestimmen
		// E: b^2*x^2+a^2*y^2 = a^2*b^2
		// g: y = m*x+n
		//
		//              (-n*m+sqrt(-b^2*n^2+b^4+a^2*b^2*m^2))*a
		//    x1 =     _________________________________________
		//                           b^2+a^2*m^2
		//              (n*m+sqrt(-b^2*n^2+b^4+a^2*b^2*m^2))*a
		//    x2 =   - _________________________________________
		//                           b^2+a^2*m^2

		double tmp = sqrt(-b2 * n2 + b4 + a2 * b2 * m2);
		x1 = (-n * m * a + tmp) * a / (b2 + a2 * m2);
		x2 = (-n * m * a - tmp) * a / (b2 + a2 * m2);
		S1 = Point(x1, m * x1 + n);
		S2 = Point(x2, m * x2 + n);
	}
	// den Punkt mit kürzerem Abstand wählen
	// todo:fixme: save the squareroot computation
	//if ((P - S1).Norm() < (P - S2).Norm()) {
	if ((P - S1).NormSquare() < (P - S2).NormSquare()) {
		rueck = S1.CoordTransToCart(pCenter, pCosPhi, pSinPhi);
	} else {
		rueck = S2.CoordTransToCart(pCenter, pCosPhi, pSinPhi);
	}
	return rueck;
}

/* Berechnet den Punkt auf der Ellipse, der Schnittpunkt mit dem Liniensegment ist und den kürzesten
 * Abstand zu P hat
 * Parameter:
 *   - Line line muss in den Koordinaten der Ellipse gegeben sein
 *   - Point P muss in den Koordinaten der Ellipse gegeben sein, ist ein Punkt aus l
 * Rückgabewerte:
 *   - Punkt auf der Ellipse in Ellipsenkoordinaten
 * */
Point Ellipse::PointOnEllipse(const Line& line, const Point& P) const {
	double m, n; // y=m*x+n Steigung und y-Achsenabschnitt der Geraden
	double a, b;
	double a2, b2, m2, n2, b4; // a2 = a*a usw. damit Berechnung nur einmal
	double x1, x2;
	Point S1, S2;
	Point rueck;

	a = GetLargerAxis();
	b = GetSmallerAxis();
	a2 = a * a;
	b2 = b * b;
	b4 = b2 * b2;

	const Point& L1 = line.GetPoint1();
	const Point& L2 = line.GetPoint2();

	// Sonderfall Punkte sind bereits Punkt der Ellipse
	if (IsOn(L1))
		return L1;
	if (IsOn(L2))
		return L2;

	if (fabs(L1.GetX() - L2.GetX()) < EPS) { // parallel zur y-Achse der Ellipse, EPS_DIST zu ungenau
		double y1, y2;
		y1 = sqrt(-L2.GetX() * L2.GetX() + a2) * b / a;
		y2 = -y1;
		S1 = Point(L2.GetX(), y1);
		S2 = Point(L2.GetX(), y2);
	} else { // m und n bestimmen
		m = (L1.GetY() - L2.GetY()) / (L1.GetX() - L2.GetX());
		n = L2.GetY() - m * L2.GetX();
		m2 = m * m;
		n2 = n * n;
		// Schnittpunkte bestimmen
		// E: b^2*x^2+a^2*y^2 = a^2*b^2
		// g: y = m*x+n
		//
		//              (-n*m+sqrt(-b^2*n^2+b^4+a^2*b^2*m^2))*a
		//    x1 =     _________________________________________
		//                           b^2+a^2*m^2
		//              (n*m+sqrt(-b^2*n^2+b^4+a^2*b^2*m^2))*a
		//    x2 =   - _________________________________________
		//                           b^2+a^2*m^2

		double tmp = sqrt(-b2 * n2 + b4 + a2 * b2 * m2);
		x1 = (-n * m * a + tmp) * a / (b2 + a2 * m2);
		x2 = (-n * m * a - tmp) * a / (b2 + a2 * m2);
		S1 = Point(x1, m * x1 + n);
		S2 = Point(x2, m * x2 + n);
	}
	// den Punkt mit kürzerem Abstand wählen
	//todo: runtime normsquare?
	//if ((P - S1).Norm() < (P - S2).Norm()) {
	if ((P - S1).NormSquare() < (P - S2).NormSquare()) {
		rueck = S1;
	} else {
		rueck = S2;
	}

	return rueck;
}

/* minimal möglicher Abstand (durch Verschiebung) Ellipse <-> Segment
 * ausführliche Domkumentation siehe:
 * http://chraibi.de/sites/default/files/mindel2.pdf
 * Algorithms:
 *   - l in Koordinaten der Ellipse transformieren
 *   - Punt P mit kürzestem Abstand zu AP bestimmen
 *   - l ist Teil der Geraden g: y=m*x+n
 *   - Steigung von g, also m bestimmen
 *   - g' ist die verschobene Gerade, Verschiebungsvektor ist AP-P
 *   - Betrag der Verschienbung d = min(d1, d2) berechnen
 *   - g verschieben nach g'
 *   - gesuchter Avstand ist nun effektiver Abstand zu g'
 * Parameter:
 *   - Line l in "normalen" Koordinaten
 * Rückgabewerte:
 *   - mindist: gesuchter minimal möglicher Abstand
 * */

double Ellipse::MinimumDistanceToLine(const Line& l) const {
	// Line muss in Koordinaten der Ellipse transformiert werden
	Point AinE = l.GetPoint1().CoordTransToEllipse(pCenter, pCosPhi, pSinPhi);
	Point BinE = l.GetPoint2().CoordTransToEllipse(pCenter, pCosPhi, pSinPhi);

	// Action Point der Ellipse
	Point APinE = Point(pXp, 0);
	Line linE = Line(AinE, BinE);
	Line l_strich_inE;
	// Punkt auf line mit kürzestem Abstand zum Action Point der Ellipse
	Point PinE = linE.ShortestPoint(APinE);
	// Sonstige Punkte in Ellipsen Koordinaten
	Point A_strich_inE, B_strich_inE, SinE;

	double mindist; // Rückgabewert
	double n_strich; // neuer y_achsen-Abschnitt g': y=m*x+n'
	double sx, sy; // Koordinaten des Schnittpunktes der Ellipse mit dem verschobenen Segment

	// kürzester Punkt ist Randpunkt
	if (PinE == AinE || PinE == BinE) {
		mindist = 0;
	} else {
		// normierter Verschiebungsvector
		Point n = (APinE - PinE).Normalized();
		// Halbachsen der Ellipse
		double a = this->GetLargerAxis();
		double a2 = a * a; // um Rechenaufwand zu sparen
		double b = this->GetSmallerAxis();
		double b2 = b * b;
		// Abstand zwischen g und g'
		double d;

		// Steigung der Geraden
		if (fabs(AinE.GetX() - BinE.GetX()) < EPS_DIST) { // Gerade parallel zur y-Achse der Ellipse
			d = fabs(AinE.GetX()) - a;
			sx = sign(AinE.GetX()) * a;
			sy = 0.0;
		} else if (fabs(AinE.GetY() - BinE.GetY()) < EPS_DIST) { // Gerade parallel zur x-Achse der Ellipse
			d = fabs(AinE.GetY()) - b;
			sx = 0.0;
			sy = sign(AinE.GetY()) * b;
		} else { // Steigung der Geraden muss berechnet werden
			double m = (AinE.GetY() - BinE.GetY())
					/ (AinE.GetX() - BinE.GetX());
			// d berechnen
			double tmp1, tmp2, d1, d2;
			tmp1 = (-m * AinE.GetX() + AinE.GetY()) / (m * n.GetX() - n.GetY());
			tmp2 = sqrt(a2 * m * m + b2) / (m * n.GetX() - n.GetY());
			d1 = tmp1 + tmp2;
			d2 = tmp1 - tmp2;
			d = min(d1, d2);
			n_strich = -m * AinE.GetX() - m * d * n.GetX() + AinE.GetY() + d
					* n.GetY();
			// möglichen Schnittpunkt berechnen
			sx = (-n_strich * m * a2) / (a2 * m * m + b2);
			sy = m * sx + n_strich;
		}
		A_strich_inE = AinE + n * d; // verschobene Punkte
		B_strich_inE = BinE + n * d;
		// Schnittpunkt
		SinE = Point(sx, sy);
		// Prüfen ob S im verschobenen Liniensegment liegt
		l_strich_inE = Line(A_strich_inE, B_strich_inE);
		if (!l_strich_inE.IsInLine(SinE)) { // liegt nicht drin
			PinE = l_strich_inE.ShortestPoint(APinE);
			//TODO: runtime normquare?
			//if ((SinE - A_strich_inE).Norm() < (SinE - B_strich_inE).Norm()) { // A' ist eigentlicher Schnittpunkt
			if ((SinE - A_strich_inE).NormSquare()
					< (SinE - B_strich_inE).NormSquare()) { // A' ist eigentlicher Schnittpunkt
				Line tmpline = Line(A_strich_inE, A_strich_inE + APinE - PinE);
				A_strich_inE = PointOnEllipse(tmpline, A_strich_inE);
				B_strich_inE = BinE + A_strich_inE - AinE;
			} else { // B' ist eigentlicher Schnittpunkt
				Line tmpline = Line(B_strich_inE, B_strich_inE + APinE - PinE);
				B_strich_inE = PointOnEllipse(tmpline, B_strich_inE);
				A_strich_inE = AinE + B_strich_inE - BinE;
			}

		}
		if (IsOn(SinE)) { // Prüfen, ob S auf der Ellipse liegt
			mindist = 0;
		} else {
			Line g_strich = Line(A_strich_inE, B_strich_inE); //Verschobenes Segment
			mindist = this->EffectiveDistanceToLine(g_strich);
		}
	}

	return mindist;
}

/*
 min distance ellipses
 closest approeach between two ellipses

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
double Ellipse::MinimumDistanceToEllipse(const Ellipse& E2) const {
	Ellipse tmpE2 = E2; // verschobene Ellipse E2, so dass Berührung mit E1
	Point v; // Verschiebungsvektor
	Point tmpC2 = Point(); // verschobenes Centrum von E2
	const Point& C1 = this->GetCenter();
	const Point& C2 = E2.GetCenter();

	//double d = this->Distance2d(E2); //dist closest approach between centres
	double d = 0.5; //this->Distance2d(E2); //dist closest approach between centres
	double distance; //between c1 and c2
	double mind; // Rueckgabewert
	if (d < 0 || d != d) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() d=%f\n", d);
		Log->write(tmp);
		exit(0);
	}
	//todo: runtime normsquare?
	if ((C1 - C2).Norm() < EPS) {
		char tmp[CLENGTH];
		sprintf(tmp, "ERROR: \tEllipse::MinimumDistanceToEllipse() m=0\n"
			"xc1: %f xc2: %f yc1: %f yc2: %f\n", C1.GetX(), C2.GetX(),
				C1.GetY(), C2.GetY());
		Log->write(tmp);
		//FIXME:
		cout << "press a key: " << endl;
		//getc(stdin);
		return 0.0;
		exit(0);
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

/* prüft, ob ein Punkt sich innerhalb der Ellipse befindet
 * Parameter:
 *    Point p muss in Koordinaten der Ellipse gegeben sein
 * Rückgabewert:
 *    - true, wenn drin
 *    - false, wenn draußen
 * */bool Ellipse::IsInside(const Point& p) const {
	double a = GetLargerAxis();
	double b = GetSmallerAxis();
	double x = p.GetX();
	double y = p.GetY();

	return (x * x) / (a * a) + (y * y) / (b * b) < 1 + EPS_DIST;
}

/* prüft, ob ein Punkt sich außerhalb der Ellipse befindet
 * * Parameter:
 *    Point p muss in Koordinaten der Ellipse gegeben sein
 * Rückgabewert:
 *    - true, wenn draußen
 *    - false, wenn drin
 * */bool Ellipse::IsOutside(const Point& p) const {
	double a = GetLargerAxis();
	double b = GetSmallerAxis();
	double x = p.GetX();
	double y = p.GetY();

	return (x * x) / (a * a) + (y * y) / (b * b) > 1 - EPS_DIST;
}

/* prüft, ob ein Punkt sich außerhalb der Ellipse befindet
 * * Parameter:
 *    Point p muss in Koordinaten der Ellipse gegeben sein
 * Rückgabewert:
 *    - true, wenn auf der Ellipse
 *    - false, sonst
 * */

bool Ellipse::IsOn(const Point& p) const {
	//double a = GetLargerAxis();
	//double b = GetSmallerAxis();
	double a = GetEA();
	double b = GetEB();
	double x = p.GetX();
	double y = p.GetY();

	return (-EPS_DIST < (x * x) / (a * a) + (y * y) / (b * b) - 1) && ((x * x)
			/ (a * a) + (y * y) / (b * b) - 1 < EPS_DIST);
}

bool Ellipse::IntersectionWithLine(const Line& line) {
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
