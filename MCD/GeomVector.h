// GeomVector.h: interface for the CGeomVector class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CGEOMVECTOR_H_
#define _CGEOMVECTOR_H_

#include "GeomHomog.h"
#include "../geometry/Point.h"

class CGeomVector
{
public:
	CGeomHomog meet(Point i, Point j);
	bool  on(Point p, Point q, Point r);
	bool  right(Point p, Point q, Point r);
	double delta3(Point p, Point q, Point r);
	bool  left(Point p, Point q, Point r);
	double distance2(Point i, Point j);
	double distance(Point i, Point j);

	CGeomVector();
	virtual ~CGeomVector();

};

#endif // _CGEOMVECTOR_H_
