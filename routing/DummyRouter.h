/*
 * DummyRouter.h
 *
 *  Created on: Aug 7, 2012
 *      Author: piccolo
 */

#ifndef DUMMYROUTER_H_
#define DUMMYROUTER_H_

#include "Routing.h"

class DummyRouter: public Routing {
public:
	DummyRouter();
	virtual ~DummyRouter();

	// virtuelle Funktionen
	virtual int FindExit(Pedestrian* p);
	virtual void Init(Building* b);

};

#endif /* DUMMYROUTER_H_ */
