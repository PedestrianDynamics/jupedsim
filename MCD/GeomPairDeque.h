// GeomPairDeque.h: interface for the CGeomPairDeque class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CGEOMPAIRDEQUE_H_
#define _CGEOMPAIRDEQUE_H_


class CGeomPairDeque
{
public:
	CGeomPairDeque(int n);
	virtual ~CGeomPairDeque();

//attributes:
public:
	void init(CGeomPairDeque& pd);
	void init(int n);
	int underAF();
	void restore();
	void flush();
	bool more();
	bool moreB();
	void popB();
	int underBB();
	int bB();
	int aB();
	bool emptyB();
	void pop();
	void grow(int a[], int factor);
	int bF();
	int aF();
	bool empty();
	void pushNarrow(int i, int j);
	void push(int i, int j);
	CGeomPairDeque();
	CGeomPairDeque(const CGeomPairDeque& pd);

	int m_nLength;
	int* m_nA;
	int* m_nB;
	int  m_nFp;   // front point
	int  m_nBp;   // back point
	int  m_nLast; // the high-water mark for restores
};

#endif // _CGEOMPAIRDEQUE_H_
