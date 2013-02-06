// AlgorithmMCD.cpp: implementation of the AlgorithmMCD class.
//
//////////////////////////////////////////////////////////////////////

#include "cstdlib"
#include "climits"
#include <stdio.h>
#include "AlgorithmMCD.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlgorithmMCD::AlgorithmMCD(CGeomPoly *pl)
 {
	int n = pl->m_nCount;
	m_nPointsCount = n;
	m_pl = pl;
	m_bIsReflex = new bool[n];
	m_nWeight = NULL;
	m_nReflex = NULL;
	m_pPD = NULL;

	BAD = 999990;
	NONE = 0;
	m_nGuard = 0;
}

AlgorithmMCD::~AlgorithmMCD()
{
	int n = m_nPointsCount;
	for (int i = 0; i < n; i++)
	{
		delete[] m_nWeight[i];
	}
	for (int i = 0; i < n; i++)
	{
		delete[] m_pPD[i];
	}
	delete[] m_nWeight;
	delete[] m_pPD;
	delete[] m_nReflex;
	delete[] m_bIsReflex;
}

void AlgorithmMCD::compute()
{
	int n = m_pl->m_nCount;
	int i=0;
	int j=0;
	int k=0;
	int l=0;

	init();

	for (l = 3; l < m_pl->m_nCount; l++)
	{
		for (i = m_pl->firstReflex(); i + l < n; i = m_pl->nextReflex(i))
		{
			k = i + l;
			if (visible(i, k))
			{
				initPairDeque(i, k);
				if (m_pl->IsReflex(k))
				{
					for (j = i + 1; j < k; j++)
						typeA(i, j, k);
				}
				else
				{
					for (j = m_pl->firstReflex(i + 1); j < k - 1; j = m_pl->nextReflex(j))
						typeA(i, j, k);
					typeA(i, k - 1, k);
				}
			}
		}
		for (k = m_pl->firstReflex(l); k < n; k = m_pl->nextReflex(k))
		{
			i = k - l;
			if ((!m_pl->IsReflex(i)) && visible(i, k))
			{
				initPairDeque(i, k);
				typeB(i, i + 1, k);
				for (int j = m_pl->firstReflex(i + 2); j < k; j = m_pl->nextReflex(j))
					typeB(i, j, k);
			}
		}
	}
	m_nGuard = 3 * n;
	recoverSolution(0, n - 1);
}

void AlgorithmMCD::init()
{
	m_pl->computeReflex();
	initSubDecomp();
	initVisibility();
	initSubproblems();
}

void AlgorithmMCD::initSubDecomp()
{
	int n = m_pl->m_nCount;
	int r = 0;
	m_nReflex = new int[n];

	for (int i = 0; i < n; i++)
	{
		if (m_pl->m_bIsReflex[i])
			m_nReflex[i] = r++;
	}
	int j = r;
	for (int i = 0; i < n; i++)
	{
		if (!m_pl->m_bIsReflex[i])
			m_nReflex[i] = j++;
	}

	m_nWeight = new int* [n];
	m_pPD = new CGeomPairDeque* [n];
	int temp;
	for (int i = 0; i < n; i++)
	{
		temp = n;
//		temp  = m_pl->m_bIsReflex[i]? n : r;
		m_nWeight[i] = new int[temp];
		m_pPD[i] = new CGeomPairDeque[temp];
		for (j = 0; j < temp; j++)
		{
			m_nWeight[i][j] = BAD;
			m_pPD[i][j].init(4);
		}
	}
}

void AlgorithmMCD::initVisibility()
{
	AlgorithmVP m_pVP(m_pl);
	int n = m_pl->m_nCount;
	int j;
	for (int i = m_pl->firstReflex(); i < n; i = m_pl->nextReflex(i))
	{
		m_pVP.init(i);
		m_pVP.build(i);
		while (!m_pVP.empty())
		{
			j = m_pVP.popVisible() % n;
			if (j < i)
				setWeight(j, i, INT_MAX);
			else
				setWeight(i, j, INT_MAX);
		}
	}
}

void AlgorithmMCD::initSubproblems()
{
	int n = m_pl->m_nCount;
	int i = m_pl->firstReflex();
	if (i == 0)
	{
		setAfter(i);
		i = m_pl->nextReflex(i);
	}
	if(i == 1)
	{
		setWeight(0, 1, 0);
		setAfter(i);
		i = m_pl->nextReflex(i);
	}
	while (i < n - 2)
	{
		setBefore(i);
		setAfter(i);
		i = m_pl->nextReflex(i);
	}
	if (i == n - 2)
	{
		setBefore(i);
		setWeight(i, i + 1, 0);
		i = m_pl->nextReflex(i);
	}
	if (i == n - 1)
	{
		setBefore(i);
	}
}

void AlgorithmMCD::setAfter(int i)
{
	setWeight(i, i + 1, 0);
	if (visible(i, i + 2))
		initPairDeque(i, i + 2, 0, i + 1, i + 1);
}

void AlgorithmMCD::setBefore(int i)
{
	setWeight(i - 1, i, 0);
	if (visible(i - 2, i))
		initPairDeque(i - 2, i, 0, i - 1, i - 1);
}

bool AlgorithmMCD::visible(int i, int j)
{
	return weight(i, j) < BAD;
}

void AlgorithmMCD::setWeight(int i, int j, int w)
{
	m_nWeight[i][m_nReflex[j]] = w;
}

int AlgorithmMCD::weight(int i, int j)
{
	return m_nWeight[i][m_nReflex[j]];
}

CGeomPairDeque& AlgorithmMCD::pairs(int i, int j)
{
	return m_pPD[i][m_nReflex[j]];
}

CGeomPairDeque& AlgorithmMCD::initPairDeque(int i, int j)
{
//	CGeomPairDeque* pd = new CGeomPairDeque();
//	m_pPD[i][m_nReflex[j]] = *pd;
	return m_pPD[i][m_nReflex[j]];
}

void AlgorithmMCD::initPairDeque(int i, int j, int w, int a, int b)
{
	setWeight(i, j, w);
	m_pPD[i][m_nReflex[j]].push(a, b);
}

// i is reflex, use j and k
void AlgorithmMCD::typeA(int i, int j, int k)
{
	if (!visible(i, j))
		return;
	int top = j;
	int w = weight(i, j);
	if (k - j > 1)
	{
		if (!visible(j, k))
			return;
		w += weight(j, k) + 1;
	}
	//check whether use ij pair
	if (j - i > 1)
	{
//		CGeomPairDeque pair = pairs(i, j);
		if (!m_pVector->left(m_pl->p(k), m_pl->p(j), m_pl->p(pairs(i, j).bB())))
		{
			while (pairs(i, j).moreB() && !m_pVector->left(m_pl->p(k), m_pl->p(j), m_pl->p(pairs(i, j).underBB())))
				pairs(i, j).popB();
			if ((!pairs(i, j).emptyB()) && !m_pVector->right(m_pl->p(k), m_pl->p(i), m_pl->p(pairs(i, j).aB())))
				top = pairs(i, j).aB();
			else
				w++;
		}
		else
			w++;
	}
	update(i, k, w, top, j);
}

void AlgorithmMCD::typeB(int i, int j, int k)
{
	if (!visible(j, k))
		return;
	int top = j;
	int w = weight(j, k);
	if (j - i > 1)
	{
		if (!visible(i, j))
			return;
		w += weight(i, j) + 1;
	}
	if (k - j > 1)
	{
//		CGeomPairDeque pair = pairs(j, k);
		if (!m_pVector->right(m_pl->p(i), m_pl->p(j), m_pl->p(pairs(j, k).aF())))
		{
			while (pairs(j, k).more() && !m_pVector->right(m_pl->p(i), m_pl->p(j), m_pl->p(pairs(j, k).underAF())))
				pairs(j, k).pop();
			if ((!pairs(j, k).empty()) && !m_pVector->right(m_pl->p(i), m_pl->p(k), m_pl->p(pairs(j, k).bF())))
				top = pairs(j, k).bF();
			else
				w++;
		}
		else
			w++;
	}
	update(i, k, w, j, top);
}

void AlgorithmMCD::update(int a, int b, int w, int i, int j)
{
	int ow = weight(a, b);
	if (w <= ow)
	{
//		CGeomPairDeque pair = pairs(a, b);
		if (w < ow)
		{
			pairs(a, b).flush();
			setWeight(a, b, w);
		}
		pairs(a, b).pushNarrow(i, j);
	}
}

void AlgorithmMCD::recoverSolution(int i, int k)
{
	int j;
	// Error
	if (m_nGuard-- < 0)
		return;
	if (k - i <= 1)
		return;
	//CGeomPairDeque pair = pairs(i, k);
	if (m_pl->IsReflex(i))
	{
		j = pairs(i, k).bB();
		recoverSolution(j, k);
		if (j - i > 1)
		{
			if (pairs(i, k).aB() != pairs(i, k).bB())
			{
				//CGeomPairDeque pd = pairs(i, j);
				pairs(i, j).restore();
				while (!pairs(i, j).emptyB() && pairs(i, k).aB() != pairs(i, j).aB())
					pairs(i, j).popB();
			}
			recoverSolution(i, j);
		}
	}
	else
	{
		j = pairs(i, k).aF();
		recoverSolution(i, j);
		if (k - j > 1)
		{
			if (pairs(i, k).aF() != pairs(i, k).bF())
			{
				//CGeomPairDeque pd = pairs(j, k);
				pairs(j, k).restore();
				while (!pairs(j, k).empty() && pairs(i, k).bF() != pairs(j, k).bF())
					pairs(j, k).pop();
			}
			recoverSolution(j, k);
		}
	}
}

void AlgorithmMCD::Draw( )
{
	//drawDiagnal(0, m_pl->m_nCount - 1);
	drawSubPoly(0, m_pl->m_nCount - 1);
}


void AlgorithmMCD::drawDiagnal(int i, int k)
{
	printf(" New Edge [%d] -- > [%d]\n",i,k);
	//getc(stdin);
}

void AlgorithmMCD::drawSubPoly(int i, int k)
{
	int j;
	bool  ijreal = true;
	bool  jkreal = true;
	if (k - i <= 1)
		return;

	if (m_pl->IsReflex(i))
	{
		j = pairs(i, k).bB();
		ijreal = (pairs(i, k).aB() == pairs(i, k).bB());
	}
	else
	{
		j = pairs(i, k).aF();
		jkreal = (pairs(i, k).bF() == pairs(i, k).aF());
	}
	if (ijreal)
		drawDiagnal(i, j);
	if (jkreal)
		drawDiagnal(j, k);

	drawSubPoly(i, j);
	drawSubPoly(j, k);
}
