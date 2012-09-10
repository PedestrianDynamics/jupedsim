/*
 * GraphRouter.h
 *
 *  Created on: Aug 20, 2012
 *      Author: David Haensel
 */

#ifndef GRAPHROUTER_H_
#define GRAPHROUTER_H_

#include "Routing.h"
#include "graph/RoutingGraphStorage.h"
#include "../geometry/Building.h"
#include "../geometry/Crossing.h"
#include "../geometry/SubRoom.h"



class GraphRouter: public Routing {
public:
    GraphRouter();
    virtual ~GraphRouter();

    virtual int FindExit(Pedestrian* p);
    virtual void Init(Building* b);
    
private:
    RoutingGraphStorage  g;
    Building * building;
    const set<int> empty_set;
    

};

#endif /* GRAPHROUTER_H_ */
