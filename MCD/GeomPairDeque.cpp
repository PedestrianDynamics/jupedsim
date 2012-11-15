// GeomPairDeque.cpp: implementation of the CGeomPairDeque class.
//
//////////////////////////////////////////////////////////////////////

#include "GeomPairDeque.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeomPairDeque::CGeomPairDeque()
{
	CGeomPairDeque(4);
}

CGeomPairDeque::CGeomPairDeque(int n)
{
	m_nLength = n;
	m_nA = new int[n];
	m_nB = new int[n];
	for (int i = 0; i < n; i++)
	{
		m_nA[i] = -1;
		m_nB[i] = -1;
	}
	m_nLast = -1;
	m_nFp = -1;
	m_nBp = 0;
}

CGeomPairDeque::CGeomPairDeque(CGeomPairDeque &pd)
{
	this->m_nLast = pd.m_nLast;
	this->m_nBp = pd.m_nBp;
	this->m_nFp = pd.m_nFp;
	this->m_nLength = pd.m_nLength;

	this->m_nA = new int[m_nLength];
	this->m_nA = new int[m_nLength];
	for (int i = 0; i < m_nLength; i++)
	{
		m_nA[i] = pd.m_nA[i];
		m_nB[i] = pd.m_nB[i];
	}
}

CGeomPairDeque::~CGeomPairDeque()
{
	delete[] m_nA;
	delete[] m_nB;
}

void CGeomPairDeque::pushNarrow(int i, int j)
{
	if ((!empty()) && (i <= aF()))
		return;
	while ((!empty()) && (bF() >= j))
		pop();
	push(i, j);
}

void CGeomPairDeque::pop()
{
	m_nFp--;
}

void CGeomPairDeque::push(int i, int j)
{
	if (m_nLength <= ++m_nFp)
	{
		grow(m_nA, 2);
		grow(m_nB, 2);
	}
	m_nA[m_nFp] = i;
	m_nB[m_nFp] = j;
	m_nLast = m_nFp;
	m_nLength++;
}

void CGeomPairDeque::grow(int a[], int factor)
{
	int* temp = a;
	a = new int[m_nLength * factor];
	for (int i = 0; i < m_nLength; i++)
		a[i] = temp[i];
	for (int i = m_nLength; i < m_nLength * factor; i++)
		a[i] = -1;
	m_nLength *= 2;
	delete temp;
}

bool CGeomPairDeque::empty()
{
	return m_nFp < 0;
}

bool CGeomPairDeque::more()
{
	return m_nFp > 0;
}

int CGeomPairDeque::aF()
{
	return m_nA[m_nFp];
}

int CGeomPairDeque::bF()
{
	return m_nB[m_nFp];
}

int CGeomPairDeque::underAF()
{
	return m_nA[m_nFp - 1];
}

bool CGeomPairDeque::emptyB()
{
	return m_nBp > m_nLast;
}

bool CGeomPairDeque::moreB()
{
	return m_nBp < m_nLast;
}

int CGeomPairDeque::aB()
{
	return m_nA[m_nBp];
}

int CGeomPairDeque::bB()
{
	return m_nB[m_nBp];
}

int CGeomPairDeque::underBB()
{
	return m_nB[m_nBp + 1];
}

void CGeomPairDeque::popB()
{
	m_nBp++;
}

void CGeomPairDeque::flush()
{
	m_nLast = -1;
	m_nFp = -1;
}

void CGeomPairDeque::restore()
{
	m_nBp = 0;
	m_nFp = m_nLast;
}

void CGeomPairDeque::init(int n)
{
	m_nLength = n;
	m_nA = new int[n];
	m_nB = new int[n];
	m_nLast = -1;
	m_nFp = -1;
	m_nBp = 0;

	for(int i=0;i<n;i++)
	{
		m_nA[i] = 0;
		m_nB[i] = 0;
	}
}

void CGeomPairDeque::init(CGeomPairDeque &pd)
{
	m_nLength = pd.m_nLength;
	m_nA = new int[m_nLength];
	m_nB = new int[m_nLength];
	for (int i = 0; i < m_nLength; i++)
	{
		m_nA[i] = pd.m_nA[i];
		m_nB[i] = pd.m_nB[i];
	}
	m_nLast = pd.m_nLast;
	m_nFp = pd.m_nFp;
	m_nBp = pd.m_nBp;
}
