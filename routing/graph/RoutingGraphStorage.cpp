/*
 * RoutingGraphStorage.cpp
 *
 *  Created on: Sep 10, 2012
 *      Author: David Haensel
 */



#include "RoutingGraphStorage.h"

using namespace std;


RoutingGraphStorage::RoutingGraphStorage()
{
}


RoutingGraphStorage::~RoutingGraphStorage()
{
     map<set<int>, RoutingGraph*>::iterator it;

     for(it = graphs.begin(); it != graphs.end(); it++) {
          delete it->second;

     }

}


void RoutingGraphStorage::init(Building * b)
{
     //create the first RoutingGraph with empty set as Key (no door closed right at the beginning

     graphs[empty_set] = new RoutingGraph(b);
}


RoutingGraph * RoutingGraphStorage::GetGraph(set<int> closed_doors)
{
     map<set<int>, RoutingGraph*>::iterator it;
     // check if the graph with closed doors already exists
     it = graphs.find(closed_doors);
     if(it != graphs.end()) {
          return it->second;
     }

     // find the biggest existing subset

     GenerateNewGraph(closed_doors);

     return graphs[closed_doors];
}

void RoutingGraphStorage::GenerateNewGraph(set<int> iset, set<int> from)
{
     set<int>::iterator it;
     graphs[iset] = new RoutingGraph(GetGraph(from));

     for(it = iset.begin(); it != iset.end(); it++) {
          //check if UniqueID is already closed in the "from" graph
          if(from.find(*it) == from.end()) {
               graphs[iset]->closeDoor(*it);

          }

     }
     return;

}

/**
 * Returns the biggest subset for which a graph already exists
 * not finished yet ;)
 */
set<int> RoutingGraphStorage::GetBiggestSubSet(set<int> iset) const
{
     set<int> oset;
     for(int i = iset.size(); i > 1; i--) {
          //check all subsets of size i

     }
     return oset;

}
