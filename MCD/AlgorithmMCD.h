// AlgorithmMCD.h: interface for the AlgorithmMCD class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ALGORITHMMCD_H_
#define _ALGORITHMMCD_H_


#include "AlgorithmVP.h"
#include "GeomPairDeque.h"

class AlgorithmMCD : public AlgorithmBase 
{
public:

	AlgorithmMCD(CGeomPoly* pl);
	virtual ~AlgorithmMCD();

	virtual void compute();

	void recoverSolution(int i, int k);
	void update(int a, int b, int w, int i, int j);
	void typeB(int i, int j, int k);
	void typeA(int i, int j, int k);
	bool visible(int i, int j);
	void setBefore(int i);
	void setAfter(int i);
	void initPairDeque(int i, int j, int w, int a, int b);
	CGeomPairDeque& initPairDeque(int i, int j);
	CGeomPairDeque& pairs(int i, int j);
	int weight(int i, int j);
	void setWeight(int i, int j, int w);

	void initSubDecomp();
	void initVisibility();
	void initSubproblems();
	void init();

	//drawing/returning the polygons
	void Draw();
	void drawSubPoly(int i, int k);
	void drawDiagnal(int i, int k);


private:
	int m_nPointsCount;
	int m_nGuard;
	CGeomPoly* m_pl;
	bool* m_bIsReflex;
	int** m_nWeight;
	int* m_nReflex;
	CGeomPairDeque** m_pPD;
	int NONE;
	int BAD;

};

#endif // _ALGORITHMMCD_H_
