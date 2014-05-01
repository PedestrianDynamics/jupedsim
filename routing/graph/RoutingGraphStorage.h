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


class RoutingGraphStorage {

public:
     RoutingGraphStorage();
     ~RoutingGraphStorage();
     void init(Building * b);
     RoutingGraph * GetGraph(std::set<int> closed_doors);



private:
     std::map<std::set<int>, RoutingGraph*> graphs;
     std::set<int> empty_set;
     // do not use this function now
     std::set<int> GetBiggestSubSet(std::set<int> iset) const;

     /**
      * @param iset
      * @param from
      */
     void GenerateNewGraph(std::set<int> iset, std::set<int> from = std::set<int>());



};
#endif /* ROUTINGGRAPHSTORAGE_H_ */
