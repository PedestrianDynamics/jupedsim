// AlgorithmVP.h: interface for the AlgorithmVP class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ALGORITHMVP_H_
#define _ALGORITHMVP_H_

#include "AlgorithmBase.h"

class AlgorithmVP : public AlgorithmBase  
{
public:
	int m_nPointsCount;
	void restore(int i);
	void store(int index);
	void push(int index, int type);
	void init(int i);
	bool empty();
	int popVisible();
	Point& nextTop();
	Point& top();


	AlgorithmVP(CGeomPoly* pl);
	virtual ~AlgorithmVP();

	void build(int orgIndex);
	virtual void compute();

// attributes;
private:
	CGeomPoly* m_pl;	// simple polygon
	CGeomHomog* m_pHomog;
    Point m_pOrigin;		// origin of visibility polygon
    int   m_nTop;		// stack pointer to top element
    int*  m_sVP;		// stack holds indices of visibility polygon
	int*  m_sType;
	int** m_nResult;
	int** m_nType;
/**    polygon vertex types:    LLID-------------------RLID
                                       |            |
                                       |            |
                             --------LWALL        RWALL---------
*/
	int RLID;
	int LLID;
	int RWALL;
	int LWALL;
	int NOTSAVED;
    int LLidIdx, RLidIdx;
//    Color vtypeLUT[] = {Color.yellow, Color.black, Color.green, Color.blue};
//    String vtypeString[] = {"RL ", "LL ", "RW ", "LW "};

/** Procedures to keep the lid above the current vertex on top of the stack,
    leaving the top() as a vertex of the VP that is also a vertex of sp.
    These use global status variables to keep the code  for build() cleaner.
*/

private:
	int exitRBay(int j, Point bot, const CGeomHomog& lid);
	int exitLBay(int j, Point bot, const CGeomHomog& lid);
	void saveLid();
	void restoreLid();
};

#endif // _ALGORITHMVP_H_
