// AlgorithmVP.cpp: implementation of the AlgorithmVP class.
//
//////////////////////////////////////////////////////////////////////

#include "AlgorithmVP.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlgorithmVP::AlgorithmVP(CGeomPoly* pl)
{
	int n = pl->m_nCount;
	m_nPointsCount = n;
	m_nTop = -1;
	m_pl  = pl;
	m_sVP = new int[n];
	m_sType = new int[n];
	m_pOrigin = Point(0,0);
	m_nResult = new int* [n];
	for (int j = 0; j < n; j++)
		m_nResult[j] = new int[n + 1];
	m_nType = new int* [n];
	for (int j = 0; j < n; j++)
		m_nType[j] = new int[n + 1];

	RLID = 0;
	LLID = 1;
	RWALL = 2;
	LWALL = 3;
	NOTSAVED = -1;
}

AlgorithmVP::~AlgorithmVP()
{
	int n = m_nPointsCount;
	for (int i = 0; i < n; i++)
	{
		delete[] m_nResult[i];
		delete[] m_nType[i];
	}
	delete[] m_nResult;
	delete[] m_nType;
	delete[] m_sType;
	delete[] m_sVP;
}

void AlgorithmVP::compute()
{
	for (int i = 0; i < m_pl->m_nCount; i++)
	{
		init(i);
		build(i);
		store(i);
	}
}

void AlgorithmVP::init(int i)
{
	m_pOrigin = m_pl->p(i);
	m_nTop = -1;
}

void AlgorithmVP::build(int orgIndex)
{
	CGeomHomog edgej;		// during the loops, this is the line p(j-1)->p(j)

	//FIXME: check the correct use of infinity here
	//CGeomHomog INFINITY(1.0, 0.0, 0.0);
	CGeomHomog undendlich(1.0, 0.0, 0.0);

	int j = orgIndex;
	push(j++, RWALL);	// assume the first vertex is visible.
	do {			// loop always pushes pj and increments j.
	    push(j++, RWALL);
	    if (j >= m_pl->m_nCount + orgIndex)  // finish all vertices, done.
			return;
	    edgej.meet(m_pl->p(j - 1), m_pl->p(j));
	    if (edgej.left(m_pOrigin))  // easiest case: add edge to VP.
			continue;
    	// else pj backtracks, we must determine where
	    if (!edgej.left(m_pl->p(j - 2))) // pj is above last VP edge
		{
			j = exitRBay(j, top(), undendlich);
			push(j++, RLID);  // exits bay; push next two
			continue;
	    }
    
	    saveLid();		// else pj below top edge; becomes lid or pops
	    do 	// pj hides some of VP; break loop when can push pj.
		{
			if (m_pVector->left(m_pOrigin, top(), m_pl->p(j))) // saved lid ok so far...
			{
				if (m_pVector->right(m_pl->p(j), m_pl->p(j+1), m_pOrigin)) // continue to hide
					j++;
				else if (edgej.left(m_pl->p(j+1)))  // or turns up into bay
				{
					j = exitLBay(j, m_pl->p(j), m_pVector->meet(m_pl->p(LLidIdx), m_pl->p(LLidIdx-1))) + 1; 
				}
				else  // or turns down; put saved lid back & add new VP edge 
				{
					restoreLid();
					push(j++, LWALL);
					break;
				}
				edgej.meet(m_pl->p(j - 1), m_pl->p(j)); // loop continues with new j; update edgej
			}
			else	// lid is no longer visible
				if (!edgej.left(top())) // entered RBay, must get out
				{
//					assert((RLidIdx != NOTSAVED), "no RLid saved " +LLidIdx+RLidIdx+toString());
					j = exitRBay(j, top(), edgej.neg()); // exits bay;
					push(j++, RLID);   // found new visible lid to add to VP.
					break;
				}
				else
					saveLid(); // save new lid from VP; continue to hide VP.
		} while (true);
	} while (j < m_pl->m_nCount + orgIndex); // don't push origin again.
}

Point& AlgorithmVP::top()
{
	return m_pl->p(m_sVP[m_nTop]);
}

void AlgorithmVP::push(int index, int type)
{
	m_sVP[++m_nTop] = index;
	m_sType[m_nTop] = type;
}

Point& AlgorithmVP::nextTop()
{
	return m_pl->p(m_sVP[m_nTop - 1]);
}

bool AlgorithmVP::empty()
{
	return m_nTop < 0;
}

int AlgorithmVP::popVisible() 
{ 
	while ((m_sType[m_nTop] == RLID) || (m_sType[m_nTop] == LLID)) m_nTop--;
	return m_sVP[m_nTop--]; 
}
    
/** exit a bay: proceed from j, j++, .. until exiting the bay defined
    to the right (or left for exitLBay) of the line from m_pOrigin through
    point bot to line lid.  Return j such that (j,j+1) forms new lid
    of this bay.  Assumes that pl.p(j) is not left (right) of the line
    m_pOrigin->bot.  
*/
int AlgorithmVP::exitRBay(int j, Point bot, const CGeomHomog& lid)
{
	int wn = 0;		// winding number
	CGeomHomog mouth = m_pVector->meet(m_pOrigin, bot);
	bool lastLeft, currLeft = false;
	while (++j < 3 * m_pl->m_nCount) 
	{
	    lastLeft = currLeft;
	    currLeft = mouth.left(m_pl->p(j));
		// make sure that vertex does not cross ray m_pOrigin->bot at the m_pOrigin side
	    if ((currLeft != lastLeft) 
			&& (m_pVector->left(m_pl->p(j - 1), m_pl->p(j), m_pOrigin) == currLeft)) 
		{ 
			// If cross ray m_pOrigin->bot, update wn
			if (!currLeft) // cross backward
				wn--;
			else if (wn++ == 0) //cross forward
			{ // on 0->1 transitions, check window
				// cross ray m_pOrigin->bot at the outside of the bot
				CGeomHomog edge = m_pVector->meet(m_pl->p(j - 1), m_pl->p(j));
				if(edge.left(bot) && !edge.cw(mouth, edge, lid))
					return j-1;
			}
		}
	}
	return j;
} 

int AlgorithmVP::exitLBay(int j, Point bot, const CGeomHomog& lid)
{
	int wn = 0;		// winding number
	CGeomHomog mouth = m_pVector->meet(m_pOrigin, bot);
	bool lastRight, currRight = false; // called with !right(org,bot,pj)
	while (++j <= 3 * m_pl->m_nCount) 
	{
	    lastRight = currRight; 
	    currRight = mouth.right(m_pl->p(j));
	    if ((currRight != lastRight) // If cross ray org->bot, update wn
			&& (m_pVector->right(m_pl->p(j - 1), m_pl->p(j), m_pOrigin) == currRight)) 
		{ 
			if (!currRight)
				wn++;
			else if (wn-- == 0) 
			{ // on 0->-1 transitions, check window
				CGeomHomog edge = m_pVector->meet(m_pl->p(j - 1), m_pl->p(j));
				if(edge.right(bot) && !edge.cw(mouth, edge, lid))
					return j-1; // j exits window!
			}
		}
	}
	return j;
} 
    
void AlgorithmVP::saveLid() 
{ 
	if (m_sType[m_nTop] == LWALL)  // for LWALL, lid is previous two
		m_nTop--;
	LLidIdx = m_sVP[m_nTop--];
	if (m_sType[m_nTop] == RLID)  // if not RLID, just leave on top().
		RLidIdx = m_sVP[m_nTop--];
	else 
		RLidIdx = NOTSAVED;
}
    
void AlgorithmVP::restoreLid() 
{
	if (RLidIdx != NOTSAVED) 
		push(RLidIdx, RLID); 
	push(LLidIdx, LLID);
}

void AlgorithmVP::store(int index)
{
	for (int i = 0; i <= m_nTop; i++)
	{
		m_nResult[index][i] = m_sVP[i];
		m_nType[index][i] = m_sType[i];
	}
	m_nResult[index][m_pl->m_nCount] = m_nTop;
}

void AlgorithmVP::restore(int i)
{
	m_nTop = m_nResult[i][m_pl->m_nCount];
	m_pOrigin = m_pl->p(i);
	for (int j = 0; j <= m_nTop; j++)
	{
		m_sVP[j] = m_nResult[i][j];
		m_sType[j] = m_nType[i][j];
	}
}

//void AlgorithmVP::Draw(CDC *pDC)
//{
//	if (m_bDone)
//	{
//		restore(0);
//		drawVP(pDC);
//		return;
//	}
//
//	for (int i = 2; i < m_pl->m_nCount; i++)
//	{
//		restore(i);
//		drawVP(pDC);
//		if (!m_bAnimating)
//			return;
//		::Sleep(1000);
//	}
//	restore(0);
//	drawVP(pDC);
//	m_bDone = true;
//}
//
//void AlgorithmVP::drawVP(CDC *pDC)
//{
//	CPen penPoint, penPoly, penVP, penAllPoints;
//	CBrush brushWhite;
//	penPoint.CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
//	penVP.CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
//	penPoly.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
//	penAllPoints.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
//	brushWhite.CreateSolidBrush(RGB(255, 255, 255));
//
//	CRect rect;
//	pDC->GetWindow()->GetClientRect(&rect);
//
//	if (empty())
//		return;
//
//	CGeomHomog p, q;
//	CPen* pOldPen;
//	pOldPen = (CPen*)pDC->SelectObject(&penPoly);
//
//	// wipe out the window
//	pDC->FillRect(rect, &brushWhite);
//	// draw the polygon
//	pDC->Polygon(m_pl->m_pList, m_pl->m_nCount);
//	pDC->SelectObject(&penAllPoints);
//	for (int i = 0; i < m_pl->m_nCount; i++)
//		pDC->Ellipse(m_pl->p(i).x - 2, m_pl->p(i).y - 2, m_pl->p(i).x + 2, m_pl->p(i).y + 2);
//
//	// draw the first point
//	pDC->SelectObject(&penPoint);
//	pDC->Ellipse(m_pOrigin.x - 4, m_pOrigin.y - 4, m_pOrigin.x + 4, m_pOrigin.y + 4);
//
//	// draw the visible area
//	CPoint last = m_pl->p(m_sVP[m_nTop]);
//	pDC->SelectObject(&penVP);
//	for (i = 0; i <= m_nTop; i++)
//	{
//		if (m_sType[i] == RLID)
//		{
//			p.meet(m_pOrigin, last);
//			q.meet(m_pl->p(m_sVP[i]), m_pl->p(m_sVP[i + 1]));
//			p.meet(q);
//			pDC->MoveTo(last);
//			pDC->LineTo(p.toPoint());
//		}
//		else if (m_sType[i] == LWALL)
//		{
//			p.meet(m_pOrigin, m_pl->p(m_sVP[i]));
//			q.meet(m_pl->p(m_sVP[i - 2]), m_pl->p(m_sVP[i - 1]));
//			p.meet(q);
//			pDC->MoveTo(m_pl->p(m_sVP[i]));
//			pDC->LineTo(p.toPoint());
//		}
//		else
//		{
//			pDC->MoveTo(last);
//			pDC->LineTo(m_pl->p(m_sVP[i]));
//		}
//		last = m_pl->p(m_sVP[i]);
//	}
//
//	pDC->SelectObject(pOldPen);
//
//	penPoint.DeleteObject();
//	penVP.DeleteObject();
//}

