// GeomCGeomHomog.h: interface for the CGeomHomog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOMCGeomHomog_H__EAC480E6_8D86_4BDD_83C4_DC5D2F248697__INCLUDED_)
#define AFX_GEOMCGeomHomog_H__EAC480E6_8D86_4BDD_83C4_DC5D2F248697__INCLUDED_


#include "math.h"
#include "../geometry/Point.h"


class CGeomHomog
{
public:
	CGeomHomog();
	CGeomHomog(double pw, double px, double py);
	CGeomHomog(Point p);
	CGeomHomog(int px, int py);
	CGeomHomog(const CGeomHomog& homog);
	virtual ~CGeomHomog();

protected:
	double w;
	double x;
	double y;

public:
	Point toPoint();
	void meet(CGeomHomog p);
	void meet(Point p);
	void meet(Point i, Point j);
	CGeomHomog& neg();
	double perpdot(CGeomHomog& p);
	double det(CGeomHomog p, CGeomHomog q, CGeomHomog r);
	bool ccw(CGeomHomog p, CGeomHomog q, CGeomHomog r);
	bool cw(CGeomHomog p, CGeomHomog q, CGeomHomog r);
	double dot(CGeomHomog h);
	double dot(Point p);
	bool right(Point p);
	bool left(Point p);

	CGeomHomog assign(double pw, double px, double py);
	int getX() { return (int) (x / w); } // drawing needs these
	int getY() { return (int) (-y / w); }

	CGeomHomog& add(CGeomHomog& p);

};

#endif // !defined(AFX_GEOMCGeomHomog_H__EAC480E6_8D86_4BDD_83C4_DC5D2F248697__INCLUDED_)
