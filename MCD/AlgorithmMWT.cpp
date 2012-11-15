// AlgorithmMWT.cpp: implementation of the AlgorithmMWT class.
//
//////////////////////////////////////////////////////////////////////

#include "AlgorithmMWT.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlgorithmMWT::AlgorithmMWT(CGeomPoly *pl)
{
	int n = pl->m_nCount;
	m_nPointsCount = n;
	m_pl = pl;

	// create nxn array
	m_dWeight = new double* [n];
	for (int i = 0; i < n; i++)
	{
		m_dWeight[i] = new double[n];
		for (int j = 0; j < n; j++)
			m_dWeight[i][j] = 0;
	}

	m_nUsedEdge = new int* [n];
	for (int i = 0; i < n; i++)
	{
		m_nUsedEdge[i] = new int[n];
		for (int j = 0; j < n; j++)
			m_nUsedEdge[i][j] = 0;
	}

}

AlgorithmMWT::~AlgorithmMWT()
{
	int n = m_nPointsCount;

	for (int i = 0; i < n; i++)
	{
		delete[] m_dWeight[i];
		delete[] m_nUsedEdge[i];
	}
	delete[] m_dWeight;
	delete[] m_nUsedEdge;
}

void AlgorithmMWT::compute()
{
	CGeomHomog* m_pHomog = new CGeomHomog(1.0, 5.0, 5.0);
	CGeomHomog homog(1.0, 1.0, 1.0);
	CGeomHomog test = m_pHomog->add(homog);

	for (int l = 2; l < m_pl->m_nCount; l++)
	{
		for (int i = 0; i < m_pl->m_nCount- l; i++)
		{
			int last = NONE;
			double w = INFINITY;
			for (int j = 1; j < l; j++)
			{
				if ((w > getWeight(i, i + j) + getWeight(i + j, l + i)) &&
					m_pVector->left(m_pl->m_pList[i], m_pl->m_pList[i + j], m_pl->m_pList[i + l]))
				{
					w = getWeight(i, i + j) + getWeight(i + j, l + i);
					last = i + j;
				}
			}
			setWeight(i, i + l, w + m_pVector->distance(m_pl->m_pList[i], m_pl->m_pList[i + l]));
			setTriangle(i, last, i + l);
		}
	}

	delete m_pHomog;
}

void AlgorithmMWT::setTriangle(int i, int j, int k)
{
	m_nUsedEdge[i][k] = j;
}

void AlgorithmMWT::setWeight(int i, int k, double w)
{
	m_dWeight[i][k] = w;
}

double AlgorithmMWT::getWeight(int i, int l)
{
	return m_dWeight[i][l];
}

//void AlgorithmMWT::Draw(CDC *pDC)
//{
//	CPen penPoint, penPoly;
//	penPoint.CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
//	penPoly.CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
//
//	CPen* pOldPen;
//	pOldPen = (CPen*)pDC->SelectObject(&penPoly);
//
//	drawLine(pDC, 0, m_pl->m_nCount - 1);
//
//	pDC->SelectObject(pOldPen);
//
//	penPoint.DeleteObject();
//	penPoly.DeleteObject();
//}
//
//void AlgorithmMWT::drawLine(CDC *pDC, int i, int j)
//{
//    if (i != j + 1 && j != i + 1 && !(i == 0 && j == m_pl->m_nCount - 1))
//	{
//		if (m_bAnimating)
//			::Sleep(200);
//		pDC->MoveTo(m_pl->m_pList[i]);
//		pDC->LineTo(m_pl->m_pList[j]);
//	}
//
//	if (m_nUsedEdge[i][j] != NONE)
//	{
//		drawLine(pDC, i, m_nUsedEdge[i][j]);
//		drawLine(pDC, m_nUsedEdge[i][j], j);
//	}
//}
//
