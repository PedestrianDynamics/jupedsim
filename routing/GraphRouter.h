/*
 * GraphRouter.h
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#ifndef GRAPHROUTER_H_
#define GRAPHROUTER_H_

#include "Router.h"
#include "GlobalRouter.h"
#include "graph/RoutingGraphStorage.h"
#include "../geometry/Building.h"

class GraphRouter: public GlobalRouter {
public:
	GraphRouter();
	virtual ~GraphRouter();

	virtual int FindExit(Pedestrian* p);
	virtual void Init(Building* b);

private:
	RoutingGraphStorage g;
	Building * _building;
	const std::set<int> empty_set;

};

#endif /* GRAPHROUTER_H_ */
