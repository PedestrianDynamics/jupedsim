// AlgorithmMWT.h: interface for the AlgorithmMWT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ALGORITHMMWT_H_
#define _ALGORITHMMWT_H_

#include "AlgorithmBase.h"

const int NONE = 0;

class AlgorithmMWT : public AlgorithmBase  
{
public:

	AlgorithmMWT(CGeomPoly *pl);
	virtual ~AlgorithmMWT();

	int m_nPointsCount;
	double getWeight(int i, int l);
	void setWeight(int i, int l, double w);
	void setTriangle(int i, int j, int k);
	virtual void compute();


//attributes:
private:
	CGeomPoly* m_pl;
	double** m_dWeight;
	int** m_nUsedEdge;

};

#endif // _ALGORITHMMWT_H_
