/**
 * File:   Line.cpp
 *
 * Created on 30. September 2010, 09:40
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

#include "Line.h"

/************************************************************
  Konstruktoren
 ************************************************************/
Line::Line() {
	SetPoint1(Point()); //Default-Constructor  (0.0,0.0)
	SetPoint2(Point());
}

Line::Line(const Point& p1, const Point& p2) {
	SetPoint1(p1);
	SetPoint2(p2);
}

Line::Line(const Line& orig) {
	pPoint1 = orig.GetPoint1();
	pPoint2 = orig.GetPoint2();
	pCentre = orig.GetCentre();
}

Line::~Line() {
}

/*************************************************************
 Setter-Funktionen
 ************************************************************/
void Line::SetPoint1(const Point& p) {
	pPoint1 = p;
}

void Line::SetPoint2(const Point& p) {
	pPoint2 = p;
	pCentre = (pPoint1+pPoint2)*0.5;
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/
const Point& Line::GetPoint1() const {
	return pPoint1;
}

const Point& Line::GetPoint2() const {
	return pPoint2;
}

const Point& Line::GetCentre()const {
	return pCentre;
}

/*************************************************************
 Ausgabe
 ************************************************************/
string Line::Write() const {
	string geometry;
	char wall[500] = "";
	geometry.append("\t\t<wall color=\"100\">\n");
	sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
			(GetPoint1().GetX()) * FAKTOR,
			(GetPoint1().GetY()) * FAKTOR);
	geometry.append(wall);
	sprintf(wall, "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\"/>\n",
			(GetPoint2().GetX()) * FAKTOR,
			(GetPoint2().GetY()) * FAKTOR);
	geometry.append(wall);
	geometry.append("\t\t</wall>\n");
	return geometry;
}


/*************************************************************
 Sonstige Funktionen
 ************************************************************/
// Normalen vector zur Linie

Point Line::NormalVec() const {
	double nx, ny, norm;
	Point r = GetPoint2() - GetPoint1();

	if (r.GetX() == 0.0) {
		nx = 1;
		ny = 0;
	} else {
		nx = -r.GetY() / r.GetX();
		ny = 1;
		/* Normieren */
		norm = sqrt(nx * nx + ny * ny);
		if (fabs(norm) < EPS) {
			Log->write("ERROR: \tLine::NormalVec() norm==0\n");
			exit(0);
		}
		nx /= norm;
		ny /= norm;
	}
	return Point(nx, ny);
}

// Normale Komponente von v auf l

double Line::NormalComp(const Point& v) const {
	// Normierte Vectoren
	Point l = (GetPoint2() - GetPoint1()).Normalized();
	const Point& n = NormalVec();


	double lx = l.GetX();
	double ly = l.GetY();
	double nx = n.GetX();
	double ny = n.GetY();
	double alpha;

	if (fabs(lx) < EPS) {
		alpha = v.GetX() / nx;
	} else if (fabs(ly) < EPS) {
		alpha = v.GetY() / ny;
	} else {
		alpha = (v.GetY() * lx - v.GetX() * ly) / (nx * ly - ny * lx);
	}

	return fabs(alpha);
}
// Lotfußpunkt zur Geraden Line
// Muss nicht im Segment liegen

Point Line::LotPoint(const Point& p) const {
	//TODO: reference vs copy??
	const Point& r = GetPoint1();
	const Point& s = GetPoint2();
	const Point& t = r - s;
	Point tmp;
	double lambda;

	tmp = p - s;
	lambda = tmp.ScalarP(t) / t.ScalarP(t);
	Point f = s + t*lambda;
	return f;
}

/* Punkt auf der Linie mit kürzestem Abstand zu p
 * In der Regel Lotfußpunkt, Ist der Lotfußpunkt nicht im Segment
 * wird der entsprechende Eckpunkt der Line genommen
 * */
Point Line::ShortestPoint(const Point& p) const {

	const Point& t = pPoint1 - pPoint2;

	Point tmp = p - pPoint2;
	double lambda = tmp.ScalarP(t) / t.ScalarP(t);
	Point f = pPoint2 + t*lambda;

	/* Prüfen ob Punkt in der Linie,sonst entsprechenden Eckpunkt zurückgeben */
	if (lambda < 0)
		f = pPoint2;
	if (lambda > 1)
		f = pPoint1;

	return f;
}

/* Prüft, ob Punkt p im Liniensegment enthalten ist
 * Verfahren wie bei Line::ShortestPoint(), d. h,
 * lambda berechnen und prüfen ob zwischen 0 und 1
 * */
bool Line::IsInLine(const Point& p) const {
	double ax, ay, bx, by, px, py;
	const Point& a = GetPoint1();
	const Point& b = GetPoint2();
	double lambda;
	ax = a.GetX();
	ay = a.GetY();
	bx = b.GetX();
	by = b.GetY();
	px = p.GetX();
	py = p.GetY();
	if (fabs(ax - bx) > EPS_DIST) {
		lambda = (px - ax) / (bx - ax);
	} else if (fabs(ay - by) > EPS_DIST) {
		lambda = (py - ay) / (by - ay);
	} else {
		Log->write("ERROR: \tIsInLine: Endpunkt = Startpunkt!!!");
		exit(0);
	}
	return (0 <= lambda) && (lambda <= 1);
}

/*
 *  Prüft, ob Punkt p im Liniensegment enthalten ist
 * algorithm from:
 * http://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
 * */
bool Line::IsInLineSegment(const Point& p) const {
    double ax, ay, bx, by, px, py, crossp, dotp, lengthsq;
	const Point& a = GetPoint1();
	const Point& b = GetPoint2();
	double lambda;
	ax = a.GetX();
	ay = a.GetY();
	bx = b.GetX();
	by = b.GetY();
	px = p.GetX();
	py = p.GetY();
	
	// cross product to check if point i colinear
	crossp = (py-ay)*(bx-ax)-(px-ax)*(by-ay);
	if(fabs(crossp) > EPS) return false;
	
	// dotproduct and distSquared to check if point is in segment and not just in line
	dotp = (px-ax)*(bx-ax)+(py-ay)*(by-ay);
	if(dotp < 0 || (a-b).NormSquare() < dotp) return false;
	
	return true;
	
}

/* Berechnet direkt den Abstand von p zum Segment l
 * dazu wird die Funktion Line::ShortestPoint()
 * benuzt
 * */
double Line::DistTo(const Point& p) const {
	return (p - ShortestPoint(p)).Norm();
}

double Line::DistToSquare(const Point& p) const {
	return (p - ShortestPoint(p)).NormSquare();
}

/* Zwei Linien sind gleich, wenn ihre beiden Punkte
 * gleich sind
 * */
bool Line::operator==(const Line& l) const {
	return ((pPoint1 == l.GetPoint1() && pPoint2 == l.GetPoint2()) ||
			(pPoint2 == l.GetPoint1() && pPoint1 == l.GetPoint2()));
}

/* Zwei Linien sind ungleich, wenn ihre beiden Punkte
 * ungleich sind
 * */
bool Line::operator!=(const Line& l) const {
	return ((pPoint1 != l.GetPoint1() && pPoint2 != l.GetPoint2()) &&
			(pPoint2 != l.GetPoint1() && pPoint1 != l.GetPoint2()));
}

double Line::Length() const {
	return (pPoint1 - pPoint2).Norm();
}

double Line::LengthSquare() const {
	return (pPoint1 - pPoint2).NormSquare();
}

bool Line::IntersectionWith(const Line& l) const {

	double deltaACy = this->pPoint1.GetY() - l.GetPoint1().GetY();
	double deltaDCx = l.GetPoint2().GetX() - l.GetPoint1().GetX();
	double deltaACx = this->pPoint1.GetX() - l.GetPoint1().GetX();
	double deltaDCy = l.GetPoint2().GetY() - l.GetPoint1().GetY();
	double deltaBAx = this->pPoint2.GetX() - this->pPoint1.GetX();
	double deltaBAy = this->pPoint2.GetY() - this->pPoint1.GetY();

	double denominator = deltaBAx * deltaDCy - deltaBAy * deltaDCx;
	double numerator = deltaACy * deltaDCx - deltaACx * deltaDCy;

	if (denominator == 0) {
		if (numerator == 0) {
			// collinear. Potentially infinite intersection points.
			// Check and return one of them.
			if (pPoint1.GetX() >= l.GetPoint1().GetX() && pPoint1.GetX() <= l.GetPoint2().GetX()) {
				//return AB.Start;
				return true;
			} else if (l.GetPoint1().GetX() >= pPoint1.GetX() && l.GetPoint1().GetX() <= pPoint2.GetX()) {
				//return CD.Start;
				return true;
			} else {
				return false;
			}
		} else { // parallel
			return false;
		}
	}

	double r = numerator / denominator;
	if (r < 0 || r > 1) {
		return false;
	}

	double s = (deltaACy * deltaBAx - deltaACx * deltaBAy) / denominator;
	if (s < 0 || s > 1) {
		return false;
	}

	//return new PointF ((float) (AB.Start.X + r * deltaBAx), (float) (AB.Start.Y + r * deltaBAy));
	return true;
}

bool Line::IntersectionWithCircle(const Point& centre, double radius /*cm for pedestrians*/){

	double r=radius;
	double x1=pPoint1.GetX();
	double y1=pPoint1.GetY();

	double x2=pPoint2.GetX();
	double y2=pPoint2.GetY();

	double xc=centre.GetX();
	double yc=centre.GetY();

	//this formula assumes that the circle is centered the origin.
	// so we translate the complete stuff such that the circle ends up at the origin
	x1=x1-xc;y1=y1-yc;
	x2=x2-xc;y2=y2-yc;
	//xc=xc-xc;yc=yc-yc; to make it perfect

	// we first check the intersection of the circle and the  infinite line defined by the segment
	double dr2=((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	double D2=(x1*y2-x2*y1)*(x1*y2-x2*y1);
	double r2=radius*radius;

	double delta=r2*dr2-D2;
	if(delta<=0.0) return false;


	double a=(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
	double b=2*((x1*(x2-x1))+y1*(y2-y1));
	double c=x1*x1+y1*y1-r*r;

	delta=b*b-4*a*c;

	if((x1==x2)&&(y1==y2)){
		Log->write("isLineCrossingCircle: Your line is a point");
		return false;
	}
	if(delta<0.0){
		char tmp[CLENGTH];
		sprintf(tmp,"there is a bug in 'isLineCrossingCircle', delta(%f) can t be <0 at this point.",delta);
		Log->write(tmp);
		Log->write("press ENTER");
		return false; //fixme
		//getc(stdin);
	}

	double t1= (-b + sqrt(delta))/(2*a);
	double t2= (-b - sqrt(delta))/(2*a);
	if((t1<0.0) || (t1>1.0)) return false;
	if((t2<0.0) || (t2>1.0)) return false;
	return true;
}
