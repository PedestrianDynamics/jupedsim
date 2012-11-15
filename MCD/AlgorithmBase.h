// AlgorithmBase.h: interface for the AlgorithmBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ALGORITHMBASE_H_
#define _ALGORITHMBASE_H_



#include "GeomPoly.h"
#include "GeomHomog.h"
#include "GeomVector.h"

class AlgorithmBase  
{
public:
	bool m_bDone;
	CGeomVector* m_pVector;

public:
	AlgorithmBase();
	virtual ~AlgorithmBase();


	virtual void compute(){};
};

#endif //_ALGORITHMBASE_H_
