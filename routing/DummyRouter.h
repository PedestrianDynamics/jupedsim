/*
 * DummyRouter.h
 *
 *  Created on: Aug 7, 2012
 *      Author: piccolo
 */

#ifndef DUMMYROUTER_H_
#define DUMMYROUTER_H_

#include "Router.h"

class DummyRouter: public Router {
private:
     Building* _building;
public:
     DummyRouter();
     virtual ~DummyRouter();

     virtual int FindExit(Pedestrian* p);
     virtual void Init(Building* b);

};

#endif /* DUMMYROUTER_H_ */
