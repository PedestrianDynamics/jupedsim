// GeomPoly.h: interface for the CGeomPoly class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CGEOMPOLY_H_
#define _CGEOMPOLY_H_

#include "GeomVector.h"

const int SIZE_MAX=1000;

class CGeomPoly {
public:
	CGeomPoly();
	virtual ~CGeomPoly();

	Point& p(int i);
	int firstReflex(int i);
	bool IsReflex(int i);
	int firstReflex();
	int nextReflex(int i);
	void computeReflex();
	void changeOrder();
	void convertCCWToCW();
	void convertCWToCCW();
	bool checkCW();
	int mostLeftPoint();
	bool isCCW();
	bool isCW();

public:
	//for storing point list.
	Point m_pCurrent;
	Point m_pPrevious;
	Point m_pList[SIZE_MAX];
	int m_nCount;
	bool m_bIsReflex[SIZE_MAX];
	int m_nNextReflex[SIZE_MAX];
	int m_nFirstReflex;

	CGeomVector* m_pVector;

};

#endif // _CGEOMPOLY_H_
