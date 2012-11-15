// GeomPoly.cpp: implementation of the CGeomPoly class.
//
//////////////////////////////////////////////////////////////////////

#include "GeomPoly.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeomPoly::CGeomPoly()
{
	m_nCount = 0;
	m_pVector = new CGeomVector();
	m_nFirstReflex = 0;
	m_nFirstReflex=0;
	m_nCount=0;

	m_pCurrent=Point(0,0);
	m_pPrevious=Point(0,0);

	for(int i=0;i<SIZE_MAX;i++){
		m_pList[i]=Point(0,0);
		m_bIsReflex[i]=false;
		m_nNextReflex[i]=0;
	}


}

CGeomPoly::~CGeomPoly()
{
	delete m_pVector;
}

int CGeomPoly::mostLeftPoint()
{
	int leftPoint = 0;
	long x = m_pList[0].pX;
	for (int i = 1; i < m_nCount; i++)
	{
		if (m_pList[i].pX < x)
		{
			leftPoint = i;
			x = m_pList[i].pX;
		}
	}
	return leftPoint;
}

bool  CGeomPoly::checkCW()
{
	int j = mostLeftPoint();
	int i = (j - 1 + m_nCount) % m_nCount;
	int k = (j + 1) % m_nCount;
	return m_pVector->left(m_pList[i], m_pList[j], m_pList[k]);
}

bool  CGeomPoly::isCW()
{
	return checkCW();
}

bool  CGeomPoly::isCCW()
{
	return !checkCW();
}

void CGeomPoly::changeOrder()
{
	Point temp;
	for (int i = 0; i < (m_nCount - 1) / 2; i++)
	{
		temp = m_pList[i + 1];
		m_pList[i + 1] = m_pList[m_nCount - i - 1];
		m_pList[m_nCount - i - 1] = temp;
	}
}

void CGeomPoly::convertCWToCCW()
{
	if (isCCW())
		return;
	else
		changeOrder();
}

void CGeomPoly::convertCCWToCW()
{
	if (isCW())
		return;
	else
		changeOrder();
}

void CGeomPoly::computeReflex()
{
	int wrap = 0;
	m_bIsReflex[wrap] = true;  // convention
	for (int i = m_nCount - 1; i > 0; i--)
	{
		m_bIsReflex[i] = m_pVector->right(m_pList[i - 1], m_pList[i], m_pList[wrap]);
		wrap = i;
	}

	// set next reflex
	m_nFirstReflex = m_nCount;
	for (int i = m_nCount - 1; i >= 0; i--)
	{
		m_nNextReflex[i] = m_nFirstReflex;
		if (m_bIsReflex[i])
			m_nFirstReflex = i;
	}
}

int CGeomPoly::nextReflex(int i)
{
	return m_nNextReflex[i];
}

int CGeomPoly::firstReflex()
{
	return m_nFirstReflex;
}

int CGeomPoly::firstReflex(int i)
{
	if (i <= 0)
		return m_nFirstReflex;
	if (i > m_nCount)
		return m_nCount;
	return m_nNextReflex[i - 1];
}

bool  CGeomPoly::IsReflex(int i)
{
	return m_bIsReflex[i];
}


Point& CGeomPoly::p(int i)
{
	return m_pList[i % m_nCount];
}
