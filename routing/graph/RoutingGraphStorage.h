/*
 * RoutingGraphStorage.h
 *
 *  Created on: Sep 10, 2012
 *      Author: David Haensel
 */

#ifndef ROUTINGGRAPHSTORAGE_H_
#define ROUTINGGRAPHSTORAGE_H_

#include "RoutingGraph.h"
#include <map>
#include <set>


class RoutingGraphStorage 
{

public:
    RoutingGraphStorage();
    ~RoutingGraphStorage();
    void init(Building * b);
    RoutingGraph * GetGraph(set<int> closed_doors);
    
    

private:
    map<set<int>, RoutingGraph*> graphs;
    set<int> empty_set;
    // do not use this function now
    set<int> GetBiggestSubSet(set<int> iset) const;
    /**
     *
     * @param iset
     * @param from
     */
    void GenerateNewGraph(set<int> iset, set<int> from = set<int>());
    
    
    
};
#endif /* ROUTINGGRAPHSTORAGE_H_ */
